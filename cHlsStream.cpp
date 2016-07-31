/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cHlsStream.cpp
 * Author: karl
 * 
 * Created on 23. Juni 2016, 16:12
 */

#include <vdr/tools.h>
#include "cHlsStream.h"
#include "streamControl.h"

cHlsStream::cHlsStream(cHlsStreamParameter parameter, map<string, string> conInfo)
    : cBaseStream(parameter.FFmpegCmd(), conInfo, true), parameter(parameter),
      m3u8(""), streamid(0), last_m3u8_access(0), firstAccess(true) {
}

cHlsStream::cHlsStream(const cHlsStream& src)
    : cBaseStream(src), parameter(src.parameter), m3u8(src.m3u8), segments(src.segments), streamid(src.streamid), last_m3u8_access(src.last_m3u8_access), firstAccess(src.firstAccess) {
}

cHlsStream::~cHlsStream() {
    if(this->Active()) {
        this->Cancel(-1);
    }
    this->Close();
    for(map<string, segmentBuffer>::iterator it = this->segments.begin(); it != this->segments.end(); it++) {
        delete[] it->second.buffer;
    }
    this->segments.clear();
}

bool cHlsStream::StartStream() {
    if(this->f == NULL) {
        this->Open(cmd.c_str(), "r");
    }
    else {
        return false;
    }

    if(this->f == NULL) {
        esyslog("xmlapi: Cant start ffmpeg");
        return false;
    }
    return this->Start();
}

string cHlsStream::M3U8() {
    
    if(this->m3u8 == "") {
        this->m3u8mutex.Lock();
        this->m3u8condVar.Wait(this->m3u8mutex);
        this->m3u8mutex.Unlock();
        firstAccess = false;
    }
    time(&this->last_m3u8_access);
    return this->m3u8;
}

segmentBuffer *cHlsStream::Segments(string segment) {
    return &this->segments[segment];
}

string cHlsStream::StreamName() {
    return this->parameter.ChannelId() + this->parameter.ProfileName();
}

void cHlsStream::SetStreamId(int id) {
    this->streamid = id;
}

