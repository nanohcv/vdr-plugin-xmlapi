/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cPluginConfig.cpp
 * Author: karl
 * 
 * Created on 9. Februar 2016, 14:59
 */

#include "cPluginConfig.h"
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <sstream>

cPluginConfig::cPluginConfig(const char *configDir, const char *pluginName, 
        const char *version) {
    this->configFile = string(configDir) + "/" + string(pluginName) + ".conf";
    
    this->name = string(pluginName);
    this->version = string(version);
    this->configDir = string(configDir);
    this->httpPort = 10080;
    this->httpsPort = 10443;
    this->useHttps = false;
    this->httpsOnly = false;
    this->sslKey = NULL;
    this->sslKeySize = 0;
    this->sslCert = NULL;
    this->sslCertSize = 0;
    this->userName = string(pluginName);
    this->password = this->generatePassword(12);
    this->ffmpeg = "ffmpeg";
    this->presetsFile = string(configDir) + "/presets.ini";
    this->streamdevUrl = "http://127.0.0.1:3000/";
    this->readFromConfFile(configFile);
    
}

cPluginConfig::cPluginConfig(const cPluginConfig& src) {
    this->configFile = src.configFile;
    this->name = src.name;
    this->version = src.version;
    this->configDir = src.configDir;
    this->httpPort = src.httpPort;
    this->httpsPort = src.httpsPort;
    this->useHttps = src.useHttps;
    this->httpsOnly = src.httpsOnly;
    if(src.sslKey != NULL) {
        this->sslKey = new char[src.sslKeySize];
        memcpy(this->sslKey, src.sslKey, src.sslKeySize);
        this->sslKeySize = src.sslKeySize;
    } else {
        this->sslKey = NULL;
        this->sslKeySize = 0;
    }
    if(src.sslCert != NULL) {
        this->sslCert = new char[src.sslCertSize];
        memcpy(this->sslCert, src.sslCert, src.sslCertSize);
        this->sslCertSize = src.sslCertSize;
    } else {
        this->sslCert = NULL;
        this->sslCertSize = 0;
    }
    this->userName = src.userName;
    this->password = src.password;
    this->ffmpeg = src.ffmpeg;
    this->presetsFile = src.presetsFile;
    this->streamdevUrl = src.streamdevUrl;
    
}

cPluginConfig::~cPluginConfig() {
    delete this->sslKey;
    delete this->sslCert;
}

cPluginConfig& cPluginConfig::operator = (const cPluginConfig& src) {
    if(this != &src)
    {
        this->configFile = src.configFile;
        this->name = src.name;
        this->version = src.version;
        this->configDir = src.configDir;
        this->httpPort = src.httpPort;
        this->httpsPort = src.httpsPort;
        this->useHttps = src.useHttps;
        this->httpsOnly = src.httpsOnly;
        this->userName = src.userName;
        this->password = src.password;
        this->ffmpeg = src.ffmpeg;
        this->presetsFile = src.presetsFile;
        this->streamdevUrl = src.streamdevUrl;
        if(src.sslKey != NULL) {
            delete this->sslKey;
            this->sslKey = new char[src.sslKeySize];
            memcpy(this->sslKey, src.sslKey, src.sslKeySize);
            this->sslKeySize = src.sslKeySize;
        } else {
            delete this->sslKey;
            this->sslKey = NULL;
            this->sslKeySize = 0;
        }
        if(src.sslCert != NULL) {
            delete this->sslCert;
            this->sslCert = new char[src.sslCertSize];
            memcpy(this->sslCert, src.sslCert, src.sslCertSize);
            this->sslCertSize = src.sslCertSize;
        } else {
            delete this->sslCert;
            this->sslCert = NULL;
            this->sslCertSize = 0;
        }
    }
    return *this;
}

string cPluginConfig::GetConfigFile() {
    return this->configFile;
}

string cPluginConfig::GetConfigDir() {
    return this->configDir;
}

string cPluginConfig::GetVersion() {
    return this->version;
}

string cPluginConfig::GetPluginName() {
    return this->name;
}

