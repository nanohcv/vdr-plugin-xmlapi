/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   cStreamControl.cpp
 * Author: karl
 *
 * Created on 1. Mai 2016, 07:36
 */
#include <sstream>
#include <unistd.h>
#include <wait.h>
#include "cStreamControl.h"

cStreamControl::cStreamControl() {
    this->Start();
}

cStreamControl::cStreamControl(const cStreamControl& orig) {
    this->streams = orig.streams;
}

cStreamControl::~cStreamControl() {
    if(this->Active()) {
        this->Cancel(1);
    }
    for(map<int, cBaseStream*>::iterator it = this->streams.begin(); it != this->streams.end(); it++) {
        delete it->second;
    }
    this->streams.clear();
}

int cStreamControl::AddStream(cBaseStream* stream) {
    int nextid = 1;
    for(map<int, cBaseStream*>::iterator it = this->streams.begin(); it != this->streams.end(); it++) {
        if(it->first == nextid)
            nextid++;
        else
            break;
    }
    this->streams.insert(pair<int, cBaseStream*>(nextid, stream));
    return nextid;
}

cBaseStream* cStreamControl::GetStream(int streamid) {
    map<int, cBaseStream*>::iterator it = this->streams.find(streamid);
    if(it == this->streams.end())
        return NULL;
    return it->second;
}

cHlsStream* cStreamControl::GetHlsStream(string streamName) {
    for(map<int, cBaseStream*>::iterator it = this->streams.begin(); it != this->streams.end(); it++) {
        if(it->second->IsHlsStream()) {
            cHlsStream *hlsStream = (cHlsStream*)it->second;
            if(hlsStream->StreamName() == streamName)
                return hlsStream;
        }
    }
    return NULL;
}

void cStreamControl::RemoveStream(int streamid) {
    dsyslog("xmlapi: removing stream with id %d ...", streamid);
    this->Mutex.Lock();
    delete this->streams[streamid];
    this->streams.erase(streamid);
    this->Mutex.Unlock();
    dsyslog("xmlapi: stream with id %d removed", streamid);
}

void cStreamControl::RemoveHlsStream(int streamid) {
    dsyslog("xmlapi: removing hls stream with id %d ...", streamid);
    this->hlsRemoveMutex.Lock();
    this->hlsStreamsToRemove.push_back((cHlsStream*)this->streams[streamid]);
    this->hlsRemoveCondVar.Broadcast();
    this->hlsRemoveMutex.Unlock();
    this->streams.erase(streamid);
}

int cStreamControl::RemoveStreamsByIP(string ip) {
    int streams_affected = 0;
    this->Mutex.Lock();
    for(map<int, cBaseStream*>::iterator it = this->streams.begin(); it != this->streams.end(); it++) {
        if(it->second->GetClientIP() == ip) {
            delete it->second;
            this->streams.erase(it->first);
            streams_affected++;
        }
    }
    this->Mutex.Unlock();
    return streams_affected;
}

int cStreamControl::RemoveStreamsByUserAgent(string useragent) {
    int streams_affected = 0;
    this->Mutex.Lock();
    for(map<int, cBaseStream*>::iterator it = this->streams.begin(); it != this->streams.end(); it++) {
        if(it->second->GetUserAgent() == useragent) {
            delete it->second;
            this->streams.erase(it->first);
            streams_affected++;
        }
    }
    this->Mutex.Unlock();
    return streams_affected;
}

int cStreamControl::RemoveStreamsByUserAgentAndIP(string ip, string useragent) {
    int streams_affected = 0;
    this->Mutex.Lock();
    for(map<int, cBaseStream*>::iterator it = this->streams.begin(); it != this->streams.end(); it++) {
        if(it->second->GetClientIP() == ip && it->second->GetUserAgent() == useragent) {
            delete it->second;
            this->streams.erase(it->first);
            streams_affected++;
        }
    }
    this->Mutex.Unlock();
    return streams_affected;
}

void cStreamControl::WaitingForStream(int streamid) {
    int status;
    waitpid(this->streams[streamid]->GetPid(), &status, 0);
}

void cStreamControl::WaitingForStreamsByIP(string ip) {
    int status;
    for(map<int, cBaseStream*>::iterator it = this->streams.begin(); it != this->streams.end(); it++) {
        if(it->second->GetClientIP() == ip) {
            waitpid(it->second->GetPid(), &status, 0);
        }
    }
}

void cStreamControl::WaitingForStreamsByUserAgent(string useragent) {
    int status;
    for(map<int, cBaseStream*>::iterator it = this->streams.begin(); it != this->streams.end(); it++) {
        if(it->second->GetUserAgent() == useragent) {
            waitpid(it->second->GetPid(), &status, 0);
        }
    }
}

void cStreamControl::WaitingForStreamsByUserAgentAndIP(string ip, string useragent) {
    int status;
    for(map<int, cBaseStream*>::iterator it = this->streams.begin(); it != this->streams.end(); it++) {
        if(it->second->GetClientIP() == ip && it->second->GetUserAgent() == useragent) {
            waitpid(it->second->GetPid(), &status, 0);
        }
    }
}

string cStreamControl::GetStreamsXML() {
    string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    xml += "<streams>\n";
    for(map<int, cBaseStream*>::iterator it = this->streams.begin(); it != this->streams.end(); it++) {
        xml += "    <stream id=\"";
        ostringstream temp;
        temp<<it->first;
        xml += temp.str() + "\">\n";
        xml += "        <pid>";
        temp.clear();
        temp<<it->second->GetPid();
        xml += temp.str() + "</pid>\n";
        xml += "        <ip>" + it->second->GetClientIP() + "</ip>\n";
        xml += "        <useragent>" + it->second->GetUserAgent() + "</useragent>\n";
        xml += "    </stream>\n";
    }
    xml += "</streams>\n";
    return xml;
}

void cStreamControl::Action() {
    while(this->Running()) {
        this->hlsRemoveMutex.Lock();
        this->hlsRemoveCondVar.Wait(this->hlsRemoveMutex);
        for(vector<cHlsStream*>::iterator it = this->hlsStreamsToRemove.begin(); it != this->hlsStreamsToRemove.end(); it++) {
            int streamId = (*it)->StreamId();
            this->Mutex.Lock();
            delete (*it);
            this->Mutex.Unlock();
            dsyslog("xmlapi: hls stream with id %d removed", streamId);        
        }
        this->hlsStreamsToRemove.clear();
        this->hlsRemoveMutex.Unlock();;
    }
}