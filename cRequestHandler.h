/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cRequestHandler.h
 * Author: karl
 *
 * Created on 7. Februar 2016, 12:10
 */
#include <microhttpd.h>
#include <unistd.h>
#include <map>
#include <string>
#include "cDaemonParameter.h"
#include "cPluginConfig.h"
#include "cPreset.h"
#include "cPresets.h"

#ifndef CREQUESTHANDLER_H
#define CREQUESTHANDLER_H

class cRequestHandler {
public:
    cRequestHandler(struct MHD_Connection *connection, cDaemonParameter *daemonParameter);
    virtual ~cRequestHandler();
    int HandleRequest(const char *url);
private:
    struct MHD_Connection *connection;
    cDaemonParameter *daemonParameter;
    cPluginConfig config;
    cPresets presets;
    
    int handleVersion();
    int handleStream(const char *url);
    int handleStreamControl();
    int handleLogos(const char *url);
    int handlePresets();
    int handleChannels();
    string channelsToXml();
    int handleEPG();
    
    int handle404Error();
    
    static ssize_t stream_reader (void *cls, uint64_t pos, char *buf, size_t max);
    static void clear_stream(void *cls);
    
    std::map<std::string, std::string> conInfo;

};

#endif /* CREQUESTHANDLER_H */

