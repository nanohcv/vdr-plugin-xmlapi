/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cHlsPreset.h
 * Author: karl
 *
 * Created on 24. Juni 2016, 06:46
 */

#ifndef CHLSPRESET_H
#define CHLSPRESET_H

#include <string>

using namespace std;

class cHlsPreset {
public:
    cHlsPreset(string cmd, int segDuration_s, size_t segBufferSize, int numSegments, int m3u8waitTimeout, int streamTimeout);
    cHlsPreset(const cHlsPreset& src);
    virtual ~cHlsPreset();
    
    cHlsPreset& operator = (const cHlsPreset& src);
    
    string FFmpegCmd(string ffmpeg, string input, int start = 0);
    string Cmd();
    int SegmentDuration();
    size_t SegmentBuffer();
    int NumSegments();
    int M3U8WaitTimeout();
    int StreamTimeout();
    
private:
    string cmd;
    int segDuration_s;
    size_t segBufferSize;
    int numSegments;
    int m3u8waitTimeout;
    int streamTimeout;

};

#endif /* CHLSPRESET_H */

