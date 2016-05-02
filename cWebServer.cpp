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

#define EXTERN
#include "streamControl.h"

cWebServer::cWebServer(cPluginConfig config) : config(config) {
    this->http_daemon = NULL;
    this->https_daemon = NULL;
    StreamControl = new cStreamControl();
}

cWebServer::~cWebServer() {
    delete StreamControl;
    StreamControl = NULL;
}

bool cWebServer::Start() {
    
    if(this->config.GetUseHttps())
    {
        this->https_daemon = MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION | MHD_USE_SSL,
  	   		     this->config.GetHttpsPort(), &cWebServer::on_client_connect, this,
                             &cWebServer::handle_connection, this,
                             MHD_OPTION_HTTPS_MEM_KEY, this->config.GetSSLKey(),
                             MHD_OPTION_HTTPS_MEM_CERT, this->config.GetSSLCert(),
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
                               &cWebServer::handle_connection, this,
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
    
    cWebServer *srv = (cWebServer *)cls;
    struct MHD_Response *response;
    int ret;;
    char *user = NULL;
    char *pass = NULL;
    bool fail;

    if (0 != strcmp (method, MHD_HTTP_METHOD_GET)) {
        printf("No get. Method = %s\n", method);
        return MHD_NO;
    }
        
    
    if (NULL == *con_cls)
    {
        *con_cls = connection;
        return MHD_YES;
    }
    cRequestHandler *handler = new cRequestHandler(connection, srv->config);
    if(srv->config.GetUserName() != "" &&
            srv->config.GetPassword() != "")
    {
        user = MHD_basic_auth_get_username_password (connection, &pass);
        fail = ( (user == NULL) ||
               (0 != strcmp (user, srv->config.GetUserName().c_str())) ||
               (0 != strcmp (pass, srv->config.GetPassword().c_str())));
        if (user != NULL) free (user);
        if (pass != NULL) free (pass);
        if(fail)
        {
            const char *page = "<html><body>Wrong credentials.</body></html>";
            response =
                MHD_create_response_from_buffer (strlen (page), (void *) page, 
                                               MHD_RESPMEM_PERSISTENT);
            ret = MHD_queue_basic_auth_fail_response (connection,
                                                      "XMLAPI",
                                                      response);
            MHD_destroy_response (response);
            delete handler;
            return ret;
        }
        else
        {
            ret = handler->HandleRequest(url);
            delete handler;
            return ret;
        }
    }    
    else
    {
        ret = handler->HandleRequest(url);
        delete handler;
        return ret;
    }
    return MHD_NO;
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


