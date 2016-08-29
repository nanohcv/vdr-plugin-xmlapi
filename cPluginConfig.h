/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   cPluginConfig.h
 * Author: karl
 *
 * Created on 9. Februar 2016, 14:59
 */

#ifndef CPLUGINCONFIG_H
#define CPLUGINCONFIG_H

#include <string>
#include <vector>
#include "cUsers.h"

using namespace std;

class cPluginConfig {
public:
    cPluginConfig(const char *configDir, const char *cacheDir, const char *pluginName, const char *version);
    cPluginConfig(const cPluginConfig& src);
    virtual ~cPluginConfig();

    cPluginConfig& operator = (const cPluginConfig& src);

    string GetConfigFile();
    string GetPluginName();
    string GetVersion();
    string GetConfigDir();
    int GetHttpPort();
    int GetHttpsPort();
    bool GetUseHttps();
    bool GetHttpsOnly();
    char *GetSSLKey();
    char *GetSSLCert();
    string GetUsersFile();
    cUsers GetUsers();
    string GetFFmpeg();
    bool GetWaitForFFmpeg();
    string GetPresetsFile();
    string GetHlsPresetsFile();
    string GetHlsTmpDir();
    string GetStreamdevUrl();
    string GetWebSrvRoot();
    string GetWebSrvHeadersFile();

private:
    string configFile;
    string version;
    string name;
    string configDir;
    int httpPort;
    int httpsPort;
    bool useHttps;
    bool httpsOnly;
    char *sslKey;
    size_t sslKeySize;
    char *sslCert;
    size_t sslCertSize;
    string usersFile;
    cUsers users;
    string ffmpeg;
    bool waitForFFmpeg;
    string presetsFile;
    string hlsPresetsFile;
    string hlsTmpDir;
    string streamdevUrl;
    string websrvroot;
    string websrvheaders;

    string generatePassword(unsigned int length);
    bool readFromConfFile(string configFile);
    bool createDefaultPresetFile(string presetFile);
    bool createDefaultHlsPresetFile(string hlsPresetFile);
    bool createDefaultUserFile(string usersFile);
    bool createDefaultWebSrvHeadersFile(string headersFile);


};

#endif /* CPLUGINCONFIG_H */

