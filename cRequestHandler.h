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
#include <vdr/timers.h>
#include <vdr/keys.h>
#include <vdr/remote.h>
#include "cDaemonParameter.h"
#include "cPluginConfig.h"
#include "cPreset.h"
#include "cPresets.h"
#include "cUser.h"
#include "cExtensionHeaders.h"
#include "cResponseHandler.h"
#include "cAuth.h"

#ifndef CREQUESTHANDLER_H
#define CREQUESTHANDLER_H

using namespace std;

class cRequestHandler {
public:
    cRequestHandler(struct MHD_Connection *connection, cDaemonParameter *daemonParameter);
    virtual ~cRequestHandler();
    int HandleRequest(const char *url);
private:
    struct MHD_Connection *connection;
    cDaemonParameter *daemonParameter;
    cPluginConfig config;
    cUser user;
    cExtensionHeaders extHeaders;
    cAuth *auth;
    
    int handleRights();
    
    int handleWebSrv(const char *url);

    int handle404Error();
    int handle403Error();

    std::map<std::string, std::string> conInfo;
    
    int handleNotAuthenticated();
    
    cResponseHandler GetErrorHandler();


};

#endif /* CREQUESTHANDLER_H */

