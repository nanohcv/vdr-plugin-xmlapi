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
}

cStreamControl::cStreamControl(const cStreamControl& orig) {
    this->streams = orig.streams;
}

cStreamControl::~cStreamControl() {
    for(map<int, cStream*>::iterator it = this->streams.begin(); it != this->streams.end(); it++) {
        delete it->second;
    }
    this->streams.clear();
}

int cStreamControl::AddStream(cStream* stream) {
    int nextid = 1;
    for(map<int, cStream*>::iterator it = this->streams.begin(); it != this->streams.end(); it++) {
        if(it->first == nextid)
            nextid++;
        else
            break;
    }
    this->streams.insert(pair<int, cStream*>(nextid, stream));
    return nextid;
}

cStream* cStreamControl::GetStream(int streamid) {
    map<int, cStream*>::iterator it = this->streams.find(streamid);
    if(it == this->streams.end())
        return NULL;
    return it->second;
}

void cStreamControl::RemoveStream(int streamid) {
    this->Mutex.Lock();
    delete this->streams[streamid];
    this->streams.erase(streamid);
    this->Mutex.Unlock();

}

int cStreamControl::RemoveStreamsByIP(string ip) {
    int streams_affected = 0;
    this->Mutex.Lock();
    for(map<int, cStream*>::iterator it = this->streams.begin(); it != this->streams.end(); it++) {
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
    for(map<int, cStream*>::iterator it = this->streams.begin(); it != this->streams.end(); it++) {
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
    for(map<int, cStream*>::iterator it = this->streams.begin(); it != this->streams.end(); it++) {
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
    for(map<int, cStream*>::iterator it = this->streams.begin(); it != this->streams.end(); it++) {
        if(it->second->GetClientIP() == ip) {
            waitpid(it->second->GetPid(), &status, 0);
        }
    }
}

void cStreamControl::WaitingForStreamsByUserAgent(string useragent) {
    int status;
    for(map<int, cStream*>::iterator it = this->streams.begin(); it != this->streams.end(); it++) {
        if(it->second->GetUserAgent() == useragent) {
            waitpid(it->second->GetPid(), &status, 0);
        }
    }
}

void cStreamControl::WaitingForStreamsByUserAgentAndIP(string ip, string useragent) {
    int status;
    for(map<int, cStream*>::iterator it = this->streams.begin(); it != this->streams.end(); it++) {
        if(it->second->GetClientIP() == ip && it->second->GetUserAgent() == useragent) {
            waitpid(it->second->GetPid(), &status, 0);
        }
    }
}

string cStreamControl::GetStreamsXML() {
    string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    xml += "<streams>\n";
    for(map<int, cStream*>::iterator it = this->streams.begin(); it != this->streams.end(); it++) {
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