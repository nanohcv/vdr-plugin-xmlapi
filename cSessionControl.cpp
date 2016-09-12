/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cSessionControl.cpp
 * Author: karl
 * 
 * Created on 3. September 2016, 20:27
 */

#include <unistd.h>
#include "cSessionControl.h"
#include "helpers.h"

cSessionControl::cSessionControl() {
}

cSessionControl::cSessionControl(const cSessionControl& src) : map<cUser, vector<cSession> >(src) {
}

cSessionControl::~cSessionControl() {
}

vector<cSession> cSessionControl::GetSessions(cUser user) {
    vector<cSession> emptySessions;
    for(map<cUser, vector<cSession> >::iterator it = this->begin(); it != this->end(); ++it) {
        if(it->first == user) {
            this->Mutex.Lock();
            vector<cSession> sessions(it->second);
            this->Mutex.Unlock();
            return sessions;
        }
    }
    return emptySessions;
}

cSession cSessionControl::AddSession(cUser user, long lifetime) {
    this->Mutex.Lock();
    map<cUser, vector<cSession> >::iterator it = this->find(user);
    if(it != this->end()) {
        cSession session(lifetime);
        (*this)[user].push_back(session);
        this->Mutex.Unlock();
        return session;
    }
    vector<cSession> sessions;
    cSession session(lifetime);
    sessions.push_back(session);
    this->insert(pair<cUser, vector<cSession> >(user, sessions));
    this->Mutex.Unlock();
    return session;
}

void cSessionControl::AddSession(cUser user, cSession session) {
    this->Mutex.Lock();
    map<cUser, vector<cSession> >::iterator it = this->find(user);
    if(it != this->end()) {
        (*this)[user].push_back(session);
    } else {
        vector<cSession> sessions;
        sessions.push_back(session);
        this->insert(pair<cUser, vector<cSession> >(user, sessions));
    }
    this->Mutex.Unlock();
}

const cUser* cSessionControl::GetUserBySessionId(string sessionId) {
    for(map<cUser, vector<cSession> >::iterator it = this->begin(); it != this->end(); ++it) {
        for(vector<cSession>::iterator itv = it->second.begin(); itv != it->second.end(); ++itv) {
            if(itv->GetSessionId() == sessionId)
                return &it->first;
        }
    }
    return NULL;
}

cSession* cSessionControl::GetSessionBySessionId(string sessionId) {
    for(map<cUser, vector<cSession> >::iterator it = this->begin(); it != this->end(); ++it) {
        for(vector<cSession>::iterator itv = it->second.begin(); itv != it->second.end(); ++itv) {
            if(itv->GetSessionId() == sessionId)
                return &(*itv);
        }
    }
    return NULL;
}

void cSessionControl::RemoveSessionBySessionId(string sessionId) {
    this->Mutex.Lock();
    for(map<cUser, vector<cSession> >::iterator it = this->begin(); it != this->end(); ++it) {
        for(vector<cSession>::iterator itv = it->second.begin(); itv != it->second.end(); ++itv) {
            if(itv->GetSessionId() == sessionId) {
                it->second.erase(itv);
                this->Mutex.Unlock();
                return;
            }
        }
    }
    this->Mutex.Unlock();
}

void cSessionControl::RemoveSessionsByUser(cUser user) {
    this->Mutex.Lock();
    this->erase(user);
    this->Mutex.Unlock();
}

void cSessionControl::RemoveAllSessions() {
    this->Mutex.Lock();
    this->clear();
    this->Mutex.Unlock();
}

void cSessionControl::RemoveExpiredSessions() {
    this->Mutex.Lock();
    map<cUser, vector<cSession> >::iterator it = this->begin();
    while(it != this->end()) {
        vector<cSession>::iterator itv = it->second.begin();
        while(itv != it->second.end()) {
            if(itv->IsExpired()) {
                itv = it->second.erase(itv);
            } else {
                ++itv;
            }
        }  
        if(it->second.empty()) {
            this->erase(it++);
        } else {
            ++it;
        }
    }
    this->Mutex.Unlock();
}
