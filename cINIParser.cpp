/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cINIParser.cpp
 * Author: karl
 * 
 * Created on 10. Februar 2016, 07:12
 */

#include "cINIParser.h"
#include <fstream>
#include <sstream>

cINIParser::cINIParser(string iniFile) {
    parse(iniFile);
}

cINIParser::cINIParser(const cINIParser& src) 
    : map(src)
{
}

cINIParser::~cINIParser() {
}

vector<string> cINIParser::GetKeys() {
    vector<string> keys;
    for(map<string, map<string, string> >::iterator it = this->begin(); it != this->end(); ++it) {
        keys.push_back(it->first);
    }
    return keys;
}

vector<string> cINIParser::split(string str, char delimiter) {
    vector<string> internal;
    stringstream ss(str);
    string tok;

    while(getline(ss, tok, delimiter)) {
        internal.push_back(tok);
    }

    return internal;
}

void cINIParser::trim(string& str) {
    string::size_type pos = str.find_last_not_of(' ');
    if(pos != string::npos) {
        str.erase(pos + 1);
        pos = str.find_first_not_of(' ');
        if(pos != string::npos) str.erase(0, pos);
    }
    else str.erase(str.begin(), str.end());
}

void cINIParser::parse(string iniFile) {
    ifstream ini;
    ini.open(iniFile.c_str());
    if(ini.fail())
        return;
    string line;
    string section;
    while(getline(ini, line)) {
        trim(line);
        if(line.length() > 0 && line[0] == ';')
            continue;
        if(line.length() > 2 && line[0] == '[' && line[line.length()-1] == ']')
        {
            section = line.substr(1, line.length() -2);
            trim(section);
            map<string, string> keyvaluepair;
            this->insert(pair<string, map<string, string> >(section, keyvaluepair));
        }
        if(section == "")
            continue;
        string key;
        string value;
        vector<string> kvp = split(line, '=');
        if(kvp.size() < 2)
            continue;
        else if (kvp.size() == 2) {
            key = kvp[0];
            value = kvp[1];
        }
        else {
            key = kvp[0];
            for(int i=1; i<kvp.size()-1; i++){
                value += kvp[i] + "=";
            }
            value += kvp[kvp.size()-1];
        }      
        trim(key);
        trim(value);
        if(section != "")
            this->at(section).insert(pair<string, string>(key, value));
    }
    ini.close();
}

