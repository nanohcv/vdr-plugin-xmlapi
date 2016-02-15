/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cStreamer.h
 * Author: karl
 *
 * Created on 8. Februar 2016, 20:11
 */

#ifndef CSTREAMER_H
#define CSTREAMER_H

#include <cstdio>
#include <vdr/thread.h>
#include "cPluginConfig.h"
#include "cPreset.h"

class cStreamer : public cThread {
public:
    cStreamer(cPluginConfig config, cPreset preset, string chid);
    cStreamer(const cStreamer& orig);
    virtual ~cStreamer();
    
    cStreamer& operator =(const cStreamer& src);
    
    bool StartFFmpeg();
    void StopFFmpeg();
    void ResetTimeoutTimer();
    
    ssize_t Read(char *buf, size_t max);  
    
private:
    cPluginConfig config;
    cPreset preset;
    string chid;
    FILE *ffmpeg;
    int ffmpeg_fd;
    
    volatile int secs;
    
    void Action();
};

#endif /* CSTREAMER_H */

