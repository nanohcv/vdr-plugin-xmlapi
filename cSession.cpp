/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cSession.cpp
 * Author: karl
 * 
 * Created on 3. September 2016, 15:44
 */

#include "cSession.h"

cSession::cSession(long lifetime) : lifetime(lifetime ? lifetime : 60*60*24*365), sessionId(generateSessionId()), start(time(NULL)) {
}

cSession::cSession(const cSession& src) : lifetime(src.lifetime), sessionId(src.sessionId), start(src.start) {
}

cSession::~cSession() {
}

cSession& cSession::operator =(const cSession& src) {
    if(this != &src) {
        this->lifetime = src.lifetime;
        this->sessionId = src.sessionId;
        this->start = src.start;
    }
    return *this;
}

string cSession::GetSessionId() const {
    return this->sessionId;
}

long cSession::GetLifetime() const {
    return this->lifetime;
}

time_t cSession::GetStart() const {
    return this->start;
}

bool cSession::IsExpired() const {
    time_t now = time(NULL);
    if(this->start + this->lifetime < now) {
        return true;
    }
    return false;
}

string cSession::Expires() const {
    time_t tmp = this->start + this->lifetime;
    struct tm *gm = gmtime(&tmp);
    char gmTimeStr[32];
    char *old_LC_TIME = setlocale(LC_TIME, NULL);
    setlocale(LC_TIME, "C");
    strftime(gmTimeStr, sizeof(gmTimeStr), "%a, %d-%b-%Y %T GMT", gm);
    setlocale(LC_TIME, old_LC_TIME);
    return string(gmTimeStr);
}

string cSession::Cookie() const {
    string cookie = "vdr-plugin-xmlapi_sessionid=" + this->sessionId + ";expires=" + this->Expires();
    return cookie;
}

void cSession::UpdateStart() {
    this->start = time(NULL);
}

void cSession::defaultRand(unsigned char *buf, size_t len) {
    srand(time(NULL));
    for(size_t i=0; i<len; i++) {
        buf[i] = (unsigned char)rand()%255;
    }
}

string cSession::generateSessionId() {
    string id = "";
    unsigned char buf[64];
    char hexBuf[3];
    FILE *f = NULL;
    f = fopen("/dev/urandom", "r");
    if(f == NULL) {
        esyslog("xmlapi: Can't open /dev/urandom! Generate unsafe session id with rand()...");
        defaultRand(buf, sizeof(buf));
    } else {
        size_t readed = 0;
        readed = fread(buf, sizeof(buf), 1, f);
        if(readed != 1) {
            esyslog("xmlapi: Can't read from /dev/urandom! Generate unsafe session id with rand()...");
            defaultRand(buf, sizeof(buf));
        }
        fclose(f);
    }
    for(size_t i=0; i<sizeof(buf); i++) {
        snprintf(hexBuf, 3, "%02x", buf[i]);
        id += string(hexBuf);
    }
    return id;
}

bool operator ==(cSession const& lhs, cSession const& rhs) {
    return lhs.GetLifetime() == rhs.GetLifetime() && lhs.GetSessionId() == rhs.GetSessionId() && lhs.GetStart() == rhs.GetStart();
}

bool operator !=(cSession const& lhs, cSession const& rhs) {
    return !(lhs == rhs);
}
