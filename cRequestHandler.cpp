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
#include <ctime>
#include <vdr/tools.h>
#include <vdr/channels.h>
#include <vdr/epg.h>
#include "cRequestHandler.h"
#include "cStreamer.h"
#include "helpers.h"

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
    if(0 == strcmp(url, "/version.xml"))
    {
        return this->handleVersion();
    }
    else if (startswith(url, "/stream"))
    {
        return this->handleStream(url);
    }
    else if (0 == strcmp(url, "/presets.ini")) {
        return this->handlePresets();
    }
    else if (0 == strcmp(url, "/channels.xml")) {
        return this->handleChannels();
    }
    else if (0 == strcmp(url, "/epg.xml")) {
        return this->handleEPG();
    }
    else {
        return this->handle404Error();
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
        esyslog("xmlapi: stream -> No preset given!");
        return this->handle404Error();
    }
    string ps(cstr_preset);
    cPreset preset = this->presets[ps];
    string expectedUrl = "/stream" + preset.Extension();
    if(0 != strcmp(url, expectedUrl.c_str())) {
        esyslog("xmlapi: Url %s doen't ends with stream%s", url, expectedUrl.c_str());
        return this->handle404Error();
    }
    const char* chid = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "chid");
    if(chid == NULL)
    {
        esyslog("xmlapi: stream -> No chid given!");
        return this->handle404Error();
    }
    dsyslog("xmlapi: request %s?chid=%s&preset=%s", url, chid, cstr_preset);
    tChannelID id = tChannelID::FromString(chid);
    if(!id.Valid()) {
        esyslog("xmlapi: stream -> invalid chid given");
        return this->handle404Error();
    }
    
    string channelId(chid);
    
    cStreamer *streamer = new cStreamer(this->config, preset, channelId);
    if(!streamer->StartFFmpeg())
    {
        delete streamer;
        return MHD_NO;
    }
    dsyslog("xmlapi: Stream started");
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
    streamer->StopFFmpeg();
    delete streamer;
    dsyslog("xmlapi: Stream stopped");
}

