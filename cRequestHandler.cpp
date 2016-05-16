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
#include <vector>
#include <algorithm>
#include <ctime>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vdr/tools.h>
#include <vdr/channels.h>
#include <vdr/recording.h>
#include <vdr/epg.h>
#include "cRequestHandler.h"
#include "cStream.h"
#include "helpers.h"
#include "streamControl.h"

cRequestHandler::cRequestHandler(struct MHD_Connection *connection, 
                                    cDaemonParameter *daemonParameter)
    : connection(connection), daemonParameter(daemonParameter), 
        config(daemonParameter->GetPluginConfig()), 
        presets(daemonParameter->GetPluginConfig().GetPresetsFile()) {    
}

cRequestHandler::~cRequestHandler() {
}

int cRequestHandler::HandleRequest(const char* url) {

    const MHD_ConnectionInfo *connectionInfo = MHD_get_connection_info (connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS);
    if (connectionInfo->client_addr->sa_family == AF_INET)
    {
        struct sockaddr_in *sin = (struct sockaddr_in *) connectionInfo->client_addr;
        char *ip = inet_ntoa(sin->sin_addr);
        if(ip != NULL)
        {
            this->conInfo.insert(pair<string,string>("ClientIP", string(ip)));
        }
    }
    const char *useragent = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "User-Agent");
    if(useragent != NULL)
    {
        this->conInfo.insert(pair<string,string>("User-Agent",string(useragent)));
    }
    const char *host = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "Host");
    if(host != NULL)
    {
        this->conInfo.insert(pair<string,string>("Host", string(host)));
    }
    
    if(0 == strcmp(url, "/version.xml"))
    {
        return this->handleVersion();
    }
    else if (startswith(url, "/stream"))
    {
        if (0 == strcmp(url, "/streamcontrol.xml"))
            return this->handleStreamControl();
        return this->handleStream(url);
    }
    else if (startswith(url, "/recstream")) {
        return this->handleRecStream(url);
    }
    else if (startswith(url, "/logos/") && endswith(url, ".png")) {
        return this->handleLogos(url);
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
    else if (0 == strcmp(url, "/recordings.xml")) {
        return this->handleRecordings();
    }
    else if (0 == strcmp(url, "/deletedrecordings.xml")) {
        return this->handleDeletedRecordings();
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
    string fullurl = this->config.GetStreamdevUrl() + channelId + ".ts";
    string ffmpegcmd = preset.FFmpegCmd(this->config.GetFFmpeg(), fullurl);
    dsyslog("xmlapi: FFmpeg Cmd=%s\n", ffmpegcmd.c_str());
    cStream *stream = new cStream(ffmpegcmd, this->conInfo);
    if(this->config.GetWaitForFFmpeg()) {
        StreamControl->WaitingForStreamsByUserAgentAndIP(this->conInfo["ClientIP"], this->conInfo["User-Agent"]);
        sleep(1);
    }
    int *streamid = new int;
    
    *streamid = StreamControl->AddStream(stream);
    if(!stream->StartFFmpeg())
    {
        StreamControl->RemoveStream(*streamid);
        delete streamid;
        return MHD_NO;
    }
    dsyslog("xmlapi: Stream started");
    response = MHD_create_response_from_callback (MHD_SIZE_UNKNOWN,
                                                8*1024,
                                                &cRequestHandler::stream_reader,
                                                streamid,
                                                &cRequestHandler::clear_stream);
    MHD_add_response_header (response, "Content-Type", preset.MimeType().c_str());
    MHD_add_response_header (response, "Cache-Control", "no-cache");
    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    return ret;
}

int cRequestHandler::handleRecStream(const char* url) {
    const char* cstr_preset = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "preset");
    if(cstr_preset == NULL)
    {
        esyslog("xmlapi: stream -> No preset given!");
        return this->handle404Error();
    }
    string ps(cstr_preset);
    cPreset preset = this->presets[ps];
    string expectedUrl = "/recstream" + preset.Extension();
    if(0 != strcmp(url, expectedUrl.c_str())) {
        esyslog("xmlapi: Url %s doen't ends with recstream%s", url, expectedUrl.c_str());
        return this->handle404Error();
    }
    const char* recfile = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "filename");
    if(recfile == NULL)
    {
        esyslog("xmlapi: stream -> No file name given!");
        return this->handle404Error();
    }
    dsyslog("xmlapi: request %s?filename=%s&preset=%s", url, recfile, cstr_preset);
    cRecording *rec = Recordings.GetByName(recfile);
    if(rec == NULL) {
        dsyslog("xmlapi: No recording found with file name '%s'", recfile);
        return this->handle404Error();
    }
    const char* cstr_start = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "start");
    int starttime = 0;
    if(cstr_start != NULL) {
        starttime = atoi(cstr_start);
    }
    string recfiles = string(rec->FileName()) + "/*.ts";
    string input = "concat:$(ls -1 " + recfiles + " | perl -0pe 's/\\n/|/g;s/\\|$//g')";
    string ffmpegcmd = preset.FFmpegCmd(this->config.GetFFmpeg(), input, starttime);
    dsyslog("xmlapi: FFmpeg Cmd=%s\n", ffmpegcmd.c_str());
    
    struct MHD_Response *response;
    int ret;
    cStream *stream = new cStream(ffmpegcmd, this->conInfo);
    if(this->config.GetWaitForFFmpeg()) {
        StreamControl->WaitingForStreamsByUserAgentAndIP(this->conInfo["ClientIP"], this->conInfo["User-Agent"]);
        sleep(1);
    }
    int *streamid = new int;
    
    *streamid = StreamControl->AddStream(stream);
    if(!stream->StartFFmpeg())
    {
        StreamControl->RemoveStream(*streamid);
        delete streamid;
        return MHD_NO;
    }
    dsyslog("xmlapi: Stream started");
    response = MHD_create_response_from_callback (MHD_SIZE_UNKNOWN,
                                                8*1024,
                                                &cRequestHandler::stream_reader,
                                                streamid,
                                                &cRequestHandler::clear_stream);
    MHD_add_response_header (response, "Content-Type", preset.MimeType().c_str());
    MHD_add_response_header (response, "Cache-Control", "no-cache");
    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    return ret;
    
}

