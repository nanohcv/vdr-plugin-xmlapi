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
#include <sstream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vdr/device.h>
#include <vdr/tools.h>
#include <vdr/channels.h>
#include <vdr/recording.h>
#include <vdr/epg.h>
#include <vdr/timers.h>
#include "cRequestHandler.h"
#include "cStream.h"
#include "helpers.h"
#include "globals.h"
#include "cResponseHeader.h"
#include "cResponseVersion.h"
#include "cResponseChannels.h"
#include "cResponsePresets.h"
#include "cResponseEpg.h"
#include "cResponseLogo.h"
#include "cResponseHlsStream.h"
#include "cResponseStreamControl.h"
#include "cSession.h"

cRequestHandler::cRequestHandler(struct MHD_Connection *connection,
                                    cDaemonParameter *daemonParameter)
    : connection(connection), daemonParameter(daemonParameter),
        config(daemonParameter->GetPluginConfig()),
        presets(daemonParameter->GetPluginConfig().GetPresetsFile()),
        extHeaders(daemonParameter->GetPluginConfig().GetWebSrvHeadersFile()), auth(NULL) {
    this->initRemoteKeys();
}

cRequestHandler::~cRequestHandler() {
	delete this->auth;
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



    //***************************************************


    this->auth = new cAuth(this->connection, this->config);

    if (!this->auth->authenticated()) return this->handleNotAuthenticated();

    if(0 == strcmp(url, "/version.xml"))
    {
    	cResponseVersion response(this->connection, this->auth->Session(), this->daemonParameter);
    	return response.toXml();
    }

    else if (startswith(url, "/stream"))
    {

        if(!this->auth->User().Rights().Streaming()) {
            dsyslog("xmlapi: The user %s don't have the permission to access %s", this->auth->User().Name().c_str(), url);
        	return this->GetErrorHandler().handle403Error();
        }
    	if (0 == strcmp(url, "/streamcontrol.xml")) {

    		cResponseStreamControl response(this->connection, this->auth->Session(), this->daemonParameter);
    		return response.toXml();
    	} else {
    		return this->handleStream(url);
    	}
    }

    else if (startswith(url, "/recstream")) {

        if(!this->auth->User().Rights().Streaming()) {
            dsyslog("xmlapi: The user %s don't have the permission to access %s", this->auth->User().Name().c_str(), url);
        	return this->GetErrorHandler().handle403Error();
        }
        return this->handleRecStream(url);
    }
    else if (startswith(url, "/hls/")) {

        if(!this->auth->User().Rights().Streaming()) {
            dsyslog("xmlapi: The user %s don't have the permission to access %s", this->auth->User().Name().c_str(), url);
        	return this->GetErrorHandler().handle403Error();
        }

        cResponseHlsStream response(this->connection, this->auth->Session(), this->daemonParameter);
        return response.respond(url);
    }
    else if (startswith(url, "/logos/") && endswith(url, ".png")) {


    	cResponseLogo response(this->connection, this->auth->Session(), this->daemonParameter);
    	int ret = response.toImage(url);

        if (ret == MHD_HTTP_NOT_FOUND) return this->handle404Error();
        return ret;
    }
    else if (0 == strcmp(url, "/presets.ini")) {
    	cResponsePresets response(this->connection, this->auth->Session(), this->daemonParameter);
    	return response.toIni();
    }
    else if (0 == strcmp(url, "/channels.xml")) {
    	cResponseChannels response(this->connection, this->auth->Session(), this->daemonParameter);
    	return response.toXml();
    }
    else if (0 == strcmp(url, "/epg.xml")) {
    	cResponseEpg response(this->connection, this->auth->Session(), this->daemonParameter);
    	return response.toXml();
    }
    else if (0 == strcmp(url, "/recordings.xml")) {
        return this->handleRecordings();
    }
    else if (0 == strcmp(url, "/deletedrecordings.xml")) {
        return this->handleDeletedRecordings();
    }
    else if (0 == strcmp(url, "/timers.xml")) {
        return this->handleTimers();
    }
    else if (0 == strcmp(url, "/switch.xml")) {
        return this->handleSwitchToChannel();
    }
    else if (0 == strcmp(url, "/remote.xml")) {
        return this->handleRemote();
    }
    else if (0 == strcmp(url, "/rights.xml")) {
        return this->handleRights();
    }
    else if (startswith(url, "/websrv/")) {
        return this->handleWebSrv(url);
    }
    else {

    	return this->GetErrorHandler().handle404Error();
    }
    return MHD_NO;
}

