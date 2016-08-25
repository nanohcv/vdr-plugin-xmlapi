/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cBaseStream.h
 * Author: karl
 *
 * Created on 23. Juni 2016, 15:44
 */

#ifndef CBASESTREAM_H
#define CBASESTREAM_H

#include <map>
#include <string>
#include <unistd.h>
#include <sys/types.h>

using namespace std;

class cBaseStream {
public:
    cBaseStream(map<string, string> conInfo, bool hls);
    cBaseStream(const cBaseStream& src);
    virtual ~cBaseStream();
    
    
    
    string GetClientIP();
    string GetUserAgent();
    pid_t GetPid();
    bool IsHlsStream();
    
    
protected:
    string cmd;
    map<string, string> connectionInfo;
    pid_t pid;
    bool isHlsStream;
    
    void SetFFmpegCmd(string cmd);
    
private:

};

#endif /* CBASESTREAM_H */