ssize_t cRequestHandler::stream_reader(void* cls, uint64_t pos, char* buf, size_t max) {
    int *streamid = (int *)cls;
    cStream *stream = StreamControl->GetStream(*streamid);
    return stream->Read(buf, max);    
}

void cRequestHandler::clear_stream(void* cls) {
    int *streamid = (int *)cls;
    StreamControl->Mutex.Lock();
    cStream *stream = StreamControl->GetStream(*streamid);
    if(stream != NULL)
    {
        stream->StopFFmpeg();
        StreamControl->RemoveStream(*streamid);
    }
    StreamControl->Mutex.Unlock();
    delete streamid;
    dsyslog("xmlapi: Stream stopped");
}

int cRequestHandler::handleStreamControl() {
    
    const char* removeid = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "remove");
    if(removeid != NULL) 
    {
        int streamid = atoi(removeid);
        StreamControl->RemoveStream(streamid);
    }
    string xml = StreamControl->GetStreamsXML();
    struct MHD_Response *response;
    int ret;
    char *page = (char *)malloc((xml.length() + 1) *sizeof(char));
    strcpy(page, xml.c_str());
    response = MHD_create_response_from_buffer (strlen (page), 
                                               (void *) page, 
                                               MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header (response, "Content-Type", "text/xml");
    MHD_add_response_header (response, "Cache-Control", "no-cache");
    ret = MHD_queue_response(this->connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    return ret;
    
        
}

int cRequestHandler::handleLogos(const char* url) {
    int ret;
    int fd;
    struct stat sbuf;
    struct MHD_Response *response;
    string logofile = this->config.GetConfigDir() + url;
    
    if ( (-1 == (fd = open (logofile.c_str(), O_RDONLY))) ||
       (0 != fstat (fd, &sbuf)) ) {
        if (fd != -1)
            close (fd);
        return this->handle404Error();
    }
    response = MHD_create_response_from_fd(sbuf.st_size, fd);
    MHD_add_response_header (response, "Content-Type", "image/png");
    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    
    return ret;
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
    string logourl = "";
    string host = this->conInfo["Host"];
    if(host != "")
    {
        if(this->daemonParameter->GetDaemonPort() == this->config.GetHttpsPort())
        {
            logourl += "https://" + host;
        }
        else
        {
            logourl += "http://" + host;
        }
    }
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
        if(channel->Vpid() == 0 || channel->Vpid() == 1) {
            xml += "        <isradio>true</isradio>\n";
        }
        else
        {
            xml += "        <isradio>false</isradio>\n";
        }
        string name = channel->Name();
        string shortname = channel->ShortName();
        string logo = name;
        transform(logo.begin(), logo.end(), logo.begin(), ::tolower);
        replace(logo.begin(), logo.end(), '/', '-');
        logo = urlEncode(logo);
        xmlEncode(name);
        xmlEncode(shortname);
        xmlEncode(logo);
        xml += "            <name>" + name + "</name>\n";
        xml += "            <shortname>" + shortname + "</shortname>\n";
        xml += "            <logo>" + logourl + "/logos/" + logo + ".png</logo>\n";
        xml += "        </channel>\n";
    }
    xml += "    </group>\n";
    xml += "</groups>\n";
    return xml;
}

