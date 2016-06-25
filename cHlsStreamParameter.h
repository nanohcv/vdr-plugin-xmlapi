/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cHlsStreamParameter.h
 * Author: karl
 *
 * Created on 24. Juni 2016, 07:04
 */

#ifndef CHLSSTREAMPARAMETER_H
#define CHLSSTREAMPARAMETER_H

#include <string>
#include "cHlsPreset.h"

using namespace std;


class cHlsStreamParameter {
public:
    cHlsStreamParameter(string ffmpegCmd, string channelId, string profileName, string baseUrl, 
                        cHlsPreset preset);
    cHlsStreamParameter(string ffmpegCmd, string channelId, string profileName,
                        string baseUrl, int segmentDuration, size_t segmentBufferSize,
                        int numSegments, int m3u8waitTimeout, int streamTimeout);
    cHlsStreamParameter(const cHlsStreamParameter& src);
    virtual ~cHlsStreamParameter();
    
    cHlsStreamParameter& operator = (const cHlsStreamParameter& src);
    
    string FFmpegCmd();
    string ChannelId();
    string ProfileName();
    string BaseUrl();
    int SegmentDuration();
    size_t SegmentBufferSize();
    int NumSegments();
    int M3U8WaitTimeout();
    int StreamTimeout();
    
private:
    string ffmpegCmd;
    string channelId;
    string profileName;
    string baseUrl;
    int segmentDuration;
    size_t segmentBufferSize;
    int numSegments;
    int m3u8waitTimeout;
    int streamTimeout;
};

#endif /* CHLSSTREAMPARAMETER_H */

