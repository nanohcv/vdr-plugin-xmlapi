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

cStreamer::cStreamer(cPluginConfig config, cPreset preset)
    : config(config), preset(preset)
{
    this->ffmpeg = NULL;
}

cStreamer::cStreamer(const cStreamer& orig)
    : config(orig.config), preset(orig.preset)
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
    }
    return *this;
}

void cStreamer::Start() {
    
}

void cStreamer::Stop() {
    
}