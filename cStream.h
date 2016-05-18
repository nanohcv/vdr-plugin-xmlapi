/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   cStream.h
 * Author: karl
 *
 * Created on 1. Mai 2016, 07:38
 */

#ifndef CSTREAM_H
#define CSTREAM_H

#include <string>
#include <map>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <vdr/tools.h>
#include "cPreset.h"


using namespace std;

class cStream {
public:
    cStream(string ffmpegCmd, map<string, string> conInfo);
    cStream(const cStream& src);
    virtual ~cStream();

    cStream& operator = (const cStream& src);

    bool StartFFmpeg();
    void StopFFmpeg();
    ssize_t Read(char *buf, size_t max);

    string GetClientIP();
    string GetUserAgent();
    pid_t GetPid();


private:
    pid_t pid;
    FILE *f;
    string cmd;
    map<string, string> connectionInfo;

    bool Open(const char *Command, const char *Mode);
    int Close(void);
};

#endif /* CSTREAM_H */

