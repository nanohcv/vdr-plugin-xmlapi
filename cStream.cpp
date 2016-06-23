/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   cStream.cpp
 * Author: karl
 *
 * Created on 1. Mai 2016, 07:38
 */

#include <microhttpd.h>
#include <unistd.h>
#include "cStream.h"

cStream::cStream(string ffmpegCmd, map<string, string> conInfo)
    : cBaseStream(ffmpegCmd, conInfo, false) {
}

cStream::cStream(const cStream& src) 
    : cBaseStream(src) {
}

cStream::~cStream() {
}

cStream& cStream::operator =(const cStream& src) {
    if(this != &src)
    {
        this->pid = src.pid;
        this->f = src.f;
        this->connectionInfo = src.connectionInfo;
        this->cmd = src.cmd;
        this->isHlsStream = src.isHlsStream;
    }
    return *this;
}

bool cStream::StartFFmpeg() {
    if(this->f == NULL) {
        this->Open(cmd.c_str(), "r");
    }
    else {
        return false;
    }

    if(this->f == NULL) {
        esyslog("xmlapi: Cant start ffmpeg");
        return false;
    }

    return true;
}

void cStream::StopFFmpeg() {
    if(this->f != NULL) {
        this->Close();
        this->f = NULL;
    }
}

ssize_t cStream::Read(char* buf, size_t max) {
    ssize_t n = 0;
    if(this->f != NULL) {
        n = read(fileno(this->f), buf, max);
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