int cPluginConfig::GetHttpPort() {
    return this->httpPort;
}

int cPluginConfig::GetHttpsPort() {
    return this->httpsPort;
}

bool cPluginConfig::GetUseHttps() {
    return this->useHttps;
}

bool cPluginConfig::GetHttpsOnly() {
    return this->httpsOnly;
}

char *cPluginConfig::GetSSLKey() {
    return this->sslKey;
}

char *cPluginConfig::GetSSLCert() {
    return this->sslCert;
}

string cPluginConfig::GetUserName() {
    return this->userName;
}

string cPluginConfig::GetPassword() {
    return this->password;
}

string cPluginConfig::GetFFmpeg() {
    return this->ffmpeg;
}

string cPluginConfig::GetPresetsFile() {
    return this->presetsFile;
}

string cPluginConfig::GetStreamdevUrl() {
    return this->streamdevUrl;
}

string cPluginConfig::generatePassword(unsigned int length){
    const char alphanum[] = "0123456789"
                            "_-!@#$%^&*"
                            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                            "abcdefghijklmnopqrstuvwxyz";
    int a_len = sizeof(alphanum) -1;
    time_t t;
    time(&t);
    srand((unsigned int)t);
    string pw = "";
    for(unsigned int i=0; i<length; i++) {
        pw += alphanum[rand() % a_len];
    }
    
    return pw;
}

bool cPluginConfig::readFromConfFile(string configFile) {
    ifstream fr;
    fr.open(configFile.c_str());
    if(fr.fail())
    {
        ofstream fc;
        fc.open(configFile.c_str());
        if(!fc.good())
        {
            return false;
        }
        fc<<"HttpPort="<<this->httpPort<<endl<<
            "HttpsPort="<<this->httpsPort<<endl<<
            "UseHttps="<<this->useHttps<<endl<<
            "HttpsOnly="<<this->httpsOnly<<endl<<
            "SSLKeyFile="<<endl<<
            "SSLCertFile="<<endl<<
            "UserName="<<this->userName<<endl<<
            "Password="<<this->password<<endl<<
            "FFMPEG="<<this->ffmpeg<<endl<<
            "Presets="<<this->presetsFile<<endl<<
            "StreamdevUrl="<<this->streamdevUrl<<endl;
        fc.close();
        this->createDefaultPresetFile(this->presetsFile);
        return true;       
    }
    
    
    
    string line;
    string keyfile;
    string certfile;
    while(getline(fr, line))
    {
        vector<string> sline;
        sline = this->split(line, '=');
        if(sline.size() != 2)
            continue;
        string left = sline[0];
        string right = sline[1];
        this->trim(left);
        this->trim(right);
        if (left == "HttpPort") {
            this->httpPort = atoi(right.c_str());
        }
        else if (left == "HttpsPort") {
            this->httpsPort = atoi(right.c_str());
        }
        else if (left == "UseHttps") {
            this->useHttps = (bool)atoi(right.c_str());
        }
        else if (left == "HttpsOnly") {
            this->httpsOnly = (bool)atoi(right.c_str());
        }
        else if (left == "SSLKeyFile") {
            keyfile = right;
        }
        else if (left == "SSLCertFile") {
            certfile = right;
        }
        else if (left == "UserName") {
            this->userName = right;
        }
        else if (left == "Password") {
            this->password = right;
        }
        else if (left == "FFMPEG") {
            if(right != "")
                this->ffmpeg = right;
        }
        else if (left == "Presets") {
            if(right != "") {
                this->presetsFile = right;
            }
        }
        else if (left == "StreamdevUrl") {
            if(right != "") {
                this->streamdevUrl = right;
            }
        }
    }
    fr.close();
    this->createDefaultPresetFile(this->presetsFile);
    if (this->httpsOnly == true && this->useHttps == false)
        this->httpsOnly = false;
    if(certfile == "" || keyfile == "")
    {
        this->useHttps = false;
        this->httpsOnly = false;
        return true;
    }
    ifstream kf;
    kf.open(keyfile.c_str(), ifstream::in | ifstream::binary);
    if(kf.fail())
    {
        this->useHttps = false;
        this->httpsOnly = false;
        return false;
    }
    kf.seekg(0, kf.end);
    this->sslKeySize = kf.tellg();
    kf.seekg(0, kf.beg);
    this->sslKey = new char[this->sslKeySize];
    kf.read(this->sslKey, this->sslKeySize);
    kf.close();
    
    ifstream cf;
    cf.open(certfile.c_str(), ifstream::in | ifstream::binary);
    if(cf.fail())
    {
        this->useHttps = false;
        this->httpsOnly = false;
        return false;
    }
    cf.seekg(0, cf.end);
    this->sslCertSize = cf.tellg();
    cf.seekg(0, kf.beg);
    this->sslCert = new char[this->sslCertSize];
    cf.read(this->sslCert, this->sslCertSize);
    cf.close();
    return true;
}

