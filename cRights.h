/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cRights.h
 * Author: karl
 *
 * Created on 28. August 2016, 17:25
 */

#ifndef CRIGHTS_H
#define CRIGHTS_H

class cRights {
public:
    cRights();
    cRights(bool admin);
    cRights(bool streaming, bool timers, bool recordings, bool remotecontrol, bool streamcontrol);
    
    cRights(const cRights& src);
    virtual ~cRights();
    
    cRights& operator = (const cRights& src);
    
    bool Streaming();
    bool Timers();
    bool Recordings();
    bool RemoteControl();
    bool StreamControl();
    
    void SetStreaming(bool streaming);
    void SetTimers(bool timers);
    void SetRecordings(bool recordings);
    void SetRemoteControl(bool remotecontrol);
    void SetStreamControl(bool streamcontrol);
    
private:
    bool streaming;
    bool timers;
    bool recordings;
    bool remotecontrol;
    bool streamcontrol;
};

#endif /* CRIGHTS_H */

