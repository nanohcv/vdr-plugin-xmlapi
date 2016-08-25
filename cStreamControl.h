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
#include <queue>
#include <vdr/thread.h>
#include <vdr/tools.h>
#include "cBaseStream.h"
#include "cHlsStream.h"

using namespace std;

class cStreamControl : public cThread {
public:
    cStreamControl();
    cStreamControl(const cStreamControl& orig);
    virtual ~cStreamControl();

    int AddStream(cBaseStream *stream);
    cBaseStream* GetStream(int streamid);
    cHlsStream* GetHlsStream(string streamName);

    void RemoveStream(int streamid);
    void RemoveHlsStream(int streamid);
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
    map<int, cBaseStream*> streams;
    vector<cHlsStream*> hlsStreamsToRemove;
    cMutex hlsRemoveMutex;
    cCondVar hlsRemoveCondVar;
    
    void Action();
};

#endif /* CSTREAMCONTROL_H */

