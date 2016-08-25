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
#include <cstdio>

using namespace std;

class cHlsPreset {
public:
    cHlsPreset(string cmd, int streamTimeout, int minSegments);
    cHlsPreset(const cHlsPreset& src);
    virtual ~cHlsPreset();
    
    cHlsPreset& operator = (const cHlsPreset& src);
    
    string FFmpegCmd(string input, string hlsTmpPath, int streamid, int start = 0);
    string Cmd();
    int StreamTimeout();
    int MinSegments();
    int HlsTime();
    int HlsListSize();
    
private:
    string cmd;
    int streamTimeout;
    int minSegments;
    
    int hls_time;
    int hls_list_size;
    
    void setHlsTimeFromCmd();
    void setHlsListSizeFromCmd();

};

#endif /* CHLSPRESET_H */

