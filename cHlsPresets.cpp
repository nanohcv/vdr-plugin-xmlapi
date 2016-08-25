/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cHlsPresets.cpp
 * Author: karl
 * 
 * Created on 24. Juni 2016, 18:55
 */

#include "cINIParser.h"
#include "cHlsPresets.h"

cHlsPresets::cHlsPresets(string iniFile) {
    cINIParser parser(iniFile);
    vector<string> keys = parser.GetKeys();
    for(unsigned int i=0; i<keys.size(); i++)
    {
        string profileName = keys[i];
        map<string, string> parameters = parser[keys[i]];
        string cmd = parameters["Cmd"];
        int streamTimeout = atoi(parameters["StreamTimeout"].c_str());
        int minSegments = atoi(parameters["MinSegments"].c_str());
        if(profileName != "" && streamTimeout != 0) {
            cHlsPreset preset(cmd, streamTimeout, minSegments);
            this->insert(pair<string, cHlsPreset>(profileName, preset));
        }
    }
}

cHlsPresets::cHlsPresets(const cHlsPresets& src) {
}

cHlsPresets::~cHlsPresets() {
}

cHlsPreset cHlsPresets::operator [](string key) {
    map<string,cHlsPreset>::iterator it = this->find(key);
    if(it == this->end()) {
        return this->GetDefaultPreset();
    }
    return this->at(key);
}

cHlsPreset cHlsPresets::GetDefaultPreset() {
    string cmd = "-analyzeduration 1M {start} -i \"{infile}\" -f mpegts -vcodec libx264 -bufsize 2000k -maxrate 1200k -crf 22 -g 50 -map 0:v -map a:0 -vf \"yadif=0:-1:1, scale=640:360\" -preset medium -tune film -vprofile main -level 30 -acodec aac -strict -2 -ab 64k -ar 44100 -ac 2 -async 1 pipe:1";
    int sTimeout = 5;
    int minSegments = 3;
    cHlsPreset preset(cmd, sTimeout, minSegments);
    return preset;
}
