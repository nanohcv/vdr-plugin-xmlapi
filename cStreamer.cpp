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
    this->ffmpeg_fd = 0;
    this->secs = 0;
}

cStreamer::cStreamer(const cStreamer& orig) 
    : config(orig.config), preset(orig.preset), chid(orig.chid)
{
    this->ffmpeg = orig.ffmpeg;
    this->ffmpeg_fd = orig.ffmpeg_fd;
    this->secs = orig.secs;
}

cStreamer::~cStreamer() {
}

cStreamer& cStreamer::operator =(const cStreamer& src) {
    if(this != &src) {
        this->config = src.config;
        this->preset = src.preset;
        this->chid = src.chid;
        this->ffmpeg = src.ffmpeg;
        this->ffmpeg_fd = src.ffmpeg_fd;
        this->secs = src.secs;
    }
    return *this;
}

bool cStreamer::StartFFmpeg() {
    string input = config.GetStreamdevUrl() + this->chid + ".ts";
    string cmd = preset.FFmpegCmd(config.GetFFmpeg(), input);
    if(this->ffmpeg == NULL) {
        this->ffmpeg = popen(cmd.c_str(), "r");
    }
    else {
        return false;
    }
    
    if(this->ffmpeg == NULL) {
        esyslog("xmlapi: Cant start ffmpeg");
        return false;
    }
    this->ffmpeg_fd = fileno(this->ffmpeg);
    this->Start();
    return true;
}

void cStreamer::StopFFmpeg() {
    this->Cancel();
    if(this->ffmpeg != NULL) {
        this->Lock();
        pclose(this->ffmpeg);
        this->ffmpeg = NULL;
        this->Unlock();
    }
    
}

void cStreamer::ResetTimeoutTimer() {
    this->secs = 0;
}

ssize_t cStreamer::Read(char* buf, size_t max) {
    ssize_t n = 0;
    if(this->ffmpeg != NULL) {
        this->Lock();
        n = read(this->ffmpeg_fd, buf, max);
        this->Unlock();
    }
    else {
        return MHD_CONTENT_READER_END_WITH_ERROR;
    }
    this->ResetTimeoutTimer();
    if (0 == n)
        return MHD_CONTENT_READER_END_OF_STREAM;
    if (n < 0)
        return MHD_CONTENT_READER_END_WITH_ERROR;
    return n;
}

void cStreamer::Action() {
    while(this->Running() && this->secs < this->config.GetFFmpegReadTimeout()) {
        sleep(1);
        this->secs++;
    }
    if(this->secs >= this->config.GetFFmpegReadTimeout())
        dsyslog("xmlapi: FFmpeg read timeout");
    
    if(this->ffmpeg != NULL) {
        this->Lock();
        pclose(this->ffmpeg);
        this->ffmpeg = NULL;
        this->Unlock();
    }
}
