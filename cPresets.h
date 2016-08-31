/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   cPresets.h
 * Author: karl
 *
 * Created on 11. Februar 2016, 14:03
 */

#ifndef CPRESETS_H
#define CPRESETS_H

#include <string>
#include <map>
#include <vector>
#include "cPreset.h"

using namespace std;

class cPresets : public map<string, cPreset> {
public:
    cPresets(string iniFile);
    cPresets(const cPresets& src);
    virtual ~cPresets();
    cPreset operator[] (string key);
    cPreset GetDefaultPreset();
    vector<string> GetPresetNames();
private:
    
    vector<string> keys;

};

#endif /* CPRESETS_H */

