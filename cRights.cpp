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

cRights::cRights() : streaming(false), timers(false), recordings(false), remotecontrol(false), streamcontrol(false), sessioncontrol(false) {
}

cRights::cRights(bool admin) : streaming(admin), timers(admin), recordings(admin), remotecontrol(admin), streamcontrol(admin), sessioncontrol(admin) {
}

cRights::cRights(bool streaming, bool timers, bool recordings, bool remotecontrol, bool streamcontrol, bool sessioncontrol) : streaming(streaming), timers(timers), recordings(recordings), remotecontrol(remotecontrol), streamcontrol(streamcontrol), sessioncontrol(sessioncontrol) {
}

cRights::cRights(const cRights& src) : streaming(src.streaming), timers(src.timers), recordings(src.recordings), remotecontrol(src.remotecontrol), streamcontrol(src.streamcontrol), sessioncontrol(src.sessioncontrol) {
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
        this->sessioncontrol = src.sessioncontrol;
    }
    return *this;
}

bool cRights::Streaming() const {
    return this->streaming;
}

bool cRights::Timers() const {
    return this->timers;
}

bool cRights::Recordings() const {
    return this->recordings;
}

bool cRights::RemoteControl() const {
    return this->remotecontrol;
}

bool cRights::StreamControl() const {
    return this->streamcontrol;
}

bool cRights::SessionControl() const {
    return this->sessioncontrol;
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

void cRights::SetSessionControl(bool sessioncontrol) {
    this->sessioncontrol = sessioncontrol;
}

bool operator ==(cRights const& lhs, cRights const& rhs) {
    return lhs.Streaming() == rhs.Streaming() &&
            lhs.Timers() == rhs.Timers() &&
            lhs.Recordings() == rhs.Recordings() &&
            lhs.RemoteControl() == rhs.RemoteControl() &&
            lhs.StreamControl() == rhs.StreamControl() &&
            lhs.SessionControl() == rhs.SessionControl();
}

bool operator !=(cRights const& lhs, cRights const& rhs) {
    return !(lhs == rhs);
}