void cHlsStream::Action() {
    double prev_segment_time = 0;
    unsigned int output_index = 1;
    AVInputFormat *ifmt;
    AVOutputFormat *ofmt;
    AVFormatContext *ic = NULL;
    AVFormatContext *oc;
    AVStream *video_st = NULL;
    AVStream *audio_st = NULL;
    AVCodec *codec;
    AVIOContext *avio_ctx = NULL;
    uint8_t *avio_ctx_buffer = NULL;
    size_t avio_ctx_buffer_size = 64*1024;
    char *output_filename;
    char *remove_filename;
    int video_index = -1;
    int audio_index = -1;
    unsigned int first_segment = 1;
    unsigned int last_segment = 0;
    int write_index = 1;
    int decode_done;
    int ret;
    unsigned int i;
    int remove_file;
    char *infile;
    time_t currentTime;
    
    infile = new char[16];
    snprintf(infile, 16, "pipe:%d", fileno(this->f));
    
    output_filename = new char[25];
    remove_filename = new char[25];
    
    avio_ctx_buffer = (uint8_t*)av_malloc(avio_ctx_buffer_size);
    
    av_register_all();
    
    ifmt = av_find_input_format("mpegts");
    if (!ifmt) {
        fprintf(stderr, "Could not find MPEG-TS demuxer\n");
    }
    ret = avformat_open_input(&ic, infile, ifmt, NULL);
    if (ret != 0) {
        fprintf(stderr, "Could not open input file, make sure it is an mpegts file: %d\n", ret);
    }

    if (avformat_find_stream_info(ic, NULL) < 0) {
        fprintf(stderr, "Could not read stream information\n");
    }

    ofmt = av_guess_format("mpegts", NULL, NULL);
    if (!ofmt) {
        fprintf(stderr, "Could not find MPEG-TS muxer\n");
    }

    oc = avformat_alloc_context();
    if (!oc) {
        fprintf(stderr, "Could not allocated output context");
    }
    oc->oformat = ofmt;
    
    for (i = 0; i < ic->nb_streams && (video_index < 0 || audio_index < 0); i++) {
        switch (ic->streams[i]->codec->codec_type) {
            case AVMEDIA_TYPE_VIDEO:
                video_index = i;
                ic->streams[i]->discard = AVDISCARD_NONE;
                video_st = add_output_stream(oc, ic->streams[i]);
                break;
            case AVMEDIA_TYPE_AUDIO:
                audio_index = i;
                ic->streams[i]->discard = AVDISCARD_NONE;
                audio_st = add_output_stream(oc, ic->streams[i]);
                break;
            default:
                ic->streams[i]->discard = AVDISCARD_ALL;
                break;
        }
    }
    // Don't print warnings when PTS and DTS are identical.
    ic->flags |= AVFMT_FLAG_IGNDTS;
    
    //av_dump_format(oc, 0, output_prefix, 1);

    if (video_st) {
      codec = avcodec_find_decoder(video_st->codec->codec_id);
      if (!codec) {
          fprintf(stderr, "Could not find video decoder %x, key frames will not be honored\n", video_st->codec->codec_id);
      }

      if (avcodec_open2(video_st->codec, codec, NULL) < 0) {
          fprintf(stderr, "Could not open video decoder, key frames will not be honored\n");
      }
    }
    
    snprintf(output_filename, 25, "%d-%u.ts", this->streamid, output_index++);
    segmentBuffer segBuf;
    segBuf.buffer = new uint8_t[this->parameter.SegmentBufferSize()];
    segBuf.size = 0;
    segBuf.maxSize = this->parameter.SegmentBufferSize();
    this->segments.insert(pair<string, segmentBuffer>(string(output_filename), segBuf));
    avio_ctx = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size,
                                  1, &this->segments[output_filename], NULL, &cHlsStream::writeToSegment, NULL);
    oc->pb = avio_ctx;
    oc->flags |= AVIO_FLAG_WRITE;
    
    if (avformat_write_header(oc, NULL)) {
        fprintf(stderr, "Could not write mpegts header to first output file\n");
    }
    
    write_index = !this->writeM3U8(first_segment, last_segment, 0);
    
    do {
        double segment_time = prev_segment_time;
        AVPacket packet;
        time(&currentTime);
        if ((!this->Running()) || ((currentTime - this->last_m3u8_access > this->parameter.StreamTimeout()) && (!this->firstAccess)) ){
          break;
        }

        decode_done = av_read_frame(ic, &packet);
        if (decode_done < 0) {
            break;
        }

        if (av_dup_packet(&packet) < 0) {
            fprintf(stderr, "Could not duplicate packet");
            av_free_packet(&packet);
            break;
        }

        // Use video stream as time base and split at keyframes. Otherwise use audio stream
        if (packet.stream_index == video_index && (packet.flags & AV_PKT_FLAG_KEY)) {
            segment_time = packet.pts * av_q2d(video_st->time_base);
        }
        else if (video_index < 0) {
            segment_time = packet.pts * av_q2d(audio_st->time_base);
        }
        else {
          segment_time = prev_segment_time;
        }


        if (segment_time - prev_segment_time >= this->parameter.SegmentDuration()) {
            av_write_trailer(oc);   // close ts file and free memory
            avio_flush(oc->pb);
            //avio_close(oc->pb);

            if (this->parameter.NumSegments() && (int)(last_segment - first_segment) >= this->parameter.NumSegments() - 1) {
                remove_file = 1;
                first_segment++;
            }
            else {
                remove_file = 0;
            }

            if (write_index) {
                write_index = !this->writeM3U8(first_segment, ++last_segment, 0);
            }

            if (remove_file) {
                snprintf(remove_filename, 25, "%d-%u.ts", this->streamid, first_segment - 1);
                this->Lock();
                delete[] this->segments[remove_filename].buffer;
                this->segments.erase(remove_filename);
                this->Unlock();
            }

            snprintf(output_filename, 25, "%d-%u.ts", this->streamid, output_index++);
            segmentBuffer sBuf;
            sBuf.buffer = new uint8_t[this->parameter.SegmentBufferSize()];
            sBuf.size = 0;
            sBuf.maxSize = this->parameter.SegmentBufferSize();
            this->segments.insert(pair<string, segmentBuffer>(string(output_filename), sBuf));
            avio_ctx = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size,
                                  1, &this->segments[output_filename], NULL, &cHlsStream::writeToSegment, NULL);
            oc->pb = avio_ctx;
            oc->flags |= AVIO_FLAG_WRITE;

            // Write a new header at the start of each file
            if (avformat_write_header(oc, NULL)) {
              fprintf(stderr, "Could not write mpegts header to first output file\n");
              break;
            }

            prev_segment_time = segment_time;
        }

        ret = av_interleaved_write_frame(oc, &packet);
        if (ret < 0) {
            fprintf(stderr, "Warning: Could not write frame of stream\n");
        }
        else if (ret > 0) {
            fprintf(stderr, "End of stream requested\n");
            av_free_packet(&packet);
            break;
        }

        av_free_packet(&packet);
    } while (!decode_done);
    
    av_write_trailer(oc);

    if (video_st) {
      avcodec_close(video_st->codec);
    }

    for(i = 0; i < oc->nb_streams; i++) {
        av_freep(&oc->streams[i]->codec);
        av_freep(&oc->streams[i]);
    }

    //avio_close(oc->pb);
    av_free(oc);

    if (this->parameter.NumSegments() && (int)(last_segment - first_segment) >= this->parameter.NumSegments() - 1) {
        remove_file = 1;
        first_segment++;
    }
    else {
        remove_file = 0;
    }

    if (write_index) {
        this->writeM3U8(first_segment, ++last_segment, 1);
    }

    if (remove_file) {
        snprintf(remove_filename, 25, "%d-%u.ts", this->streamid, first_segment - 1);
        this->Lock();
        delete[] this->segments[remove_filename].buffer;
        this->segments.erase(remove_filename);
        this->Unlock();
    }
    this->Close();
    
    delete[] infile;
    delete[] output_filename;
    delete[] remove_filename;
    
    if(this->streamid != 0) {
        StreamControl->RemoveStream(this->streamid);
    }
}

