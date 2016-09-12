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
#include <vdr/tools.h>
#include "helpers.h"

cPluginConfig::cPluginConfig(const char *configDir, const char *cacheDir, const char *pluginName,
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
    this->httpsPriorities = "NORMAL";
    this->usersFile = string(configDir) + "/users.ini";
    this->ffmpeg = "ffmpeg";
    this->waitForFFmpeg = true;
    this->presetsFile = string(configDir) + "/presets.ini";
    this->hlsPresetsFile = string(configDir) + "/hls_presets.ini";
    this->hlsTmpDir = string(cacheDir) + "/streams";
    this->streamdevUrl = "http://127.0.0.1:3000/";
    this->websrvroot = string(configDir) + "/websrv";
    this->websrvheaders = string(configDir) + "/websrv_file_extensions.ini";
    this->corsOrigin = string("*");
    this->realtiveLogoUrl = true;
    this->readFromConfFile(configFile);
    this->createDefaultUserFile(this->usersFile);
    this->users.ReadFromINI(this->usersFile);

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
    this->httpsPriorities = src.httpsPriorities;
    this->usersFile = src.usersFile;
    this->users = src.users;
    this->ffmpeg = src.ffmpeg;
    this->waitForFFmpeg = src.waitForFFmpeg;
    this->presetsFile = src.presetsFile;
    this->hlsPresetsFile = src.hlsPresetsFile;
    this->hlsTmpDir = src.hlsTmpDir;
    this->streamdevUrl = src.streamdevUrl;
    this->websrvroot = src.websrvroot;
    this->websrvheaders = src.websrvheaders;
    this->corsOrigin = src.corsOrigin;
    this->realtiveLogoUrl = src.realtiveLogoUrl;

}

