/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cRights.cpp
 * Author: karl
 * 
 * Created on 28. August 2016, 17:25
 */

#include "cRights.h"

cRights::cRights() : streaming(false), timers(false), recordings(false), remotecontrol(false), streamcontrol(false) {
}

cRights::cRights(bool admin) : streaming(admin), timers(admin), recordings(admin), remotecontrol(admin), streamcontrol(admin) {
}

cRights::cRights(bool streaming, bool timers, bool recordings, bool remotecontrol, bool streamcontrol) : streaming(streaming), timers(timers), recordings(recordings), remotecontrol(remotecontrol), streamcontrol(streamcontrol) {
}

cRights::cRights(const cRights& src) : streaming(src.streaming), timers(src.timers), recordings(src.recordings), remotecontrol(src.remotecontrol), streamcontrol(src.streamcontrol) {
}

cRights::~cRights() {
}

cRights& cRights::operator =(const cRights& src) {
    if(this != &src) {
        this->streaming = src.streaming;
        this->timers = src.timers;
        this->recordings = src.recordings;
        this->remotecontrol = src.remotecontrol;
        this->streamcontrol = src.streamcontrol;
    }
    return *this;
}

bool cRights::Streaming() {
    return this->streaming;
}

bool cRights::Timers() {
    return this->timers;
}

bool cRights::Recordings() {
    return this->recordings;
}

bool cRights::RemoteControl() {
    return this->remotecontrol;
}

bool cRights::StreamControl() {
    return this->streamcontrol;
}

void cRights::SetStreaming(bool streaming) {
    this->streaming = streaming;
}

void cRights::SetTimers(bool timers) {
    this->timers = timers;
}

void cRights::SetRecordings(bool recordings) {
    this->recordings = recordings;
}

void cRights::SetRemoteControl(bool remotecontrol) {
    this->remotecontrol = remotecontrol;
}

void cRights::SetStreamControl(bool streamcontrol) {
    this->streamcontrol = streamcontrol;
}