vector<string> cPluginConfig::split(string str, char delimiter) {
    vector<string> internal;
    stringstream ss(str);
    string tok;

    while(getline(ss, tok, delimiter)) {
        internal.push_back(tok);
    }

    return internal;
}

void cPluginConfig::trim(string& str)
{
    string::size_type pos = str.find_last_not_of(' ');
    if(pos != string::npos) {
        str.erase(pos + 1);
        pos = str.find_first_not_of(' ');
        if(pos != string::npos) str.erase(0, pos);
    }
    else str.erase(str.begin(), str.end());
}

bool cPluginConfig::createDefaultPresetFile(string presetFile) {
    ifstream prfile;
    prfile.open(presetFile.c_str());
    if(prfile.fail()) {
        ofstream pcfile;
        pcfile.open(presetFile.c_str());
        if(!pcfile.good()) {
            return false;
        }
        string preset_low = "[Low]\n"
                            "Cmd=-analyzeduration 1M -threads 2 -i \"{infile}\""
                                 " -threads 2 -f mpegts -vcodec libx264"
                                 " -bufsize 1400k -maxrate 700k -crf 25 -g 50"
                                 " -map 0:v -map a:0"
                                 " -vf \"yadif=0:-1:1, scale=512:288\""
                                 " -preset medium -tune film"
                                 " -vprofile baseline -level 30"
                                 " -acodec libmp3lame -ab 64k -ar 44100 -ac 1"
                                 " -async 1 pipe:1\n"
                            "MimeType=video/mpeg\n"
                            "Ext=.ts\n";
        
        string preset_mid = "[Mid]\n"
                            "Cmd=-analyzeduration 1M -threads 2 -i \"{infile}\""
                                 " -threads 2 -f mpegts -vcodec libx264"
                                 " -bufsize 2000k -maxrate 1000k -crf 22 -g 50"
                                 " -map 0:v -map 0:a"
                                 " -vf \"yadif=0:-1:1, scale=640:360\""
                                 " -preset medium -tune film"
                                 " -vprofile main -level 30"
                                 " -acodec libmp3lame -ab 96k -ar 44100 -ac 2"
                                 " -async 1 pipe:1\n"
                            "MimeType=video/mpeg\n"
                            "Ext=.ts\n";
        
        string preset_high = "[High]\n"
                             "Cmd=-analyzeduration 1M -threads 2 -i \"{infile}\""
                                 " -threads 2 -f mpegts -vcodec libx264"
                                 " -bufsize 3200k -maxrate 1600k -crf 22 -g 50"
                                 " -map 0:v -map 0:a"
                                 " -vf \"yadif=0:-1:1, scale=720:405\""
                                 " -preset medium -tune film"
                                 " -vprofile main -level 30"
                                 " -acodec libmp3lame -ab 96k -ar 44100 -ac 2"
                                 " -async 1 pipe:1\n"
                             "MimeType=video/mpeg\n"
                             "Ext=.ts\n";
        
        pcfile<<preset_high<<endl<<preset_mid<<endl<<preset_low;
        pcfile.close();
        return true;
    }
    prfile.close();
    return true;
}
