/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cHlsStream.h
 * Author: karl
 *
 * Created on 23. Juni 2016, 16:12
 */

#ifndef CHLSSTREAM_H
#define CHLSSTREAM_H

#include <map>
#include <string>
#include <ctime>
#include <stdint.h>
#include <vdr/thread.h>
#ifdef __cplusplus
extern "C"
{
    #include <libavformat/avformat.h>
}
#endif
#include "cBaseStream.h"
#include "cHlsStreamParameter.h"

using namespace std;

typedef struct {
    uint8_t *buffer;
    size_t size;
} segmentBuffer;

class cHlsStream : public cBaseStream, public cThread {
public:
    cHlsStream(cHlsStreamParameter parameter, map<string, string> conInfo);
    cHlsStream(const cHlsStream& src);
    virtual ~cHlsStream();
    
    bool StartStream();
    
    string M3U8();
    segmentBuffer *Segments(string segment);
    
    string StreamName();
    
    void SetStreamId(int id);
    
private:
    cHlsStreamParameter parameter;
    string m3u8;
    map<string, segmentBuffer> segments;
    int streamid;
    cMutex m3u8mutex;
    cCondVar m3u8condVar;
    time_t last_m3u8_access;
    bool firstAccess;
    
    void Action();
    int writeM3U8(const unsigned int first_segment, const unsigned int last_segment, const int end);
    AVStream *add_output_stream(AVFormatContext *output_format_context, AVStream *input_stream);
    
    static int writeToSegment(void *opaque,  uint8_t *buf, int buf_size);
};

#endif /* CHLSSTREAM_H */

