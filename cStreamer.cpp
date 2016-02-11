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

cStreamer::cStreamer(const cStreamer& orig) 
    : config(orig.config), preset(orig.preset), chid(orig.chid)
{
    this->ffmpeg = orig.ffmpeg;
}

cStreamer::~cStreamer() {
}

cStreamer& cStreamer::operator =(const cStreamer& src) {
    if(this != &src) {
        this->config = src.config;
        this->preset = src.preset;
        this->ffmpeg = src.ffmpeg;
        this->chid = src.chid;
    }
    return *this;
}

bool cStreamer::Start() {
    string input = config.GetStreamdevUrl() + this->chid + ".ts";
    string cmd = preset.FFmpegCmd(config.GetFFmpeg(), input);
    this->ffmpeg = popen(cmd.c_str(), "r");
    if(this->ffmpeg == NULL) {
        esyslog("xmlapi: popen fail: %s", cmd.c_str());
        return false;
    }
    return true;
}

void cStreamer::Stop() {
    if(this->ffmpeg != NULL) {
        pclose(this->ffmpeg);
    }
}

ssize_t cStreamer::Read(char* buf, size_t max) {
    ssize_t n = read(fileno(this->ffmpeg), buf, max);
    if (0 == n)
        return MHD_CONTENT_READER_END_OF_STREAM;
    if (n < 0)
        return MHD_CONTENT_READER_END_WITH_ERROR;
    return n;
}