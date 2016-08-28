/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cUsers.cpp
 * Author: karl
 * 
 * Created on 29. Mai 2016, 08:48
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include "cUsers.h"
#include "cRights.h"
#include "cINIParser.h"

cUsers::cUsers() {
}

cUsers::cUsers(const cUsers& src) 
    : vector(src) {
}

cUsers::~cUsers() {
}

void cUsers::ReadFromINI(string userIniFile) {
    cINIParser parser = cINIParser(userIniFile);
    vector<string> keys = parser.GetKeys();
    for(unsigned int i=0; i<keys.size(); i++) {
        string key = keys[i];
        if(key == "AdminUsers") {
            map<string, string> users = parser[key];
            for(map<string, string>::iterator it = users.begin(); it != users.end(); ++it) {
                cUser user(it->first, it->second, cRights(true));
                this->push_back(user);
            }
        }
        else {
            map<string, string> users = parser[key];
            for(map<string, string>::iterator it = users.begin(); it != users.end(); ++it) {
                vector<string> ur = split(it->first, ':');
                if(ur.size() == 1) {
                    cUser user(ur[0], it->second, cRights(false));
                    this->push_back(user);
                }
                if(ur.size() == 2) {
                    string rights_str = ur[1];
                    vector<string> rights_vec = split(rights_str, ',');
                    cRights rights;
                    for(vector<string>::iterator rit = rights_vec.begin(); rit != rights_vec.end(); ++rit) {
                        if(*rit == "streaming")
                            rights.SetStreaming(true);
                        if(*rit == "timers")
                            rights.SetTimers(true);
                        if(*rit == "recordings")
                            rights.SetRecordings(true);
                        if(*rit == "remotecontrol")
                            rights.SetRemoteControl(true);
                        if(*rit == "streamcontrol")
                            rights.SetStreamControl(true);
                    }
                    cUser user(ur[0], it->second, rights);
                    this->push_back(user);
                }
            }
        }
    }
}

bool cUsers::MatchUser(char* name, char* password) {
    for(vector<cUser>::iterator it = this->begin(); it != this->end(); ++it) {
        if( (0 == strcmp(name, it->Name().c_str())) && (0 == strcmp(password, it->Password().c_str())) )
            return true;
    }
    return false;
}

cUser cUsers::GetUser(char* name) {
    for(vector<cUser>::iterator it = this->begin(); it != this->end(); ++it) {
        if(0 == strcmp(name, it->Name().c_str()))
            return *it;
    }
    return cUser();
}
