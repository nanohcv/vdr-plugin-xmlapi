/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   cWebServer.c
 * Author: karl
 *
 * Created on 5. Februar 2016, 09:29
 */

#include "cWebServer.h"
#include <cstring>
#include <cstdio>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vdr/tools.h>
#include <microhttpd.h>
#include "cRequestHandler.h"
#include "cUser.h"
#include "cHlsPresets.h"
#include "cPresets.h"
#include "cResponsePreflight.h"

#define EXTERN
#include "globals.h"

cWebServer::cWebServer(cPluginConfig config) : config(config) {
    this->http_daemon = NULL;
    this->https_daemon = NULL;
    this->httpDaemonParameter = new cDaemonParameter(this->config, this->config.GetHttpPort());
    this->httpsDaemonParameter = new cDaemonParameter(this->config, this->config.GetHttpsPort());
    StreamControl = new cStreamControl();
    SessionControl = new cSessionControl();
}

cWebServer::~cWebServer() {
    delete this->httpDaemonParameter;
    delete this->httpsDaemonParameter;
    delete StreamControl;
    StreamControl = NULL;
    delete SessionControl;
    SessionControl = NULL;
}

bool cWebServer::Start() {

    if(this->config.GetUseHttps())
    {
        this->https_daemon = MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION | MHD_USE_SSL,
  	   		     this->config.GetHttpsPort(), &cWebServer::on_client_connect, this,
                             &cWebServer::handle_connection, this->httpsDaemonParameter,
                             MHD_OPTION_HTTPS_MEM_KEY, this->config.GetSSLKey(),
                             MHD_OPTION_HTTPS_MEM_CERT, this->config.GetSSLCert(),
                             MHD_OPTION_HTTPS_PRIORITIES, this->config.GetHttpsPriorities().c_str(),
                             MHD_OPTION_NOTIFY_COMPLETED, &cWebServer::on_request_complete, this,
                             MHD_OPTION_END);
        if(NULL == this->https_daemon)
        {
            esyslog("xmlapi: Start of HTTPS-Daemon failed!");
            return false;
        }
        isyslog("xmlapi: HTTPS-Daemon started!");
    }
    if(!this->config.GetHttpsOnly())
    {
        this->http_daemon = MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION,
                               this->config.GetHttpPort(),
                               &cWebServer::on_client_connect, this,
                               &cWebServer::handle_connection, this->httpDaemonParameter,
                               MHD_OPTION_NOTIFY_COMPLETED, &cWebServer::on_request_complete, this,
                               MHD_OPTION_END);
        if(NULL == this->http_daemon)
        {
            esyslog("xmlapi: Start of HTTP-Daemon failed!");
            return false;
        }
        isyslog("xmlapi: HTTP-Daemon started!");
    }
    return true;
}

void cWebServer::Stop() {
    if(this->http_daemon != NULL)
    {
        MHD_stop_daemon(this->http_daemon);
    }
    if(this->https_daemon != NULL)
    {
        MHD_stop_daemon(this->https_daemon);
    }
}

int cWebServer::handle_connection (void *cls, struct MHD_Connection *connection,
          const char *url,
          const char *method, const char *version,
          const char *upload_data,
          size_t *upload_data_size, void **con_cls) {

    cDaemonParameter *parameter = (cDaemonParameter *)cls;

    if (0 == strcmp (method, MHD_HTTP_METHOD_OPTIONS)) {
    	cSession *session = NULL;
    	cResponsePreflight response(connection, session, parameter);
    	return response.toText();
    }

    if (0 != strcmp (method, MHD_HTTP_METHOD_GET)) {
        printf("No get. Method = %s\n", method);
        return MHD_NO;
    }


    if (NULL == *con_cls)
    {
        *con_cls = connection;
        return MHD_YES;
    }
    cRequestHandler handler(connection, parameter);
    return handler.HandleRequest(url);
}

int cWebServer::on_client_connect (void *cls,
                              const struct sockaddr *addr,
			      socklen_t addrlen)
{
    if (addr->sa_family == AF_INET)
    {
        struct sockaddr_in *sin = (struct sockaddr_in *) addr;
        isyslog("xmlapi: Client %s connecting...", inet_ntoa(sin->sin_addr));
    }

    return MHD_YES;
}

void cWebServer::on_request_complete(void* cls, MHD_Connection* connection,
                                        void** con_cls,
                                        MHD_RequestTerminationCode toe) {
    dsyslog("xmlapi: Request complete");
}


