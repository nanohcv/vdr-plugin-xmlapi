/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cStreamer.cpp
 * Author: karl
 * 
 * Created on 8. Februar 2016, 20:11
 */

#include "cStreamer.h"
#include <vdr/tools.h>
#include <microhttpd.h>


cStreamer::cStreamer(cPluginConfig config, cPreset preset, string chid)
    : config(config), preset(preset), chid(chid)
{
    this->ffmpeg = NULL;
}

cStreamer::~cStreamer() {
}

bool cStreamer::StartFFmpeg() {
    string input = config.GetStreamdevUrl() + this->chid + ".ts";
    string cmd = preset.FFmpegCmd(config.GetFFmpeg(), input);
    if(this->ffmpeg == NULL) {
        this->Open(cmd.c_str(), "r");
        this->ffmpeg = this->operator FILE*();
    }
    else {
        return false;
    }
    
    if(this->ffmpeg == NULL) {
        esyslog("xmlapi: Cant start ffmpeg");
        return false;
    }
    
    return true;
}

void cStreamer::StopFFmpeg() {
    if(this->ffmpeg != NULL) {
        this->Close();
        this->ffmpeg = NULL;
    }  
}

ssize_t cStreamer::Read(char* buf, size_t max) {
    ssize_t n = 0;
    if(this->ffmpeg != NULL) {
        n = read(fileno(this->ffmpeg), buf, max);
    }
    else {
        return MHD_CONTENT_READER_END_WITH_ERROR;
    }
    
    if (0 == n) {
        return MHD_CONTENT_READER_END_OF_STREAM;
    }
    if (n < 0) {
        return MHD_CONTENT_READER_END_WITH_ERROR;
    }
    return n;
}
