/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cExtensionHeaders.h
 * Author: karl
 *
 * Created on 29. August 2016, 10:33
 */

#ifndef CEXTENSIONHEADERS_H
#define CEXTENSIONHEADERS_H

#include <string>
#include <map>
#include <vector>
#include "cResponseHeader.h"

using namespace std;

class cExtensionHeaders : public map<string, vector<cResponseHeader> > {
public:
    cExtensionHeaders(string iniFile);
    cExtensionHeaders(const cExtensionHeaders& src);
    virtual ~cExtensionHeaders();
    
    vector<cResponseHeader> operator[] (string key);
    
private:

};

#endif /* CEXTENSIONHEADERS_H */