cResponseHandler cRequestHandler::GetErrorHandler() {

	cSession *session = NULL;
	cResponseHandler response(connection, session, this->daemonParameter);
	return response;
};

int cRequestHandler::handleStream(const char *url) {
    if(!this->user.Rights().Streaming()) {
        dsyslog("xmlapi: The user %s don't have the permission to access %s", this->user.Name().c_str(), url);
        return this->handle403Error();
    }
        
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
    cStream *stream = new cStream(this->config.GetFFmpeg(), preset, this->conInfo);
    if(this->config.GetWaitForFFmpeg()) {
        StreamControl->WaitingForStreamsByUserAgentAndIP(this->conInfo["ClientIP"], this->conInfo["User-Agent"]);
        sleep(1);
    }
    int *streamid = new int;

    *streamid = StreamControl->AddStream(stream);
    string channelId(chid);
    string input = this->config.GetStreamdevUrl() + channelId + ".ts";
    if(!stream->StartFFmpeg(input))
    {
        StreamControl->RemoveStream(*streamid);
        delete streamid;
        return this->handle404Error();
    }
    dsyslog("xmlapi: Stream started");
    response = MHD_create_response_from_callback (MHD_SIZE_UNKNOWN,
                                                8*1024,
                                                &cRequestHandler::stream_reader,
                                                streamid,
                                                &cRequestHandler::clear_stream);
    MHD_add_response_header (response, "Content-Type", preset.MimeType().c_str());
    MHD_add_response_header (response, "Cache-Control", "no-cache");
    MHD_add_response_header (response, "Access-Control-Allow-Origin", "*");
    MHD_add_response_header (response, "Access-Control-Allow-Headers", "Authorization");
    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    return ret;
}

int cRequestHandler::handleRecStream(const char* url) {
    if(!this->user.Rights().Streaming()) {
        dsyslog("xmlapi: The user %s don't have the permission to access %s", this->user.Name().c_str(), url);
        return this->handle403Error();
    }
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
    string recfiles = "'" + string(rec->FileName()) + "/'*.ts";
    string input = "concat:$(ls -1 " + recfiles + " | perl -0pe 's/\\n/|/g;s/\\|$//g')";

    struct MHD_Response *response;
    int ret;
    cStream *stream = new cStream(this->config.GetFFmpeg(), preset, this->conInfo);
    if(this->config.GetWaitForFFmpeg()) {
        StreamControl->WaitingForStreamsByUserAgentAndIP(this->conInfo["ClientIP"], this->conInfo["User-Agent"]);
        sleep(1);
    }
    int *streamid = new int;

    *streamid = StreamControl->AddStream(stream);
    if(!stream->StartFFmpeg(input, starttime))
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
    MHD_add_response_header (response, "Access-Control-Allow-Origin", "*");
    MHD_add_response_header (response, "Access-Control-Allow-Headers", "Authorization");
    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    return ret;

}

ssize_t cRequestHandler::stream_reader(void* cls, uint64_t pos, char* buf, size_t max) {
    int *streamid = (int *)cls;
    StreamControl->Mutex.Lock();
    cStream *stream = (cStream*)StreamControl->GetStream(*streamid);
    ssize_t size = stream->Read(buf, max);
    StreamControl->Mutex.Unlock();
    return size;
}

