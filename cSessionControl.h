/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cSessionControl.h
 * Author: karl
 *
 * Created on 3. September 2016, 20:27
 */

#ifndef CSESSIONCONTROL_H
#define CSESSIONCONTROL_H

#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <vdr/tools.h>
#include <vdr/thread.h>
#include "cUser.h"
#include "cSession.h"

using namespace std;

class cSessionControl : protected map<cUser, vector<cSession> > {
public:
    cSessionControl();
    cSessionControl(const cSessionControl& src);
    virtual ~cSessionControl();
    
    vector<cSession> GetSessions(cUser user);
    cSession AddSession(cUser user, long lifetime);
    void AddSession(cUser user, cSession session);
    
    const cUser* GetUserBySessionId(string sessionId);   
    cSession* GetSessionBySessionId(string sessionId);
    
    void RemoveSessionBySessionId(string sessionId);
    void RemoveSessionsByUser(cUser user);
    void RemoveAllSessions();
    
    void RemoveExpiredSessions();
    
    string GetSessionsXml();
    
    cMutex Mutex;
    
    
    
private:
    

};

#endif /* CSESSIONCONTROL_H */

