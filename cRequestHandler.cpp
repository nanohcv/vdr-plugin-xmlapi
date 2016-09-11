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
#include "cRequestHandler.h"
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
#include "cResponseStream.h"
#include "cResponseRecordings.h"
#include "cResponseTimer.h"
#include "cResponseSwitch.h"
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

    else if (startswith(url, "/stream") || startswith(url, "/recstream"))
    {

        if(!this->auth->User().Rights().Streaming()) {
            dsyslog("xmlapi: The user %s don't have the permission to access %s", this->auth->User().Name().c_str(), url);
        	return this->GetErrorHandler().handle403Error();
        }
    	if (0 == strcmp(url, "/streamcontrol.xml")) {

    		cResponseStreamControl response(this->connection, this->auth->Session(), this->daemonParameter);
    		return response.toXml();
    	} else {

    		cResponseStream response(this->connection, this->auth->Session(), this->daemonParameter);
    		return response.toStream(url);
    	}
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
        cResponseRecordings response(this->connection, this->auth->Session(), this->daemonParameter);
        return response.toXml();
    }
    else if (0 == strcmp(url, "/deletedrecordings.xml")) {
        cResponseRecordings response(this->connection, this->auth->Session(), this->daemonParameter);
        return response.deletedToXml();
    }
    else if (0 == strcmp(url, "/timers.xml")) {
        cResponseTimer response(this->connection, this->auth->Session(), this->daemonParameter);
        return response.toXml();
    }
    else if (0 == strcmp(url, "/switch.xml")) {
        cResponseSwitch response(this->connection, this->auth->Session(), this->daemonParameter);
        return response.toXml();
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
