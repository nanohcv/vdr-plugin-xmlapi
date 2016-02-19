/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cWebServer.h
 * Author: karl
 *
 * Created on 5. Februar 2016, 09:29
 */

#ifndef CWEBSERVER_H
#define CWEBSERVER_H

#include <microhttpd.h>
#include <sys/socket.h>
#include "cPluginConfig.h"

class cWebServer {
public:
    cWebServer(cPluginConfig config);
    virtual ~cWebServer();
    
    bool Start(void);
    void Stop(void);
private:
    cPluginConfig config;
    struct MHD_Daemon *http_daemon;
    struct MHD_Daemon *https_daemon;
    
    static int handle_connection (void *cls, struct MHD_Connection *connection,
          const char *url,
          const char *method, const char *version,
          const char *upload_data,
          size_t *upload_data_size, void **con_cls);
    
    static int on_client_connect (void *cls,
                              const struct sockaddr *addr,
			      socklen_t addrlen);
    
    static void on_request_complete (void *cls, 
                                    struct MHD_Connection * connection,
				    void **con_cls,
                                    enum MHD_RequestTerminationCode toe);

};

#endif /* CWEBSERVER_H */

