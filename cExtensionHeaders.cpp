/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cExtensionHeaders.cpp
 * Author: karl
 * 
 * Created on 29. August 2016, 10:33
 */

#include "cINIParser.h"
#include "cExtensionHeaders.h"

cExtensionHeaders::cExtensionHeaders(string iniFile) {
    cINIParser parser(iniFile);
    vector<string> keys = parser.GetKeys();
    for(unsigned int i=0; i<keys.size(); i++)
    {
        string extension = keys[i];
        map<string, string> headers = parser[keys[i]];
        vector<cResponseHeader> vheaders;
        for(map<string, string>::iterator it = headers.begin(); it != headers.end(); ++it) {
            cResponseHeader header(it->first, it->second);
            vheaders.push_back(header);
        }
        this->insert(pair<string, vector<cResponseHeader> >(extension, vheaders));
    }
}

cExtensionHeaders::cExtensionHeaders(const cExtensionHeaders& orig) {
}

cExtensionHeaders::~cExtensionHeaders() {
}

vector<cResponseHeader> cExtensionHeaders::operator [](string key) {
    map<string, vector<cResponseHeader> >::iterator it = this->find(key);
    if(it == this->end()) {
        vector<cResponseHeader> headers;
        headers.push_back(cResponseHeader("Content-Type", "text/plain"));
        return headers;
    }
    return this->at(key);
}