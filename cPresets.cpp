/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   cPresets.cpp
 * Author: karl
 *
 * Created on 11. Februar 2016, 14:03
 */

#include "cPresets.h"
#include "cINIParser.h"
#include <vector>

using namespace std;

cPresets::cPresets(string iniFile) {
    cINIParser parser(iniFile);
    this->keys = parser.GetKeys();
    for(unsigned int i=0; i<keys.size(); i++)
    {
        string profileName = keys[i];
        map<string, string> parameters = parser[keys[i]];
        string cmd = parameters["Cmd"];
        string mime = parameters["MimeType"];
        string ext = parameters["Ext"];
        if(profileName != "" && cmd != "" && mime != "" && ext != "") {
            cPreset preset(cmd, mime, ext);
            this->insert(pair<string, cPreset>(profileName, preset));
        }
    }

}

cPresets::cPresets(const cPresets& src) {
}

cPresets::~cPresets() {
}

cPreset cPresets::operator [](string key) {
    map<string,cPreset>::iterator it = this->find(key);
    if(it == this->end()) {
        return this->GetDefaultPreset();
    }
    return this->at(key);
}

cPreset cPresets::GetDefaultPreset() {
    string cmd = "-analyzeduration 1M -threads 2 {start} -i \"{infile}\" -threads 2"
              " -f mpegts -vcodec libx264 -bufsize 2000k"
              " -maxrate 1000k -crf 22 -g 50 -map 0:v -map a:0"
              " -vf \"yadif=0:-1:1, scale=640:360\" -preset medium -tune film"
              " -vprofile main -level 30 -acodec libmp3lame -ab 96k -ar 44100"
              " -ac 2 -async 1 pipe:1";
    string mime = "video/mpeg";
    string ext = ".ts";
    cPreset preset(cmd, mime, ext);
    return preset;
}

vector<string> cPresets::GetPresetNames() {
    return this->keys;
}
