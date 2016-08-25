/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cHlsStream.h
 * Author: karl
 *
 * Created on 23. Juni 2016, 16:12
 */

#ifndef CHLSSTREAM_H
#define CHLSSTREAM_H

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <signal.h>
#include <time.h>
#include <map>
#include <string>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <stdint.h>
#include <vdr/thread.h>
#include "cBaseStream.h"
#include "cHlsPreset.h"

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )


using namespace std;


class cHlsStream : public cBaseStream, public cThread {
public:
    cHlsStream(string hlsTmpPath, cHlsPreset preset, map<string, string> conInfo);
    cHlsStream(const cHlsStream& src);
    virtual ~cHlsStream();
    
    string StreamName();
    int StreamId();
    
    void SetStreamId(int id);
    void SetStreamName(string streamName);
    bool StartStream();
    bool StartStream(string input, int start = 0);
    void StopStream();
    string m3u8File();
    string StreamPath();
    
    bool Stopped();
    
private:
    cHlsPreset preset;
    int streamid;
    string streamName;
    
    string hlsTmpPath;
    string streamPath;
    
    bool stopped;
    time_t last_m3u8_access;
    bool firstM3U8Access;
    void Action();
};

#endif /* CHLSSTREAM_H */