int cRequestHandler::handleRecordings() {
    
    const char *recfile = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "filename");
    const char *action = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "action");
    string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    
    if(recfile != NULL && action != NULL) {
        xml += "<actions>\n";
        cRecording *rec = Recordings.GetByName(recfile);
        if(rec == NULL) {
            return this->handle404Error();
        }
        if(0 == strcmp(action, "delete")) {
            if(rec->Delete())
            {
                xml += "    <delete>true</delete>\n";
            }
            else {
                xml += "    <delete>false</delete>\n";
            }
        }
        else {
            xml += "    <unknown>" + string(action) + "</unknown>\n";
        }
        xml += "</actions>\n";
    }
    else {
        xml = this->recordingsToXml();
    }

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

int cRequestHandler::handleDeletedRecordings() {
    
    const char *recfile = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "filename");
    const char *action = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "action");
    string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    
    if(recfile != NULL && action != NULL) {
        xml += "<actions>\n";
        cRecording *rec = DeletedRecordings.GetByName(recfile);
        if(rec == NULL) {
            return this->handle404Error();
        }
        if(0 == strcmp(action, "undelete")) {
            if(rec->Undelete())
            {
                xml += "    <undelete>true</undelete>\n";
            }
            else {
                xml += "    <undelete>false</undelete>\n";
            }     
        }
        else if(0 == strcmp(action, "remove")) {
            if(rec->Remove())
            {
                xml += "    <remove>true</remove>\n";
            }
            else {
                xml += "    <remove>false</remove>\n";
            }
        }
        else {
            xml += "    <unknown>" + string(action) + "</unknown>\n";
        }
        xml += "</actions>\n";
    }
    else {
        xml = this->recordingsToXml(true);
    }

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

string cRequestHandler::recordingsToXml(bool deleted) {
    string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    xml +=       "<recordings>\n";
    cRecordings *recs = deleted ? &DeletedRecordings : &Recordings;
    if(recs->Load())
    {
        for(int i=0; i<recs->Count(); i++)
        {
            cRecording *rec = recs->Get(i);
            const cRecordingInfo *info = rec->Info();
            string name = string(rec->Name() ? rec->Name() : "");
            string filename = string(rec->FileName() ? rec->FileName() : "");
            string title = string(rec->Title() ? rec->Title() : "");
            string inuse = rec->IsInUse() > 0 ? "true" : "false";
            string duration = intToString(rec->LengthInSeconds());
            string filesize = intToString(rec->FileSizeMB());
            string deleted = timeToString(rec->Deleted());
            string ichannelid = string(info->ChannelID().ToString());
            string ichannelname = string(info->ChannelName());
            string ititle = string(info->Title() ? info->Title() : "");
            string ishorttext = string(info->ShortText() ? info->ShortText() : "");
            string idescription = string(info->Description() ? info->Description() : "");
            xmlEncode(name);
            xmlEncode(filename);
            xmlEncode(title);
            xmlEncode(ichannelid);
            xmlEncode(ichannelname);
            xmlEncode(ititle);
            xmlEncode(ishorttext);
            xmlEncode(idescription);
            xml += "    <recording>\n";
            xml += "        <name>" + name + "</name>\n";
            xml += "        <filename>" + filename + "</filename>\n";
            xml += "        <title>" + title + "</title>\n";
            xml += "        <inuse>" + inuse + "</inuse>\n";
            xml += "        <size>" + filesize + "</size>\n";
            xml += "        <duration>" + duration + "</duration>\n";
            xml += "        <deleted>" + deleted + "</deleted>\n";
            xml += "        <infos>\n";
            xml += "            <channelid>" + ichannelid + "</channelid>\n";
            xml += "            <channelname>" + ichannelname + "</channelname>\n";
            xml += "            <title>" + ititle + "</title>\n";
            xml += "            <shorttext>" + ishorttext + "</shorttext>\n";
            xml += "            <description>" + idescription + "</description>\n";
            xml += "        </infos>\n";
            xml += "    </recording>\n";         
        }
    }
    xml += "</recordings>\n";
    return xml;
}

