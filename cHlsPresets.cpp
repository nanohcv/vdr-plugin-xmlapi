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
        int segduration = atoi(parameters["SegmentDuration"].c_str());
        size_t segBufferSize = atoi(parameters["SegmentBuffer"].c_str());
        int numSegments = atoi(parameters["NumberOfSegments"].c_str());
        int m3u8waitTimeout = atoi(parameters["M3U8WaitTimeout"].c_str());
        int streamTimeout = atoi(parameters["StreamTimeout"].c_str());
        if(profileName != "" && cmd != "" && segduration != 0 && segBufferSize != 0 && numSegments != 0 && m3u8waitTimeout != 0, streamTimeout != 0) {
            cHlsPreset preset(cmd, segduration, segBufferSize, numSegments, m3u8waitTimeout, streamTimeout);
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
    int segDur = 2;
    size_t segBuf = 5242880;
    int numSeg = 3;
    int m3u8wait = 10;
    int sTimeout = 10;
    cHlsPreset preset(cmd, segDur, segBuf, numSeg, m3u8wait, sTimeout);
    return preset;
}