cPluginConfig::~cPluginConfig() {
    delete[] this->sslKey;
    delete[] this->sslCert;
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
        this->httpsPriorities = src.httpsPriorities;
        this->users = src.users;
        this->usersFile = src.usersFile;
        this->ffmpeg = src.ffmpeg;
        this->waitForFFmpeg = src.waitForFFmpeg;
        this->presetsFile = src.presetsFile;
        this->hlsPresetsFile = src.hlsPresetsFile;
        this->hlsTmpDir = src.hlsTmpDir;
        this->streamdevUrl = src.streamdevUrl;
        this->websrvroot = src.websrvroot;
        this->websrvheaders = src.websrvheaders;
        this->corsOrigin = src.corsOrigin;
        this->realtiveLogoUrl = src.realtiveLogoUrl;
        if(src.sslKey != NULL) {
            delete[] this->sslKey;
            this->sslKey = new char[src.sslKeySize];
            memcpy(this->sslKey, src.sslKey, src.sslKeySize);
            this->sslKeySize = src.sslKeySize;
        } else {
            delete[] this->sslKey;
            this->sslKey = NULL;
            this->sslKeySize = 0;
        }
        if(src.sslCert != NULL) {
            delete[] this->sslCert;
            this->sslCert = new char[src.sslCertSize];
            memcpy(this->sslCert, src.sslCert, src.sslCertSize);
            this->sslCertSize = src.sslCertSize;
        } else {
            delete[] this->sslCert;
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

string cPluginConfig::GetHttpsPriorities() {
    return this->httpsPriorities;
}

string cPluginConfig::GetUsersFile() {
    return this->usersFile;
}

cUsers cPluginConfig::GetUsers() {
    return this->users;
}

string cPluginConfig::GetFFmpeg() {
    return this->ffmpeg;
}

bool cPluginConfig::GetWaitForFFmpeg() {
    return this->waitForFFmpeg;
}

string cPluginConfig::GetPresetsFile() {
    return this->presetsFile;
}

string cPluginConfig::GetHlsPresetsFile() {
    return this->hlsPresetsFile;
}

string cPluginConfig::GetHlsTmpDir() {
    return this->hlsTmpDir;
}

string cPluginConfig::GetStreamdevUrl() {
    return this->streamdevUrl;
}

string cPluginConfig::GetWebSrvRoot() {
    return this->websrvroot;
}

string cPluginConfig::GetWebSrvHeadersFile() {
    return this->websrvheaders;
}

string cPluginConfig::GetCorsOrigin() {
    return this->corsOrigin;
}

bool cPluginConfig::RelativeLogoUrl() {
    return this->realtiveLogoUrl;
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
            "HttpsPriorities="<<this->httpsPriorities<<endl<<
            "Users="<<this->usersFile<<endl<<
            "FFMPEG="<<this->ffmpeg<<endl<<
            "WaitForFFmpeg="<<this->waitForFFmpeg<<endl<<
            "Presets="<<this->presetsFile<<endl<<
            "HlsPresets="<<this->hlsPresetsFile<<endl<<
            "HlsTmpDir="<<this->hlsTmpDir<<endl<<
            "StreamdevUrl="<<this->streamdevUrl<<endl<<
            "WebSrvRoot="<<this->websrvroot<<endl<<
            "WebSrvHeaders="<<this->websrvheaders<<endl<<
            "CorsOrigin="<<this->corsOrigin<<endl<<
            "RelativeLogoUrl="<<this->realtiveLogoUrl<<endl;
        fc.close();
        this->createDefaultPresetFile(this->presetsFile);
        this->createDefaultHlsPresetFile(this->hlsPresetsFile);
        this->createDefaultWebSrvHeadersFile(this->websrvheaders);
        return true;
    }



    string line;
    string keyfile;
    string certfile;
    while(getline(fr, line))
    {
        vector<string> sline;
        sline = split(line, '=');
        if(sline.size() != 2)
            continue;
        string left = sline[0];
        string right = sline[1];
        trim(left);
        trim(right);
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
        else if (left == "HttpsPriorities") {
            if(right != "") {
                this->httpsPriorities = right;
            }
        }
        else if (left == "Users") {
            if(right != "") {
                this->usersFile = right;
            }
        }
        else if (left == "FFMPEG") {
            if(right != "")
                this->ffmpeg = right;
        }
        else if (left == "WaitForFFmpeg") {
            this->waitForFFmpeg = (bool)atoi(right.c_str());
        }
        else if (left == "Presets") {
            if(right != "") {
                this->presetsFile = right;
            }
        }
        else if (left == "HlsPresets") {
            if(right != "") {
                this->hlsPresetsFile = right;
            }
        }
        else if (left == "HlsTmpDir") {
            if(right != "") {
                if(right.at(right.length() -1) == '/') {
                    this->hlsTmpDir = right.substr(0, right.length()-1);
                } else {
                    this->hlsTmpDir = right;
                }
            }
        }
        else if (left == "StreamdevUrl") {
            if(right != "") {
                this->streamdevUrl = right;
            }
        }
        else if (left == "WebSrvRoot") {
            if(right != "") {
                if(right.at(right.length() -1) == '/') {
                    this->websrvroot = right.substr(0, right.length()-1);
                } else {
                    this->websrvroot = right;
                }
            }
        }
        else if (left == "WebSrvHeaders") {
            if(right != "") {
                this->websrvheaders = right;
            }
        }
        else if (left == "CorsOrigin") {
            if(right != "") {
                this->corsOrigin = right;
            }
        }
        else if (left == "RelativeLogoUrl") {
            this->realtiveLogoUrl  = (bool)atoi(right.c_str());
        }
    }
    fr.close();
    this->createDefaultPresetFile(this->presetsFile);
    this->createDefaultHlsPresetFile(this->hlsPresetsFile);
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
        esyslog("xmlapi: Can't open SSL-Key file.");
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
        esyslog("xmlapi: Can't open SSL-Cert file.");
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

bool cPluginConfig::createDefaultPresetFile(string presetFile) {
    ifstream prfile;
    prfile.open(presetFile.c_str());
    if(prfile.fail()) {
        ofstream pcfile;
        pcfile.open(presetFile.c_str());
        if(!pcfile.good()) {
            return false;
        }
        
        string preset_nv_low = "[nv_low]\n"
                               "Cmd={ffmpeg} -analyzeduration 1M {start} -re"
                                " -i \"{infile}\""
                                " -c:v h264_nvenc -bufsize 400k -maxrate 200k"
                                " -g 60 -map 0:v -map a:0 -vf \"yadif=0:-1:1, scale=416:234\""
                                " -preset slow -profile:v baseline"
                                " -c:a libfdk_aac -profile:a aac_he -b:a 64k -ar 44100 -ac 2"
                                " -async 1"
                                " -f mpegts pipe:1\n"
                               "MimeType=video/mpeg\n"
                               "Ext=.ts\n";
        
        string preset_nv_mid = "[nv_mid]\n"
                               "Cmd={ffmpeg} -analyzeduration 1M {start} -re"
                                " -i \"{infile}\""
                                " -c:v h264_nvenc -bufsize 2400k -maxrate 1200k"
                                " -g 60 -map 0:v -map a:0 -vf \"yadif=0:-1:1, scale=640:360\""
                                " -preset slow -profile:v baseline"
                                " -c:a libfdk_aac -profile:a aac_he -ab 96k -ar 44100 -ac 2"
                                " -async 1"
                                " -f mpegts pipe:1\n"
                               "MimeType=video/mpeg\n"
                               "Ext=.ts\n";
        
        string preset_nv_main = "[nv_main]\n"
                                 "Cmd={ffmpeg} -analyzeduration 1M {start} -re"
                                 " -i \"{infile}\""
                                 " -c:v h264_nvenc -bufsize 4000k -maxrate 2000k"
                                 " -g 50 -map 0:v -map a:0 -vf \"yadif=0:-1:1, scale=960:540\""
                                 " -preset medium -profile:v main"
                                 " -c:a aac -ab 96k -ar 44100 -ac 2 -strict 2"
                                 " -async 1"
                                 " -f mpegts pipe:1\n"
                                "MimeType=video/mpeg\n"
                                "Ext=.ts\n";
        
        string preset_nv_high = "[nv_high]\n"
                                 "Cmd={ffmpeg} -analyzeduration 1M {start} -re"
                                 " -i \"{infile}\""
                                 " -c:v h264_nvenc -bufsize 7000k -maxrate 3500k"
                                 " -g 50 -map 0:v -map a:0 -vf \"yadif=0:-1:1, scale=1280:720\""
                                 " -preset medium -profile:v main"
                                 " -c:a aac -ab 128k -ar 44100 -ac 2 -strict 2"
                                 " -async 1"
                                 " -f mpegts pipe:1\n"
                                "MimeType=video/mpeg\n"
                                "Ext=.ts\n";
        
        string preset_nv_hd = "[nv_hd]\n"
                               "Cmd={ffmpeg} -analyzeduration 1M {start} -re"
                               " -i \"{infile}\""
                               " -c:v h264_nvenc -bufsize 10000k -maxrate 5000k"
                               " -g 50 -map 0:v -map a:0 -vf \"yadif=0:-1:1, scale=1920:1080\""
                               " -preset medium -profile:v high"
                               " -c:a aac -ab 128k -ar 44100 -ac 2 -strict 2"
                               " -async 1"
                               " -f mpegts pipe:1\n"
                              "MimeType=video/mpeg\n"
                              "Ext=.ts\n";

        string preset_audio = "[Audio]\n"
                              "Cmd={ffmpeg} -analyzeduration 1M {start} -re"
                                 " -i \"{infile}\""
                                 " -f mpegts -vn -acodec libmp3lame"
                                 " -ab 128k -ar 44100 -ac 2 -y pipe:1\n"
                              "MimeType=audio/mpeg\n"
                              "Ext=.ts\n";

        string preset_low = "[Low]\n"
                            "Cmd={ffmpeg} -analyzeduration 1M {start} -re"
                                 " -i \"{infile}\""
                                 " -f mpegts -vcodec libx264"
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
                            "Cmd={ffmpeg} -analyzeduration 1M {start} -re"
                                 " -i \"{infile}\""
                                 " -f mpegts -vcodec libx264"
                                 " -bufsize 2000k -maxrate 1200k -crf 22 -g 50"
                                 " -map 0:v -map a:0"
                                 " -vf \"yadif=0:-1:1, scale=640:360\""
                                 " -preset medium -tune film"
                                 " -vprofile main -level 30"
                                 " -acodec libmp3lame -ab 96k -ar 44100 -ac 2"
                                 " -async 1 pipe:1\n"
                            "MimeType=video/mpeg\n"
                            "Ext=.ts\n";

        string preset_high = "[High]\n"
                             "Cmd={ffmpeg} -analyzeduration 1M {start} -re"
                                 " -i \"{infile}\""
                                 " -f mpegts -vcodec libx264"
                                 " -bufsize 3200k -maxrate 1800k -crf 22 -g 50"
                                 " -map 0:v -map a:0"
                                 " -vf \"yadif=0:-1:1, scale=800:450\""
                                 " -preset medium -tune film"
                                 " -vprofile main -level 30"
                                 " -acodec libmp3lame -ab 96k -ar 44100 -ac 2"
                                 " -async 1 pipe:1\n"
                             "MimeType=video/mpeg\n"
                             "Ext=.ts\n";

        pcfile<<preset_high<<endl<<preset_mid<<endl<<preset_low<<endl<<preset_audio<<endl<<preset_nv_hd<<endl<<preset_nv_high<<endl<<preset_nv_main<<endl<<preset_nv_mid<<endl<<preset_nv_low;
        pcfile.close();
        return true;
    }
    prfile.close();
    return true;
}

bool cPluginConfig::createDefaultHlsPresetFile(string hlsPresetFile) {
    ifstream prfile;
    prfile.open(hlsPresetFile.c_str());
    if(prfile.fail()) {
        ofstream pcfile;
        pcfile.open(hlsPresetFile.c_str());
        if(!pcfile.good()) {
            return false;
        }
        
        string preset_nv_low = "[nv_low]\n"
                               "Cmd={ffmpeg} -analyzeduration 1M {start} -re"
                                " -i \"{infile}\""
                                " -c:v h264_nvenc -bufsize 400k -maxrate 200k"
                                " -g 60 -map 0:v -map a:0 -vf \"yadif=0:-1:1, scale=416:234\""
                                " -preset slow -profile:v baseline"
                                " -c:a libfdk_aac -profile:a aac_he -b:a 64k -ar 44100 -ac 2"
                                " -async 1"
                                " -f hls -hls_time 2 -hls_list_size 5 -hls_wrap 6 -hls_segment_filename '{hls_tmp_path}/{streamid}-%d.ts' {hls_tmp_path}/stream.m3u8\n"
                               "StreamTimeout=3\n"
                               "MinSegments=2\n";
        
        string preset_nv_mid = "[nv_mid]\n"
                               "Cmd={ffmpeg} -analyzeduration 1M {start} -re"
                                " -i \"{infile}\""
                                " -c:v h264_nvenc -bufsize 2400k -maxrate 1200k"
                                " -g 60 -map 0:v -map a:0 -vf \"yadif=0:-1:1, scale=640:360\""
                                " -preset slow -profile:v baseline"
                                " -c:a libfdk_aac -profile:a aac_he -ab 96k -ar 44100 -ac 2"
                                " -async 1"
                                " -f hls -hls_time 2 -hls_list_size 5 -hls_wrap 6 -hls_segment_filename '{hls_tmp_path}/{streamid}-%d.ts' {hls_tmp_path}/stream.m3u8\n"
                               "StreamTimeout=3\n"
                               "MinSegments=2\n";
        
        string preset_nv_main = "[nv_main]\n"
                                 "Cmd={ffmpeg} -analyzeduration 1M {start} -re"
                                 " -i \"{infile}\""
                                 " -c:v h264_nvenc -bufsize 4000k -maxrate 2000k"
                                 " -g 50 -map 0:v -map a:0 -vf \"yadif=0:-1:1, scale=960:540\""
                                 " -preset medium -profile:v main"
                                 " -c:a aac -ab 96k -ar 44100 -ac 2 -strict 2"
                                 " -async 1"
                                 " -f hls -hls_time 2 -hls_list_size 5 -hls_wrap 6 -hls_segment_filename '{hls_tmp_path}/{streamid}-%d.ts' {hls_tmp_path}/stream.m3u8\n"
                                "StreamTimeout=3\n"
                                "MinSegments=2\n";
        
        string preset_nv_high = "[nv_high]\n"
                                 "Cmd={ffmpeg} -analyzeduration 1M {start} -re"
                                 " -i \"{infile}\""
                                 " -c:v h264_nvenc -bufsize 7000k -maxrate 3500k"
                                 " -g 50 -map 0:v -map a:0 -vf \"yadif=0:-1:1, scale=1280:720\""
                                 " -preset medium -profile:v main"
                                 " -c:a aac -ab 128k -ar 44100 -ac 2 -strict 2"
                                 " -async 1"
                                 " -f hls -hls_time 2 -hls_list_size 5 -hls_wrap 6 -hls_segment_filename '{hls_tmp_path}/{streamid}-%d.ts' {hls_tmp_path}/stream.m3u8\n"
                                "StreamTimeout=3\n"
                                "MinSegments=2\n";
        
        string preset_nv_hd = "[nv_hd]\n"
                               "Cmd={ffmpeg} -analyzeduration 1M {start} -re"
                               " -i \"{infile}\""
                               " -c:v h264_nvenc -bufsize 10000k -maxrate 5000k"
                               " -g 50 -map 0:v -map a:0 -vf \"yadif=0:-1:1, scale=1920:1080\""
                               " -preset medium -profile:v high"
                               " -c:a aac -ab 128k -ar 44100 -ac 2 -strict 2"
                               " -async 1"
                               " -f hls -hls_time 2 -hls_list_size 5 -hls_wrap 6 -hls_segment_filename '{hls_tmp_path}/{streamid}-%d.ts' {hls_tmp_path}/stream.m3u8\n"
                              "StreamTimeout=3\n"
                              "MinSegments=2\n";
                                

        string preset_audio = "[Audio]\n"
                              "Cmd={ffmpeg} -analyzeduration 1M {start} -re"
                                 " -i \"{infile}\""
                                 " -vn"
                                 " -acodec aac -strict -2 -ab 64k -ar 44100 -ac 2 -y"
                                 " -f hls -hls_time 2 -hls_list_size 5 -hls_wrap 6 -hls_segment_filename '{hls_tmp_path}/{streamid}-%d.ts' {hls_tmp_path}/stream.m3u8\n"
                              "StreamTimeout=3\n"
                              "MinSegments=2\n";

        string preset_low = "[Low]\n"
                            "Cmd={ffmpeg} -analyzeduration 1M {start} -re"
                                 " -i \"{infile}\""
                                 " -vcodec libx264"
                                 " -bufsize 1400k -maxrate 700k -crf 25 -g 50"
                                 " -map 0:v -map a:0"
                                 " -vf \"yadif=0:-1:1, scale=512:288\""
                                 " -preset medium -tune film"
                                 " -vprofile baseline -level 30"
                                 " -acodec aac -strict -2 -ab 48k -ar 44100 -ac 2"
                                 " -async 1"
                                 " -f hls -hls_time 2 -hls_list_size 5 -hls_wrap 6 -hls_segment_filename '{hls_tmp_path}/{streamid}-%d.ts' {hls_tmp_path}/stream.m3u8\n"
                            "StreamTimeout=3\n"
                            "MinSegments=2\n";

        string preset_mid = "[Mid]\n"
                            "Cmd={ffmpeg} -analyzeduration 1M {start} -re"
                                 " -i \"{infile}\""
                                 " -vcodec libx264"
                                 " -bufsize 2000k -maxrate 1200k -crf 22 -g 50"
                                 " -map 0:v -map a:0"
                                 " -vf \"yadif=0:-1:1, scale=640:360\""
                                 " -preset medium -tune film"
                                 " -vprofile baseline -level 30"
                                 " -acodec aac -strict -2 -ab 64k -ar 44100 -ac 2"
                                 " -async 1"
                                 " -f hls -hls_time 2 -hls_list_size 5 -hls_wrap 6 -hls_segment_filename '{hls_tmp_path}/{streamid}-%d.ts' {hls_tmp_path}/stream.m3u8\n"
                            "StreamTimeout=3\n"
                            "MinSegments=2\n";

        string preset_high = "[High]\n"
                             "Cmd={ffmpeg} -analyzeduration 1M {start} -re"
                                 " -i \"{infile}\""
                                 " -vcodec libx264"
                                 " -bufsize 3200k -maxrate 1800k -crf 22 -g 50"
                                 " -map 0:v -map a:0"
                                 " -vf \"yadif=0:-1:1, scale=800:450\""
                                 " -preset medium -tune film"
                                 " -vprofile baseline -level 30"
                                 " -acodec aac -strict -2 -ab 96k -ar 44100 -ac 2"
                                 " -async 1"
                                 " -f hls -hls_time 2 -hls_list_size 5 -hls_wrap 6 -hls_segment_filename '{hls_tmp_path}/{streamid}-%d.ts' {hls_tmp_path}/stream.m3u8\n"
                             "StreamTimeout=3\n"
                             "MinSegments=2\n";

        pcfile<<preset_high<<endl<<preset_mid<<endl<<preset_low<<endl<<preset_audio<<endl<<preset_nv_hd<<endl<<preset_nv_high<<endl<<preset_nv_main<<endl<<preset_nv_mid<<endl<<preset_nv_low;
        pcfile.close();
        return true;
    }
    prfile.close();
    return true;
}

bool cPluginConfig::createDefaultUserFile(string usersFile) {
    ifstream ufile;
    ufile.open(usersFile.c_str());
    if(ufile.fail()) {
        ofstream urfile;
        urfile.open(usersFile.c_str());
        if(!urfile.good()) {
            return false;
        }
        string adminusers = "; To disable the authentication, remove all admin users and users.\n"
                            "; But do not remove the sections [AdminUsers] and [Users]!\n"
                            ";\n"
                            "; Admin users can do everything :-)\n\n"
                            "[AdminUsers]\n"
                            "xmlapi=" + this->generatePassword(10) + "\n\n";
        string users = "; Users can only read the api's by default!\n"
                       "; They cannot access streams, add any timers, delete/undelete or remove recordings,\n"
                       "; remote contol the vdr or killing streams via streamcontrol api.\n"
                       ";\n"
                       "; You can give an user specific rights.\n"
                       "; To do that, use the following scheme:\n"
                       "; <user name>:<rights separated by commas>=<password>\n"
                       ";\n"
                       "; Possible rights are:\n"
                       ";  streaming  (The user can access streams.)\n"
                       ";  timers  (The user can add or remove timers.)\n"
                       ";  recordings  (The user can delete, undelte or remove recordings.)\n"
                       ";  remotecontrol  (The user can use the switch to channel api and the remote control api.)\n"
                       ";  streamcontrol  (The user can see and remove active streams via stream control api.)\n"
                       ";\n"
                       "; Example:\n"
                       "; You want to give the user \"guest1\" the rights \"streaming\" and \"timers\"\n"
                       "; and the user \"guest2\" the rights \"streaming\" and \"remotecontrol\".\n"
                       ";\n"
                       "; [Users]\n"
                       "; guest1:streaming,timers=pw1234\n"
                       "; guest2:streaming,remotecontrol=pw4321\n\n"
                       "[Users]\n";
        urfile<<adminusers<<endl<<users<<endl;
        urfile.close();
        return true;
    }
    ufile.close();
    return true;
}

bool cPluginConfig::createDefaultWebSrvHeadersFile(string headersFile) {
    ifstream hfile;
    hfile.open(headersFile.c_str());
    if(hfile.fail()) {
        ofstream hrfile;
        hrfile.open(headersFile.c_str());
        if(!hrfile.good()) {
            return false;
        }
        
        string extJs = "[js]\n"
                       "Content-Type=application/javascript\n";
        string extCss = "[css]\n"
                        "Content-Type=text/css\n";
        string extHtm = "[htm]\n"
                        "Content-Type=text/html\n";
        string extHtml = "[html]\n"
                         "Content-Type=text/html\n";
        string extXml = "[xml]\n"
                        "Content-Type=text/xml\n";
        string extTxt = "[txt]\n"
                        "Content-Type=text/plain\n";
        string extJpg = "[jpg]\n"
                        "Content-Type=image/jpeg\n";
        string extJpeg = "[jpeg]\n"
                        "Content-Type=image/jpeg\n";
        string extJpe = "[jpe]\n"
                        "Content-Type=image/jpeg\n";
        string extPng = "[png]\n"
                        "Content-Type=image/png\n";
        
        hrfile<<extJs<<endl<<extCss<<endl<<extHtm<<endl<<extHtml<<endl<<extXml<<endl<<extTxt<<endl<<extJpg<<endl<<extJpeg<<endl<<extJpe<<endl<<extPng<<endl;
        hrfile.close();
        return true;    
    }
    hfile.close();
    return true;
}
