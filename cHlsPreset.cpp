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

cHlsPreset::cHlsPreset(string cmd, int streamTimeout, int minSegments) 
    : cmd(cmd), streamTimeout(streamTimeout), minSegments(minSegments), hls_time(2), hls_list_size(5) {
    this->setHlsTimeFromCmd();
    this->setHlsListSizeFromCmd();
}

cHlsPreset::cHlsPreset(const cHlsPreset& src) 
    : cmd(src.cmd), streamTimeout(src.streamTimeout), minSegments(src.minSegments), hls_time(src.hls_time), hls_list_size(src.hls_list_size) {
}

cHlsPreset::~cHlsPreset() {
}

cHlsPreset& cHlsPreset::operator =(const cHlsPreset& src) {
    if(this != &src) {
        this->cmd = src.cmd;
        this->streamTimeout = src.streamTimeout;
        this->minSegments = src.minSegments;
        this->hls_time = src.hls_time;
        this->hls_list_size = src.hls_list_size;
    }
    return *this;
}

string cHlsPreset::FFmpegCmd(string input, string hlsTmpPath, int streamid, int start) {
    string hlstmp = hlsTmpPath + "/" + intToString(streamid);
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
    rstring = "{streamid}";
    start_pos = cmd.find(rstring);
    if(start_pos != std::string::npos)
    {
        cmd.replace(start_pos, rstring.length(), intToString(streamid));
    }
    rstring = "{hls_tmp_path}";
    while((start_pos = cmd.find(rstring)) != std::string::npos) {
        cmd.replace(start_pos, rstring.length(), hlstmp);
    }
    return cmd;
}

string cHlsPreset::Cmd() {
    return this->cmd;
}


int cHlsPreset::StreamTimeout() {
    if(this->streamTimeout < 1) {
        return 1;
    }
    return this->streamTimeout;
}

int cHlsPreset::MinSegments() {
    if(this->minSegments > this->hls_list_size)
        return this->hls_list_size;
    if(this->minSegments == 0)
        return 1;
    return this->minSegments;
}

int cHlsPreset::HlsTime() {
    return this->hls_time;
}

int cHlsPreset::HlsListSize() {
    return this->hls_list_size;
}

void cHlsPreset::setHlsTimeFromCmd() {
    size_t pos = cmd.find("-hls_time");
    if(pos == string::npos)
        return;
    string sub = cmd.substr(pos);
    sscanf(sub.c_str(), "%*s%d", &this->hls_time);
}

void cHlsPreset::setHlsListSizeFromCmd() {
    size_t pos = cmd.find("-hls_list_size");
    if(pos == string::npos) {
        return;
    }
    string sub = cmd.substr(pos);
    sscanf(sub.c_str(), "%*s%d", &this->hls_list_size);
}