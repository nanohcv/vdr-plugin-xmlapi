/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cHlsPresets.h
 * Author: karl
 *
 * Created on 24. Juni 2016, 18:55
 */

#ifndef CHLSPRESETS_H
#define CHLSPRESETS_H

#include <map>
#include <vector>
#include <string>
#include <cstdlib>
#include "cHlsPreset.h"

using namespace std;

class cHlsPresets : public map<string, cHlsPreset> {
public:
    cHlsPresets(string iniFile);
    cHlsPresets(const cHlsPresets& src);
    virtual ~cHlsPresets();
    
    cHlsPreset operator[] (string key);
    cHlsPreset GetDefaultPreset();
    vector<string> GetPresetNames();
    
private:
    vector<string> keys;
};

#endif /* CHLSPRESETS_H */

