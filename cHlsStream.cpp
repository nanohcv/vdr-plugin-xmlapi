/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cHlsStream.cpp
 * Author: karl
 * 
 * Created on 23. Juni 2016, 16:12
 */

#include <vdr/tools.h>
#include <wait.h>
#include "helpers.h"
#include "cHlsStream.h"
#include "globals.h"

cHlsStream::cHlsStream(string hlsTmpPath, cHlsPreset preset, map<string, string> conInfo)
    : cBaseStream(conInfo, true), preset(preset), streamid(0), streamName(""), hlsTmpPath(hlsTmpPath), streamPath(hlsTmpPath + "/" + intToString(this->streamid)), stopped(true), last_m3u8_access(0), firstM3U8Access(true) {
}

cHlsStream::cHlsStream(const cHlsStream& src)
    : cBaseStream(src), preset(src.preset), streamid(src.streamid), streamName(src.streamName), hlsTmpPath(src.hlsTmpPath), streamPath(src.streamPath), stopped(src.stopped), last_m3u8_access(src.last_m3u8_access), firstM3U8Access(src.firstM3U8Access) {
}

cHlsStream::~cHlsStream() {
    this->streamid = 0;
    if(!stopped) 
        this->StopStream();
}

string cHlsStream::StreamName() {
    return this->streamName;
}

int cHlsStream::StreamId() {
    return this->streamid;
}

void cHlsStream::SetStreamId(int id) {
    this->streamid = id;
    streamPath = hlsTmpPath + "/" + intToString(this->streamid);
}

void cHlsStream::SetStreamName(string streamName) {
    this->streamName = streamName;
}

bool cHlsStream::StartStream() {
    if(this->cmd == "")
        return false;
    string rmCmd = "rm -f -R " + streamPath;
    if(system(rmCmd.c_str()) != 0) {
        esyslog("xmlapi: cHlsStream::StartStream() can't remove %s", streamPath.c_str());
        return false;
    }
    string mkdirCmd = "mkdir -p -m 0777 " + streamPath;
    if(system(mkdirCmd.c_str()) != 0) {
        esyslog("xmlapi: cHlsStream::StartStream() can't create %s", streamPath.c_str());
        return false;
    }

    
    if ((pid = fork()) < 0) {
        esyslog("xmlapi: cHlsStream::StartStream() fork failed");
        return false;
    }
    
    if(pid == 0) {
        setsid();
        if (execl("/bin/sh", "sh", "-c", this->cmd.c_str(), NULL) == -1) {
            LOG_ERROR_STR(this->cmd.c_str());
            _exit(-1);
        }
        _exit(0);
    }
        
    
    int length, rv, i = 0;
    int fd;
    int wd;
    char buffer[BUF_LEN];
    int filesCount = 0;
    fd_set set;
    struct timeval timeout;

    fd = inotify_init();
    if ( fd < 0 ) {
        StopStream();
        return false;
    }
    
    wd = inotify_add_watch( fd, this->streamPath.c_str(), IN_ALL_EVENTS);
    // wait until min segments created
    while(filesCount < this->preset.MinSegments()) {
        i = 0;
        FD_ZERO (&set);
        FD_SET (fd, &set);
        timeout.tv_sec = this->preset.HlsTime() + 2;
        timeout.tv_usec = 0;
        rv = select(fd + 1, &set, NULL, NULL, &timeout);
        if(rv == -1) {
            esyslog("xmlapi: cHlsStream::StartStream() select error");
            StopStream();
            return false;
        }
        else if(rv == 0) {
            esyslog("xmlapi: cHlsStream::StartStream() inotify timeout -> No stream files created in %s after %d seconds", this->streamPath.c_str(), this->preset.HlsTime() + 2);
            StopStream();
            return false;
        }
        else {
            length = read( fd, buffer, BUF_LEN );
        }
        if(length < 0){
            esyslog("xmlapi: cHlsStream::StartStream() inotify read error");
            StopStream();
            return false;
        }

        while ( i < length ) {
          struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
          if ( event->len ) {
            if (event->mask & IN_MOVED_TO) {
                string m3u8 = event->name;
                if(m3u8 == "stream.m3u8")
                    filesCount++;
            }
          }
          i += EVENT_SIZE + event->len;
        }
    }
    inotify_rm_watch( fd, wd );
    close(fd);
    this->stopped = false;
    this->Start();
    return true;
}

bool cHlsStream::StartStream(string input, int start) {
    this->SetFFmpegCmd(this->preset.FFmpegCmd(input, this->hlsTmpPath, this->streamid, start));
    return this->StartStream();
    
}

void cHlsStream::StopStream() {
    if(this->Running()) {
        this->Cancel();
    } else {
        int status = 0;
        kill(-pid, SIGKILL);
        waitpid(-pid, &status, WNOHANG);
        pid = -1;
        string rmCmd = "rm -f -R " + streamPath;
        if(system(rmCmd.c_str()) != 0) {
            esyslog("xmlapi: cHlsStream::StopStream() can't remove  %s", streamPath.c_str());
        }
        this->stopped = true;
        this->firstM3U8Access = true;
        this->last_m3u8_access = 0;
        if(this->streamid != 0) {
            StreamControl->RemoveHlsStream(this->streamid);
        }
    }   
}

string cHlsStream::m3u8File() {
    this->firstM3U8Access = false;
    time(&this->last_m3u8_access);
    return this->streamPath + "/stream.m3u8"; 
}

string cHlsStream::StreamPath() {
    return this->streamPath + "/";
}

bool cHlsStream::Stopped() {
    return this->stopped;
}

void cHlsStream::Action() {
    time_t currentTime;
    time_t tmpTime;
    time(&tmpTime);
    while(this->Running()) {
        time(&currentTime);
        if(firstM3U8Access) {
            if(currentTime - tmpTime > this->preset.StreamTimeout() + 5)
                break;
        } else {
            if(currentTime - this->last_m3u8_access > this->preset.StreamTimeout())
                break;
        }
        sleep(1);
    }
    dsyslog("xmlapi: hls stream timeout");
    this->Cancel(-1);
    this->StopStream();
}