void cRequestHandler::clear_stream(void* cls) {
    int *streamid = (int *)cls;
    StreamControl->Mutex.Lock();
    cStream *stream = (cStream*)StreamControl->GetStream(*streamid);
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

    if(!this->user.Rights().StreamControl()) {
        dsyslog("xmlapi: The user %s don't have the permission to access %s", this->user.Name().c_str(), "/streamcontrol.xml");
        return this->handle403Error();
    }
    
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

int cRequestHandler::handleRecordings() {

    const char *recfile = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "filename");
    const char *action = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "action");
    string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";

    if(recfile != NULL && action != NULL) {
        if(!this->user.Rights().Recordings()) {
            dsyslog("xmlapi: The user %s don't have the permission to do any action on /recordings.xml", this->user.Name().c_str());
            return this->handle403Error();
        }
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
    MHD_add_response_header (response, "Access-Control-Allow-Origin", "*");
    MHD_add_response_header (response, "Access-Control-Allow-Headers", "Authorization");
    ret = MHD_queue_response(this->connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    return ret;
}

int cRequestHandler::handleDeletedRecordings() {

    const char *recfile = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "filename");
    const char *action = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "action");
    string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";

    if(recfile != NULL && action != NULL) {
        if(!this->user.Rights().Recordings()) {
            dsyslog("xmlapi: The user %s don't have the permission to do any action on /deletedrecordings.xml", this->user.Name().c_str());
            return this->handle403Error();
        }
            
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
    MHD_add_response_header (response, "Access-Control-Allow-Origin", "*");
    MHD_add_response_header (response, "Access-Control-Allow-Headers", "Authorization");
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
            string ichannelname = string(info->ChannelName() ? info->ChannelName() : "");
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

int cRequestHandler::handleTimers() {
    string xml = "";
    const char *action = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "action");
    if(action != NULL) {
        if(!this->user.Rights().Timers()) {
            dsyslog("xmlapi: The user %s don't have the permission to do any action on /timers.xml", this->user.Name().c_str());
            return this->handle403Error();
        }
        if(0 == strcmp(action, "delete")) {
            const char *tid = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "id");
            bool deleted = this->deleteTimer(tid);
            xml += "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
            if(deleted)
                xml += "<deleted>true</deleted>\n";
            else
                xml += "<deleted>false</deleted>\n";
        }
        else if(0 == strcmp(action, "onoff")) {
            const char *tid = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "id");
            bool onOff = this->onOffTimer(tid);
            xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
            if(onOff) {
                xml += "<onoff>successful</onoff>\n";
            }
            else {
                xml += "<onoff>failed</onoff>\n";
            }
        }
        else if(0 == strcmp(action, "add")) {
            const char *eventid = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "eventid");
            const char *channelid = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "chid");
            const char *name = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "name");
            const char *aux = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "aux");
            const char *cstr_flags = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "flags");
            const char *cstr_weekdays = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "weekdays");
            const char *cstr_day = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "day");
            const char *cstr_start = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "start");
            const char *cstr_stop = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "stop");
            const char *cstr_priority = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "priority");
            const char *cstr_lifetime = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "lifetime");
            if(eventid != NULL && channelid != NULL &&
                    name == NULL && aux == NULL && cstr_flags == NULL &&
                    cstr_weekdays == NULL && cstr_day == NULL && cstr_start == NULL &&
                    cstr_stop == NULL && cstr_priority == NULL && cstr_lifetime == NULL) {
                bool added = this->addTimer(channelid, eventid);
                xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
                if(added)
                    xml += "<added>true</added>\n";
                else
                    xml += "<added>false</added>\n";
            }
            else if(channelid != NULL && name != NULL && aux != NULL &&
                    cstr_flags != NULL && cstr_weekdays != NULL &&
                    cstr_day != NULL && cstr_start != NULL && cstr_stop != NULL &&
                    cstr_priority != NULL && cstr_lifetime != NULL && eventid == NULL) {
                bool added = this->addTimer(channelid, name, aux, cstr_flags,
                        cstr_weekdays, cstr_day, cstr_start, cstr_stop,
                        cstr_priority, cstr_lifetime);
                xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
                if(added)
                    xml += "<added>true</added>\n";
                else
                    xml += "<added>false</added>\n";
            }
            else {
                xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
                xml += "<error>incorrect parameters</error>\n";
            }
        }
        else {
            xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
            xml = "<unknownaction>" + string(action) + "</unknownaction>\n";
        }
    }
    else {
        xml = this->timersToXml();
    }
    struct MHD_Response *response;
    int ret;
    char *page = (char *)malloc((xml.length() + 1) *sizeof(char));
    strcpy(page, xml.c_str());
    response = MHD_create_response_from_buffer (strlen (page),
                                               (void *) page,
                                               MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header (response, "Content-Type", "text/xml");
    MHD_add_response_header (response, "Access-Control-Allow-Origin", "*");
    MHD_add_response_header (response, "Access-Control-Allow-Headers", "Authorization");
    ret = MHD_queue_response(this->connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    return ret;
}

string cRequestHandler::timersToXml() {
    string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    xml +=       "<timers>\n";

    for(int i=0; i<Timers.Count(); i++) {
        cTimer *timer = Timers.Get(i);
        ostringstream builder;
        builder << string(timer->Channel()->GetChannelID().ToString()) << ":" << timer->WeekDays() << ":"
				<< timer->Day() << ":" << timer->Start() << ":" << timer->Stop();
        string id = builder.str();
        string chid = string(timer->Channel()->GetChannelID().ToString());
        string chname = string(timer->Channel()->Name() ? timer->Channel()->Name() : "");
        string name = string(timer->File() ? timer->File() : "");
        string aux = string(timer->Aux() ? timer->Aux() : "");
        string flags = uint32ToString(timer->Flags());
        string weekdays = intToString(timer->WeekDays());
        string day = timeToString(timer->Day());
        string start = intToString(timer->Start());
        string stop = intToString(timer->Stop());
        string priority = intToString(timer->Priority());
        string lifetime = intToString(timer->Lifetime());
        xmlEncode(id);
        xmlEncode(chid);
        xmlEncode(chname);
        xmlEncode(name);
        xmlEncode(aux);

        xml += "    <timer id=\"" + id + "\">\n";
        xml += "        <channelid>" + chid + "</channelid>\n";
        xml += "        <channelname>" + chname + "</channelname>\n";
        xml += "        <name>" + name + "</name>\n";
        xml += "        <aux>" + aux + "</aux>\n";
        xml += "        <flags>" + flags + "</flags>\n";
        xml += "        <weekdays>" + weekdays + "</weekdays>\n";
        xml += "        <day>" + day + "</day>\n";
        xml += "        <start>" + start + "</start>\n";
        xml += "        <stop>" + stop + "</stop>\n";
        xml += "        <priority>" + priority + "</priority>\n";
        xml += "        <lifetime>" + lifetime + "</lifetime>\n";
        xml += "    </timer>\n";
    }

    xml += "</timers>\n";
    return xml;
}

cTimer *cRequestHandler::GetTimer(const char* tid) {
    if(tid == NULL)
        return NULL;
    for(int i=0; i<Timers.Count();i++)
    {
        cTimer *timer = Timers.Get(i);
        string id = string(tid);
        vector<string> parts = split(id, ':');
        if(parts.size() != 5) {
            return NULL;
        }
        tChannelID cid = tChannelID::FromString(parts[0].c_str());
        if(!cid.Valid())
            return NULL;
        int wdays = atoi(parts[1].c_str());
        time_t day = atol(parts[2].c_str());
        int start = atoi(parts[3].c_str());
        int stop = atoi(parts[4].c_str());
        if(timer->Channel()->GetChannelID() == cid &&
           timer->WeekDays() == wdays &&
           timer->Day() == day &&
           timer->Start() == start &&
           timer->Stop() == stop) {
            return timer;
        }
    }
    return NULL;
}

const cEvent * cRequestHandler::GetEvent(tChannelID channelid, tEventID eid) {
    cSchedulesLock lock;
    const cSchedules *schedules = cSchedules::Schedules(lock);
    const cSchedule *schedule = schedules->GetSchedule(channelid);
    if(schedule == NULL)
        return NULL;
    const cEvent *event = schedule->GetEvent(eid);
    return event;
}

bool cRequestHandler::deleteTimer(const char* tid) {
    cTimer *timer = this->GetTimer(tid);
    if(timer == NULL)
        return false;
    Timers.Del(timer);
    Timers.SetModified();
    return true;
}
bool cRequestHandler::onOffTimer(const char* tid) {
    cTimer *timer = this->GetTimer(tid);
    if(timer == NULL)
        return false;
    timer->OnOff();
    return true;
}

bool cRequestHandler::addTimer(const char *channelid, const char* eventid) {
    if(eventid == NULL || channelid == NULL)
        return false;
    tEventID eid = (tEventID)strtoul(eventid, NULL, 10);
    tChannelID cid = tChannelID::FromString(channelid);
    if(!cid.Valid())
        return false;
    const cEvent *event = this->GetEvent(cid, eid);
    if(event == NULL)
        return false;
    if(Timers.GetMatch(event) != NULL)
        return false;
    cTimer *timer = new cTimer(event);
    Timers.Add(timer);
    Timers.SetModified();
    return true;
}

bool cRequestHandler::addTimer(const char* channelid, const char* name,
        const char* aux, const char* cstr_flags, const char* cstr_weekdays,
        const char* cstr_day, const char* cstr_start, const char* cstr_stop,
        const char* cstr_priority, const char* cstr_lifetime) {
    tChannelID cid = tChannelID::FromString(channelid);
    if(!cid.Valid())
        return false;
    cChannel *channel = Channels.GetByChannelID(cid);
    if(channel == NULL)
        return false;
    unsigned int flags = (unsigned int)atoi(cstr_flags);
    int weekdays = atoi(cstr_weekdays);
    time_t day = (time_t)atol(cstr_day);
    int start = atoi(cstr_start);
    int stop = atoi(cstr_stop);
    int priority = atoi(cstr_priority);
    int lifetime = atoi(cstr_lifetime);
    cTimer *timer = new cTimer(false, false, channel);
    timer->SetFlags(flags);
    timer->SetFile(name);
    timer->SetAux(aux);
    timer->SetWeekDays(weekdays);
    timer->SetDay(day);
    timer->SetStart(start);
    timer->SetStop(stop);
    timer->SetPriority(priority);
    timer->SetLifetime(lifetime);
    if(Timers.GetTimer(timer) != NULL)
        return false;
    Timers.Add(timer);
    Timers.SetModified();
    return true;
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
    MHD_add_response_header (response, "Access-Control-Allow-Origin", "*");
    MHD_add_response_header (response, "Access-Control-Allow-Headers", "Authorization");
    ret = MHD_queue_response(this->connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response (response);
    return ret;
}

int cRequestHandler::handle403Error() {
    struct MHD_Response *response;
    int ret;
    const char *page = "<html>\n"
                       "  <head>\n"
                       "    <title>403 permission denied</title>\n"
                       "  </head>\n"
                       "  <body>\n"
                       "    <h1>Permission denied</h1>\n"
                       "    <p>You don't have the permission to access this site.</p>\n"
                       "  </body>\n"
                       "</html>\n";
    response = MHD_create_response_from_buffer (strlen (page),
                                               (void *) page,
                                               MHD_RESPMEM_PERSISTENT);
    MHD_add_response_header (response, "Content-Type", "text/html");
    MHD_add_response_header (response, "Access-Control-Allow-Origin", "*");
    MHD_add_response_header (response, "Access-Control-Allow-Headers", "Authorization");
    ret = MHD_queue_response(this->connection, MHD_HTTP_FORBIDDEN, response);
    MHD_destroy_response (response);
    return ret;
}

int cRequestHandler::handleSwitchToChannel() {
    if(!this->user.Rights().RemoteControl()) {
        dsyslog("xmlapi: The user %s don't have the permission to switch to an channel", this->user.Name().c_str());
        return this->handle403Error();
    }
    struct MHD_Response *response;
    int ret;
    string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    const char* chid = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "chid");
    if(chid == NULL) {
        return this->handle404Error();
    }
    tChannelID cid = tChannelID::FromString(chid);
    if(!cid.Valid()) {
        return this->handle404Error();
    }
    cChannel *channel = Channels.GetByChannelID(cid);
    if(channel == NULL) {
        xml += "<status>\n";
        xml += "    <channel>" + string(chid) + "</channel>\n";
        xml += "    <switched>false</switched>\n";
        xml += "    <message>Channel not found</message>\n";
        xml += "</status>\n";
    } else {
        bool switched = cDevice::PrimaryDevice()->SwitchChannel(channel, true);
        if(switched) {
            xml += "<status>\n";
            xml += "    <channel>" + string(chid) + "</channel>\n";
            xml += "    <switched>true</switched>\n";
            xml += "    <message>Switched to channel</message>\n";
            xml += "</status>\n";
        } else {
            xml += "<status>\n";
            xml += "    <channel>" + string(chid) + "</channel>\n";
            xml += "    <switched>false</switched>\n";
            xml += "    <message>Switching to channel failed</message>\n";
            xml += "</status>\n";
        }
    }
    char *page = (char *)malloc((xml.length()+1) * sizeof(char));
    strcpy(page, xml.c_str());
    response = MHD_create_response_from_buffer (strlen (page),
                                               (void *) page,
                                               MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header (response, "Content-Type", "text/xml");
    MHD_add_response_header (response, "Cache-Control", "no-cache");
    MHD_add_response_header (response, "Access-Control-Allow-Origin", "*");
    MHD_add_response_header (response, "Access-Control-Allow-Headers", "Authorization");
    ret = MHD_queue_response(this->connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    
    return ret;
}

int cRequestHandler::handleRemote() {
    if(!this->user.Rights().RemoteControl()) {
        dsyslog("xmlapi: The user %s don't have the permission to send a remote command", this->user.Name().c_str());
        return this->handle403Error();
    }
    struct MHD_Response *response;
    int ret;
    string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    const char* ckey = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "key");
    if(ckey == NULL) {
        return this->handle404Error();
    }
    string key(ckey);
    map<string, eKeys>::iterator it = this->remoteKeys.find(key);
    if(it != this->remoteKeys.end()) {
        cRemote::Put(it->second);
        xml += "<status>Ok</status>\n";
    } else {
        return this->handle404Error();
    }
    
    char *page = (char *)malloc((xml.length()+1) * sizeof(char));
    strcpy(page, xml.c_str());
    response = MHD_create_response_from_buffer (strlen (page),
                                               (void *) page,
                                               MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header (response, "Content-Type", "text/xml");
    MHD_add_response_header (response, "Cache-Control", "no-cache");
    MHD_add_response_header (response, "Access-Control-Allow-Origin", "*");
    MHD_add_response_header (response, "Access-Control-Allow-Headers", "Authorization");
    ret = MHD_queue_response(this->connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    
    
    return ret;
}

int cRequestHandler::handleRights() {
    struct MHD_Response *response;
    int ret;
    string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    xml += "<rights>\n";
    xml += "    <streaming>" + string(this->user.Rights().Streaming() ? "true" : "false") + "</streaming>\n";
    xml += "    <timers>" + string(this->user.Rights().Timers() ? "true" : "false") + "</timers>\n";
    xml += "    <recordings>" + string(this->user.Rights().Recordings() ? "true" : "false") + "</recordings>\n";
    xml += "    <remotecontrol>" + string(this->user.Rights().RemoteControl() ? "true" : "false") + "</remotecontrol>\n";
    xml += "    <streamcontrol>" + string(this->user.Rights().StreamControl() ? "true" : "false") + "</streamcontrol>\n";
    xml += "    <sessioncontrol>" + string(this->user.Rights().SessionControl() ? "true" : "false") + "</sessioncontrol>\n";
    xml += "</rights>\n";
    
    char *page = (char *)malloc((xml.length()+1) * sizeof(char));
    strcpy(page, xml.c_str());
    response = MHD_create_response_from_buffer (strlen (page),
                                               (void *) page,
                                               MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header (response, "Content-Type", "text/xml");
    MHD_add_response_header (response, "Cache-Control", "no-cache");
    MHD_add_response_header (response, "Access-Control-Allow-Origin", "*");
    MHD_add_response_header (response, "Access-Control-Allow-Headers", "Authorization");
    ret = MHD_queue_response(this->connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    return ret;
}

int cRequestHandler::handleWebSrv(const char* url) {
    struct MHD_Response *response;
    int ret;
    string file = this->config.GetWebSrvRoot() + string(url).substr(7);
    if(endswith(file.c_str(), "/"))
        file += "index.html";
    int fd;
    struct stat sbuf;
    if ( (-1 == (fd = open (file.c_str(), O_RDONLY))) ||
        (0 != fstat (fd, &sbuf)) ) {
         if (fd != -1)
             close (fd);
         return this->handle404Error();
    }
    if(S_ISDIR(sbuf.st_mode)) {
        if(fd != -1)
            close(fd);
        return this->handle404Error();
    }
    
    string fileExt = "";
    size_t pos = file.find_last_of(".");
    if(pos != string::npos) {
        if(pos + 1 < file.length()) {
            fileExt = file.substr(file.find_last_of(".") + 1);
        }
    }
    
    dsyslog("xmlapi: Request file %s with extension %s", file.c_str(), fileExt.c_str());
    
    response = MHD_create_response_from_fd(sbuf.st_size, fd);
    
    vector<cResponseHeader> headers = this->extHeaders[fileExt];
    for(vector<cResponseHeader>::iterator it = headers.begin(); it != headers.end(); ++it) {
        MHD_add_response_header(response, it->Key().c_str(), it->Value().c_str());
    }
    
    ret = MHD_queue_response(this->connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    return ret;
}

void cRequestHandler::initRemoteKeys() {
    this->remoteKeys.insert(pair<string, eKeys>("up", kUp));
    this->remoteKeys.insert(pair<string, eKeys>("down", kDown));
    this->remoteKeys.insert(pair<string, eKeys>("menu", kMenu));
    this->remoteKeys.insert(pair<string, eKeys>("ok", kOk));
    this->remoteKeys.insert(pair<string, eKeys>("back", kBack));
    this->remoteKeys.insert(pair<string, eKeys>("left", kLeft));
    this->remoteKeys.insert(pair<string, eKeys>("right", kRight));
    this->remoteKeys.insert(pair<string, eKeys>("red", kRed));
    this->remoteKeys.insert(pair<string, eKeys>("green", kGreen));
    this->remoteKeys.insert(pair<string, eKeys>("yellow", kYellow));
    this->remoteKeys.insert(pair<string, eKeys>("blue", kBlue));
    this->remoteKeys.insert(pair<string, eKeys>("0", k0));
    this->remoteKeys.insert(pair<string, eKeys>("1", k1));
    this->remoteKeys.insert(pair<string, eKeys>("2", k2));
    this->remoteKeys.insert(pair<string, eKeys>("3", k3));
    this->remoteKeys.insert(pair<string, eKeys>("4", k4));
    this->remoteKeys.insert(pair<string, eKeys>("5", k5));
    this->remoteKeys.insert(pair<string, eKeys>("6", k6));
    this->remoteKeys.insert(pair<string, eKeys>("7", k7));
    this->remoteKeys.insert(pair<string, eKeys>("8", k8));
    this->remoteKeys.insert(pair<string, eKeys>("9", k9));
    
    this->remoteKeys.insert(pair<string, eKeys>("info", kInfo));
    this->remoteKeys.insert(pair<string, eKeys>("play", kPlay));
    this->remoteKeys.insert(pair<string, eKeys>("pause", kPause));
    this->remoteKeys.insert(pair<string, eKeys>("stop", kStop));
    this->remoteKeys.insert(pair<string, eKeys>("record", kRecord));
    this->remoteKeys.insert(pair<string, eKeys>("fastfwd", kFastFwd));
    this->remoteKeys.insert(pair<string, eKeys>("fastrew", kFastRew));
    this->remoteKeys.insert(pair<string, eKeys>("next", kNext));
    this->remoteKeys.insert(pair<string, eKeys>("prev", kPrev));
    this->remoteKeys.insert(pair<string, eKeys>("power", kPower));
    this->remoteKeys.insert(pair<string, eKeys>("chanup", kChanUp));
    this->remoteKeys.insert(pair<string, eKeys>("chandn", kChanDn));
    this->remoteKeys.insert(pair<string, eKeys>("chanprev", kChanPrev));
    this->remoteKeys.insert(pair<string, eKeys>("volup", kVolUp));
    this->remoteKeys.insert(pair<string, eKeys>("voldn", kVolDn));
    this->remoteKeys.insert(pair<string, eKeys>("mute", kMute));
    this->remoteKeys.insert(pair<string, eKeys>("audio", kAudio));
    this->remoteKeys.insert(pair<string, eKeys>("subtitles", kSubtitles));
    this->remoteKeys.insert(pair<string, eKeys>("schedule", kSchedule));
    this->remoteKeys.insert(pair<string, eKeys>("channels", kChannels));
    this->remoteKeys.insert(pair<string, eKeys>("timers", kTimers));
    this->remoteKeys.insert(pair<string, eKeys>("recordings", kRecordings));
    this->remoteKeys.insert(pair<string, eKeys>("setup", kSetup));
    this->remoteKeys.insert(pair<string, eKeys>("commands", kCommands));
    this->remoteKeys.insert(pair<string, eKeys>("user0", kUser0));
    this->remoteKeys.insert(pair<string, eKeys>("user1", kUser1));
    this->remoteKeys.insert(pair<string, eKeys>("user2", kUser2));
    this->remoteKeys.insert(pair<string, eKeys>("user3", kUser3));
    this->remoteKeys.insert(pair<string, eKeys>("user4", kUser4));
    this->remoteKeys.insert(pair<string, eKeys>("user5", kUser5));
    this->remoteKeys.insert(pair<string, eKeys>("user6", kUser6));
    this->remoteKeys.insert(pair<string, eKeys>("user7", kUser7));
    this->remoteKeys.insert(pair<string, eKeys>("user8", kUser8));
    this->remoteKeys.insert(pair<string, eKeys>("user9", kUser9));
    this->remoteKeys.insert(pair<string, eKeys>("none", kNone)); 
}

int cRequestHandler::handleNotAuthenticated() {
    struct MHD_Response *response;
    int ret;
    const char *page = "<html><body>Wrong credentials.</body></html>";
    response = MHD_create_response_from_buffer (strlen (page), (void *) page,
                                               MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_basic_auth_fail_response (this->connection, "XMLAPI", response);
    MHD_destroy_response (response);
    return ret;
}