int cRequestHandler::handleEPG() {
    struct MHD_Response *response;
    int ret;
    string xml;
    const char* chid = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "chid");
    const char* at = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "at");
    const char* cstr_search = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "search");
    const char* cstr_options = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "options");
    
    if(chid != NULL && cstr_search == NULL)
    {
        xml = this->eventsToXml(chid, at);
    }
    else {
        string search = "";
        string options = "";
        if(cstr_search != NULL)
            search = string(cstr_search);
        if(cstr_options != NULL)
            options = string(cstr_options);
        xml = this->searchEventsToXml(chid, search, options);
    }
    char *page = (char *)malloc((xml.length()+1) * sizeof(char));
    strcpy(page, xml.c_str());
    response = MHD_create_response_from_buffer (strlen (page), 
                                               (void *) page, 
                                               MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header (response, "Content-Type", "text/xml");
    MHD_add_response_header (response, "Cache-Control", "no-cache");
    ret = MHD_queue_response(this->connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    return ret;
}

string cRequestHandler::eventsToXml(const char* chid, const char *at) {
    string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    xml +=       "<events>\n";
    if(chid != NULL)
    {
        tChannelID cid = tChannelID::FromString(chid);
        if(cid.Valid()) {
            bool now = false;
            bool next = false;
            bool attime = false;
            time_t at_time = 0;
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
            
            cSchedulesLock lock;
            const cSchedules *schedules = cSchedules::Schedules(lock);
            const cSchedule *schedule = schedules->GetSchedule(cid);
            if(schedule != NULL) {
                const cList<cEvent> *events = schedule->Events();
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
            }
        }
        else {
            dsyslog("xmlapi: epg.xml -> invalid channel id");
        }          
    }
    xml += "</events>\n";
    return xml;
    
}

string cRequestHandler::searchEventsToXml(const char* chid, string search, string options) {
    string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    xml +=       "<events>\n";
    
    
    if(chid != NULL)
    {
        tChannelID cid = tChannelID::FromString(chid);
        if(!cid.Valid())
            chid = NULL;
    }
    cSchedulesLock lock;
    const cSchedules *schedules = cSchedules::Schedules(lock);
    for(int i=0; i<schedules->Count(); i++) {
        const cSchedule *schedule;
        if(chid != NULL)
        {
            tChannelID cid = tChannelID::FromString(chid);
            if(!cid.Valid())
                break;
            schedule = schedules->GetSchedule(cid);
            i = schedules->Count();
        }
        else {
            schedule = schedules->Get(i);
        }
        if(schedule != NULL) {
            const cList<cEvent> *events = schedule->Events();
            for(int j=0; j<events->Count(); j++) {
                cEvent *event = events->Get(j);
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
                
                if(search != "") {
                    if(options == "" || options == "T")
                    {
                        if(!searchInString(title, search))
                            continue;
                    }
                    else if(options == "S") {
                        if(!searchInString(shorttext, search))
                            continue;
                    }
                    else if(options == "D") {
                        if(!searchInString(descr, search))
                            continue;
                    }
                    else if(options == "TS" || options == "ST") {
                        if( (!searchInString(title, search)) && (!searchInString(shorttext, search)) )
                            continue;
                    }
                    else if(options == "TD" || options == "DT") {
                        if( (!searchInString(title, search)) && (!searchInString(descr, search)) )
                            continue;
                    }
                    else if(options == "SD" || options == "DS") {
                        if( (!searchInString(shorttext, search)) && (!searchInString(descr, search)) )
                            continue;
                    }
                    else if(options == "TSD" || options == "TDS" || options == "SDT" || options == "STD" || options == "DTS" || options == "DST") {
                        if( (!searchInString(title, search)) && (!searchInString(shorttext, search)) && (!searchInString(descr, search)) )
                            continue;
                    }
                }
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
        }
    }
    
    
    xml += "</events>\n";
    return xml;
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