/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cHlsPreset.cpp
 * Author: karl
 * 
 * Created on 24. Juni 2016, 06:46
 */

#include "helpers.h"
#include "cHlsPreset.h"

cHlsPreset::cHlsPreset(string cmd, int segDuration_s, size_t segBufferSize, int numSegments, int m3u8waitTimeout, int streamTimeout) 
    : cmd(cmd), segDuration_s(segDuration_s), segBufferSize(segBufferSize), numSegments(numSegments), m3u8waitTimeout(m3u8waitTimeout), streamTimeout(streamTimeout) {
}

cHlsPreset::cHlsPreset(const cHlsPreset& src) 
    : cmd(src.cmd), segDuration_s(src.segDuration_s), segBufferSize(src.segBufferSize), 
        numSegments(src.numSegments), m3u8waitTimeout(src.m3u8waitTimeout), streamTimeout(src.streamTimeout) {
}

cHlsPreset::~cHlsPreset() {
}

cHlsPreset& cHlsPreset::operator =(const cHlsPreset& src) {
    if(this != &src) {
        this->cmd = src.cmd;
        this->segDuration_s = src.segDuration_s;
        this->segBufferSize = src.segBufferSize;
        this->numSegments = src.numSegments;
        this->m3u8waitTimeout = src.m3u8waitTimeout;
        this->streamTimeout = src.streamTimeout;
    }
    return *this;
}

string cHlsPreset::FFmpegCmd(string ffmpeg, string input, int start) {
    string rstring = "{infile}";
    size_t start_pos = cmd.find(rstring);
    if(start_pos != std::string::npos)
    {
        cmd.replace(start_pos, rstring.length(), input);
    }
    rstring = "{start}";
    start_pos = cmd.find(rstring);
    if(start_pos != std::string::npos)
    {
        string s = "";
        if(start > 0) {
            s = " -ss " + intToString(start);
        }
        cmd.replace(start_pos, rstring.length(), s);
    }
    ffmpeg += " " + cmd;
    return ffmpeg;
}

string cHlsPreset::Cmd() {
    return this->cmd;
}

int cHlsPreset::SegmentDuration() {
    return this->segDuration_s;
}

size_t cHlsPreset::SegmentBuffer() {
    return this->segBufferSize;
}

int cHlsPreset::NumSegments() {
    return this->numSegments;
}

int cHlsPreset::M3U8WaitTimeout() {
    return this->m3u8waitTimeout;
}

int cHlsPreset::StreamTimeout() {
    return this->streamTimeout;
}