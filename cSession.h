/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cSession.h
 * Author: karl
 *
 * Created on 3. September 2016, 15:44
 */

#ifndef CSESSION_H
#define CSESSION_H

#include <string>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <clocale>
#include <vdr/tools.h>

using namespace std;

class cSession {
public:
    cSession(long lifetime);
    cSession(const cSession& src);
    virtual ~cSession();
    
    cSession& operator = (const cSession& src);
    
    string GetSessionId() const;
    long GetLifetime() const;
    time_t GetStart() const;
    bool IsExpired() const;
    string Expires() const;
    string Cookie() const;
    void UpdateStart();
    
private:
    long lifetime;
    string sessionId;
    time_t start;
    
    void defaultRand(unsigned char *buf, size_t len);
    string generateSessionId();

};

bool operator == (cSession const& lhs, cSession const& rhs);
bool operator != (cSession const& lhs, cSession const& rhs);

#endif /* CSESSION_H */

