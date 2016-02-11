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
#include "cPluginConfig.h"
#include "cPreset.h"

class cStreamer {
public:
    cStreamer(cPluginConfig config, cPreset preset);
    cStreamer(const cStreamer& orig);
    virtual ~cStreamer();
    
    cStreamer& operator =(const cStreamer& src);
    
    void Start();
    void Stop();
    
private:
    cPreset preset;
    cPluginConfig config;
    FILE *ffmpeg;

};

#endif /* CSTREAMER_H */