int cRequestHandler::handlePresets() {
    string ini;
    if(this->presets.size() != 0)
    {
        for(map<string,cPreset>::iterator it = presets.begin(); 
                it != presets.end(); ++it) {
            ini += "[" + it->first + "]\n";
            ini += "Cmd=" + it->second.GetCmd() + "\n";
            ini += "MimeType=" + it->second.MimeType() + "\n";
            ini += "Ext=" + it->second.Extension() + "\n\n";
        }
    } else {
        cPreset dp = presets.GetDefaultPreset();
        ini += "[Default]\n";
        ini += "Cmd=" + dp.GetCmd() + "\n";
        ini += "MimeType=" + dp.MimeType() + "\n";
        ini += "Ext=" + dp.Extension() + "\n\n";
    }
    char *page = (char *)malloc((ini.length() + 1) * sizeof(char));
    strcpy(page, ini.c_str());
    struct MHD_Response *response;
    int ret;
    
    response = MHD_create_response_from_buffer (strlen (page), 
                                               (void *) page, 
                                               MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header (response, "Content-Type", "text/plain");
    ret = MHD_queue_response(this->connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    return ret;
    
}

int cRequestHandler::handleChannels() {
    string xml = this->channelsToXml();
    struct MHD_Response *response;
    int ret;
    char *page = (char *)malloc((xml.length() + 1) *sizeof(char));
    strcpy(page, xml.c_str());
    response = MHD_create_response_from_buffer (strlen (page), 
                                               (void *) page, 
                                               MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header (response, "Content-Type", "text/xml");
    ret = MHD_queue_response(this->connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    return ret;
}

string cRequestHandler::channelsToXml() {
    string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    xml +=       "<groups>\n";
    string group = "Unsorted";
    bool firstgroup = true;
    for(int i=0; i<Channels.Count(); i++) {
        cChannel *channel = Channels.Get(i);
        if(channel->GroupSep() && firstgroup) {
            group = channel->Name();
            xml += "    <group name=\"" + group + "\">\n";
            firstgroup = false;
            continue;
        }
        if(channel->GroupSep() && !firstgroup) {
            xml += "    </group>\n";
            group = channel->Name();
            xml += "    <group name=\"" + group + "\">\n";
            continue;
        }
        if(!channel->GroupSep() && i == 0) {
            xml += "    <group name=\"" + group + "\">\n";
            firstgroup = false;
        }
        xml += "        <channel id=\"" + 
                string(channel->GetChannelID().ToString()) +
                "\">\n";
        string name = channel->Name();
        string shortname = channel->ShortName();
        xmlEncode(name);
        xmlEncode(shortname);
        xml += "            <name>" + name + "</name>\n";
        xml += "            <shortname>" + shortname + "</shortname>\n";
        xml += "            <logo>/logos/" + name + ".png</logo>\n";
        xml += "        </channel>\n";
    }
    xml += "    </group>\n";
    xml += "</groups>\n";
    return xml;
}

int cRequestHandler::handleEPG() {
    struct MHD_Response *response;
    int ret;
    const char* chid = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "chid");
    if(chid == NULL) {
        esyslog("xmlapi: epg.xml -> No chid given.");
        return this->handle404Error();
    }
    bool now = false;
    bool next = false;
    bool attime = false;
    time_t at_time = 0;
    
    const char *at = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "at");
    if(at != NULL) {
        if (0 == strcmp(at, "now")) {
            now = true;
        } else if (0 == strcmp(at, "next")) {
            next = true;
        } else {
            at_time = atol(at);
            attime = true;
        }
    }
    
    if(MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "next") != NULL) {
        next = true;
    }
    
    cSchedulesLock lock;
    const cSchedules *schedules = cSchedules::Schedules(lock);
    tChannelID cid = tChannelID::FromString(chid);
    if(!cid.Valid()) {
        esyslog("xmlapi: epg.xml -> invalid chid given");
        return this->handle404Error();
    }
    const cSchedule *schedule = schedules->GetSchedule(cid);
    const cList<cEvent> *events = schedule->Events();
    string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    xml +=       "<events>\n";
    
    for(int i=0; i<events->Count(); i++) {
        cEvent *event = NULL;
        if(now) {
            event = const_cast<cEvent *>(schedule->GetPresentEvent());
            i = events->Count();
        } else if (next) {
            event = const_cast<cEvent *>(schedule->GetFollowingEvent());
            i = events->Count();
        } else if (attime) {
            event = const_cast<cEvent *>(schedule->GetEventAround(at_time));
            i = events->Count();
        } else {
            event = events->Get(i);
        }
        if(event == NULL) {
            continue;
        }
        const char *t = event->Title();
        const char *s = event->ShortText();
        const char *d = event->Description();

        string title;
        if(t != NULL)
            title = string(event->Title());
        string shorttext;
        if(s != NULL)
            shorttext = string(event->ShortText());
        string descr;
        if(d != NULL)
            descr = string(event->Description());
        xmlEncode(title);
        xmlEncode(shorttext);
        xmlEncode(descr);
        xml += "  <event id=\"" + uint32ToString(event->EventID()) + "\">\n";
        xml += "    <channelid>" + string(event->ChannelID().ToString()) + "</channelid>\n";
        xml += "    <title>" + title + "</title>\n";
        xml += "    <shorttext>" + shorttext + "</shorttext>\n";
        xml += "    <description>" + descr + "</description>\n";
        xml += "    <start>" + timeToString(event->StartTime()) + "</start>\n";
        xml += "    <stop>" + timeToString(event->EndTime()) + "</stop>\n";
        xml += "    <duration>" + intToString(event->Duration()) + "</duration>\n";
        xml += "  </event>\n";
    }
    xml += "</events>\n";
    char *page = (char *)malloc((xml.length()+1) * sizeof(char));
    strcpy(page, xml.c_str());
    response = MHD_create_response_from_buffer (strlen (page), 
                                               (void *) page, 
                                               MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header (response, "Content-Type", "text/xml");
    ret = MHD_queue_response(this->connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    return ret;
}

int cRequestHandler::handle404Error() {
    struct MHD_Response *response;
    int ret;
    const char *page = "<html>\n"
                       "  <head>\n"
                       "    <title>404 Not Found</title>\n"
                       "  </head>\n"
                       "  <body>\n"
                       "    <h1>Not Found</h1>\n"
                       "    <p>The requested URL was not found on this server.</p>\n"
                       "  </body>\n"
                       "</html>\n";
    response = MHD_create_response_from_buffer (strlen (page), 
                                               (void *) page, 
                                               MHD_RESPMEM_PERSISTENT);
    MHD_add_response_header (response, "Content-Type", "text/html");
    ret = MHD_queue_response(this->connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response (response);
    return ret;
}