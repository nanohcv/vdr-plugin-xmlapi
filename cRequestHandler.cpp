/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cRequestHandler.cpp
 * Author: karl
 * 
 * Created on 7. Februar 2016, 12:10
 */

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vdr/tools.h>
#include "cRequestHandler.h"
#include "cStreamer.h"

cRequestHandler::cRequestHandler(struct MHD_Connection *connection, 
                                    cPluginConfig config)
    : config(config), presets(config.GetPresetsFile())
{
    this->connection = connection;
}

cRequestHandler::~cRequestHandler() {
}

int cRequestHandler::HandleRequest(const char* method, const char* url) {
    if(0 != strcmp(method, "GET"))
        return MHD_NO;
    if(0 == strcmp(url, "/version.html"))
    {
        return this->handleVersion();
    }
    else if (this->startswith("/stream", url))
    {
        return this->handleStream(url);
    }
    return MHD_NO;
}

int cRequestHandler::handleVersion() {
    string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
                 "<plugin>\n"
                 "    <name>" + this->config.GetPluginName() + "</name>\n"
                 "    <version>" + this->config.GetVersion() + "</version>\n"
                 "</plugin>\n";
    
    struct MHD_Response *response;
    int ret;
    char *page = (char *)malloc((xml.length() + 1) *sizeof(char));
    strcpy(page, xml.c_str());
    dsyslog("xmlapi: xml-length = %d", strlen(page) );
    response = MHD_create_response_from_buffer (strlen (page), 
                                               (void *) page, 
                                               MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header (response, "Content-Type", "text/xml");
    ret = MHD_queue_response(this->connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    return ret;
}

int cRequestHandler::handleStream(const char *url) {
    struct MHD_Response *response;
    int ret;
    const char* cstr_preset = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "preset");
    if(cstr_preset == NULL)
    {
        esyslog("xmlapi: No preset given!");
        return MHD_NO;
    }
    string ps(cstr_preset);
    cPreset preset = this->presets[ps];
    string expectedUrl = "/stream" + preset.Extension();
    if(0 != strcmp(url, expectedUrl.c_str())) {
        esyslog("xmlapi: Url %s doen't ends with stream%s", url, expectedUrl.c_str());
        return MHD_NO;
    }
    const char* chid = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "chid");
    if(chid == NULL)
    {
        esyslog("xmlapi: No chid given!");
        return MHD_NO;
    }
    
    string channelId(chid);
    
    cStreamer *streamer = new cStreamer(this->config, preset, channelId);
    if(!streamer->Start())
    {
        delete streamer;
        return MHD_NO;
    }
    
    response = MHD_create_response_from_callback (MHD_SIZE_UNKNOWN,
                                                8*1024,
                                                &cRequestHandler::stream_reader,
                                                streamer,
                                                &cRequestHandler::clear_stream);
    MHD_add_response_header (response, "Content-Type", preset.MimeType().c_str());
    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    return ret;
}

ssize_t cRequestHandler::stream_reader(void* cls, uint64_t pos, char* buf, size_t max) {
    cStreamer *streamer = (cStreamer *)cls;
    return streamer->Read(buf, max);    
}

void cRequestHandler::clear_stream(void* cls) {
    cStreamer *streamer = (cStreamer *)cls;
    streamer->Stop();
    delete streamer;
}

bool cRequestHandler::startswith(const char* pre, const char* str) {
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

