#include "cResponseHlsStream.h"

cResponseHlsStream::cResponseHlsStream(struct MHD_Connection *connection, cSession *session, cDaemonParameter *daemonParameter)
	: cResponseHandler(connection, session, daemonParameter), presets(this->config.GetHlsPresetsFile()) {

    this->streamName = "";
    this->input = "";
    this->starttime = 0;
};

cResponseHlsStream::~cResponseHlsStream() {

	this->url = NULL;
	this->cstr_preset = NULL;
	this->chid = NULL;
	this->recfile = NULL;
};

int cResponseHlsStream::toStream(const char *url) {

    if(!this->user->Rights().Streaming()) {
        dsyslog("xmlapi: The user %s doesn't have the permission to access hls streams", this->user->Name().c_str());
        return this->handle403Error();
    }

    if(strlen(url) == 5)
        return this->handle404Error();

    this->url = url;

    this->file = string(url).substr(5);
    if (this->file == "stream.m3u8") {

    	if (0 == this->initM3u8()) {
    		return this->toM3u8();
    	}
    } else {
    	return this->toTs();
    }

    return this->handle403Error();
}

int cResponseHlsStream::initM3u8() {

    this->cstr_preset = MHD_lookup_connection_value(this->connection, MHD_GET_ARGUMENT_KIND, "preset");
    if(this->cstr_preset == NULL)
    {
        esyslog("xmlapi: hls stream -> No preset given!");
        return this->handle404Error();
    }
    this->chid = MHD_lookup_connection_value(this->connection, MHD_GET_ARGUMENT_KIND, "chid");
    this->recfile = MHD_lookup_connection_value(this->connection, MHD_GET_ARGUMENT_KIND, "filename");
    if(this->chid == NULL && this->recfile == NULL)
    {
        esyslog("xmlapi: stream -> No chid or filename given!");
        return this->handle404Error();
    }
    if(this->chid != NULL && this->recfile != NULL) {
        esyslog("xmlapi: stream -> Chid and filename given. Only one is allowed!");
        return this->handle404Error();
    }
    if(this->chid) {
        tChannelID id = tChannelID::FromString(chid);
        if(!id.Valid()) {
            esyslog("xmlapi: stream -> invalid chid given");
            return this->handle404Error();
        }
        this->streamName = string(chid) + string(cstr_preset);
        this->input = this->config.GetStreamdevUrl() + string(chid) + ".ts";
        dsyslog("xmlapi: request %s?chid=%s&preset=%s", this->url, this->chid, this->cstr_preset);
    }
    if(this->recfile) {
#if VDRVERSNUM >= 20301
        LOCK_RECORDINGS_READ;
        const cRecording *rec = Recordings->GetByName(this->recfile);
#else
        cRecording *rec = Recordings.GetByName(this->recfile);
#endif
        if(rec == NULL) {
            dsyslog("xmlapi: No recording found with file name '%s'", this->recfile);
            return this->handle404Error();
        }
        this->streamName = string(this->recfile) + string(cstr_preset);
        string recfiles = "'" + string(rec->FileName()) + "/'*.ts";
        const char* cstr_start = MHD_lookup_connection_value(this->connection, MHD_GET_ARGUMENT_KIND, "start");
        if(cstr_start != NULL) {
        	this->starttime = atoi(cstr_start);
        }
        this->input = "concat:$(ls -1 " + recfiles + " | perl -0pe 's/\\n/|/g;s/\\|$//g')";
        dsyslog("xmlapi: request %s?filename=%s&preset=%s", this->url, this->recfile, this->cstr_preset);
    }
	return 0;
};

int cResponseHlsStream::toM3u8() {

    cHlsStream *stream = StreamControl->GetHlsStream(streamName);
    if(stream == NULL) {

        string baseurl = "/hls/";
        cHlsPreset preset = this->presets[this->cstr_preset];
        string presetName = this->cstr_preset;
        stream = new cHlsStream(this->config.GetFFmpeg(), this->config.GetHlsTmpDir(), preset, this->conInfo);
        stream->SetStreamName(streamName);
        int streamid = StreamControl->AddStream(stream);
        stream->SetStreamId(streamid);
        if(!stream->StartStream(this->input, this->starttime)) {
            StreamControl->RemoveStream(streamid);
            return this->handle404Error();
        }

        int fd;
        struct stat sbuf;
        string m3u8File = stream->m3u8File();
        if ( (-1 == (fd = open (m3u8File.c_str(), O_RDONLY))) ||
            (0 != fstat (fd, &sbuf)) ) {
             if (fd != -1)
                 close (fd);
             StreamControl->RemoveStream(streamid);
             return this->handle404Error();
         }
        return this->create(sbuf.st_size, fd)
			->header("Content-Type", "application/x-mpegURL")
			->header("Cache-Control", "no-cache")
			->cors()
			->flush();
    }
    else {
        int fd;
        struct stat sbuf;
        if ( (-1 == (fd = open (stream->m3u8File().c_str(), O_RDONLY))) ||
            (0 != fstat (fd, &sbuf)) ) {
             if (fd != -1)
                 close (fd);
             StreamControl->RemoveStream(stream->StreamId());
             return this->handle404Error();
         }
        return this->create(sbuf.st_size, fd)
			->header("Content-Type", "application/x-mpegURL")
			->header("Cache-Control", "no-cache")
			->cors()
			->flush();
    }
};

int cResponseHlsStream::toTs() {

	vector<string> parts = split(file, '-');
	if(parts.size() != 2)
		return this->handle404Error();
	int streamid = atoi(parts[0].c_str());
	StreamControl->Mutex.Lock();
	cHlsStream *stream = (cHlsStream*)StreamControl->GetStream(streamid);
	StreamControl->Mutex.Unlock();
	if(stream != NULL) {
		StreamControl->Mutex.Lock();
		string tsFile = stream->StreamPath() + file;
		StreamControl->Mutex.Unlock();
		int fd;
		struct stat sbuf;
		if ( (-1 == (fd = open (tsFile.c_str(), O_RDONLY))) ||
			(0 != fstat (fd, &sbuf)) ) {
			 if (fd != -1)
				 close (fd);
			 StreamControl->RemoveStream(stream->StreamId());
			 return this->handle404Error();
		 }
		return this->create(sbuf.st_size, fd)
			->header("Content-Type", "video/mp2t")
			->header("Cache-Control", "no-cache")
			->cors()
			->flush();
	}
	return this->handle404Error();
};