int cHlsStream::writeM3U8(const unsigned int first_segment, const unsigned int last_segment, const int end) {
    char *write_buf = new char[1024];
    string tmp_m3u8 = "";
    if(write_buf == NULL)
        return -1;
    if (this->parameter.NumSegments()) {
        snprintf(write_buf, 1024, "#EXTM3U\n#EXT-X-TARGETDURATION:%d\n#EXT-X-MEDIA-SEQUENCE:%u\n", this->parameter.SegmentDuration(), first_segment);
    }
    else {
        snprintf(write_buf, 1024, "#EXTM3U\n#EXT-X-TARGETDURATION:%d\n", this->parameter.SegmentDuration());
    }
    tmp_m3u8 += write_buf;
    for (unsigned int i = first_segment; i <= last_segment; i++) {
        snprintf(write_buf, 1024, "#EXTINF:%d,\n%s%d-%u.ts\n", this->parameter.SegmentDuration(), this->parameter.BaseUrl().c_str(), this->streamid, i);
        tmp_m3u8 += write_buf;
    }
    if (end) {
        snprintf(write_buf, 1024, "#EXT-X-ENDLIST\n");
        tmp_m3u8 += write_buf;
    }
    if(last_segment >= (unsigned int)this->parameter.NumSegments()) {
        this->m3u8mutex.Lock();
        this->m3u8 = tmp_m3u8;
        this->m3u8condVar.Broadcast();
        this->m3u8mutex.Unlock();
    }
    delete[] write_buf;
    return 0;
}

AVStream *cHlsStream::add_output_stream(AVFormatContext *output_format_context, AVStream *input_stream) {
    AVCodecContext *input_codec_context;
    AVCodecContext *output_codec_context;
    AVStream *output_stream;

    output_stream = avformat_new_stream(output_format_context, 0);

    input_codec_context = input_stream->codec;
    output_codec_context = output_stream->codec;

    output_codec_context->codec_id = input_codec_context->codec_id;
    output_codec_context->codec_type = input_codec_context->codec_type;
    output_codec_context->codec_tag = input_codec_context->codec_tag;
    output_codec_context->bit_rate = input_codec_context->bit_rate;
    output_codec_context->extradata = input_codec_context->extradata;
    output_codec_context->extradata_size = input_codec_context->extradata_size;

    if(av_q2d(input_codec_context->time_base) * input_codec_context->ticks_per_frame > av_q2d(input_stream->time_base) && av_q2d(input_stream->time_base) < 1.0/1000) {
        output_codec_context->time_base = input_codec_context->time_base;
        output_codec_context->time_base.num *= input_codec_context->ticks_per_frame;
    }
    else {
        output_codec_context->time_base = input_stream->time_base;
    }

    switch (input_codec_context->codec_type) {
        case AVMEDIA_TYPE_AUDIO:
            output_codec_context->channel_layout = input_codec_context->channel_layout;
            output_codec_context->sample_rate = input_codec_context->sample_rate;
            output_codec_context->channels = input_codec_context->channels;
            output_codec_context->frame_size = input_codec_context->frame_size;
            if ((input_codec_context->block_align == 1 && input_codec_context->codec_id == AV_CODEC_ID_MP3) || input_codec_context->codec_id == AV_CODEC_ID_AC3) {
                output_codec_context->block_align = 0;
            }
            else {
                output_codec_context->block_align = input_codec_context->block_align;
            }
            break;
        case AVMEDIA_TYPE_VIDEO:
            output_codec_context->pix_fmt = input_codec_context->pix_fmt;
            output_codec_context->width = input_codec_context->width;
            output_codec_context->height = input_codec_context->height;
            output_codec_context->has_b_frames = input_codec_context->has_b_frames;

            if (output_format_context->oformat->flags & AVFMT_GLOBALHEADER) {
                output_codec_context->flags |= CODEC_FLAG_GLOBAL_HEADER;
            }
            break;
    default:
        break;
    }

    return output_stream;
}


int cHlsStream::writeToSegment(void* opaque, uint8_t* buf, int buf_size) {
    segmentBuffer *sbuf = (segmentBuffer*)opaque;
    if((sbuf->size + buf_size) <= sbuf->maxSize) {
        memcpy(sbuf->buffer+sbuf->size, buf, buf_size);
        sbuf->size += buf_size;
    }
    return buf_size;
}
