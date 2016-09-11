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
#include "cDaemonParameter.h"
#include "cUser.h"
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
    cUser user;
    cAuth *auth;
    
    int handleNotAuthenticated();
    cResponseHandler GetErrorHandler();
};

#endif /* CREQUESTHANDLER_H */

