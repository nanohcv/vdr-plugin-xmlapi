/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cResponseHeader.h
 * Author: karl
 *
 * Created on 29. August 2016, 10:37
 */

#ifndef CRESPONSEHEADER_H
#define CRESPONSEHEADER_H

#include <string>

using namespace std;

class cResponseHeader {
public:
    cResponseHeader(string key, string value);
    cResponseHeader(const cResponseHeader& src);
    virtual ~cResponseHeader();
    
    cResponseHeader& operator = (const cResponseHeader& src);
    
    string Key();
    string Value();
    
private:
    string key;
    string value;

};

#endif /* CRESPONSEHEADER_H */

