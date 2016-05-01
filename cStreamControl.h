/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cStreamControl.h
 * Author: karl
 *
 * Created on 1. Mai 2016, 07:36
 */

#ifndef CSTREAMCONTROL_H
#define CSTREAMCONTROL_H

#include <string>
#include <map>
#include <vdr/thread.h>
#include "cStream.h"

using namespace std;

class cStreamControl {
public:
    cStreamControl();
    cStreamControl(const cStreamControl& orig);
    virtual ~cStreamControl();
    
    int AddStream(cStream *stream);
    cStream* GetStream(int streamid);
    
    void RemoveStream(int streamid);
    int RemoveStreamsByIP(string ip);
    int RemoveStreamsByUserAgent(string useragent);
    int RemoveStreamsByUserAgentAndIP(string ip, string useragent);
    
    void WaitingForStream(int streamid);
    void WaitingForStreamsByIP(string ip);
    void WaitingForStreamsByUserAgent(string useragent);
    void WaitingForStreamsByUserAgentAndIP(string ip, string useragent);
    
    string GetStreamsXML();
    
    cMutex Mutex;
    
private:
    map<int, cStream*> streams;

};

#endif /* CSTREAMCONTROL_H */

