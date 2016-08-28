/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cBaseStream.cpp
 * Author: karl
 * 
 * Created on 23. Juni 2016, 15:44
 */

#include <vdr/tools.h>
#include "cBaseStream.h"

cBaseStream::cBaseStream(string ffmpeg, map<string, string> conInfo, bool hls)
    : cmd(""), ffmpeg(ffmpeg), connectionInfo(conInfo), pid(-1), isHlsStream(hls) {
}

cBaseStream::cBaseStream(const cBaseStream& src) 
    : cmd(src.cmd), ffmpeg(src.ffmpeg), connectionInfo(src.connectionInfo), pid(src.pid), isHlsStream(src.isHlsStream) {
}

cBaseStream::~cBaseStream() {
}

void cBaseStream::SetFFmpegCmd(string cmd) {
    dsyslog("xmlapi: FFmpeg cmd: %s", cmd.c_str());
    this->cmd = cmd;
}

string cBaseStream::GetClientIP() {
    return this->connectionInfo["ClientIP"];
}

string cBaseStream::GetUserAgent() {
    return this->connectionInfo["User-Agent"];
}

pid_t cBaseStream::GetPid() {
    return this->pid;
}

bool cBaseStream::IsHlsStream() {
    return this->isHlsStream;
}

