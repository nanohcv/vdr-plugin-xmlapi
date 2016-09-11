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

#include "cRequestHandler.h"
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
#include "cResponseRemote.h"
#include "cResponseRights.h"
#include "cResponseWebSrv.h"
#include "cSession.h"

cRequestHandler::cRequestHandler(struct MHD_Connection *connection,
                                    cDaemonParameter *daemonParameter)
    : connection(connection), daemonParameter(daemonParameter), auth(NULL) {};

cRequestHandler::~cRequestHandler() {
	delete this->auth;
}

int cRequestHandler::HandleRequest(const char* url) {

    this->auth = new cAuth(this->connection, this->daemonParameter);

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

        if (ret == MHD_HTTP_NOT_FOUND) return this->GetErrorHandler().handle404Error();
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
        cResponseRemote response(this->connection, this->auth->Session(), this->daemonParameter);
        return response.toXml();
    }
    else if (0 == strcmp(url, "/rights.xml")) {
        cResponseRights response(this->connection, this->auth->Session(), this->daemonParameter);
        return response.toXml();
    }
    else if (startswith(url, "/websrv/")) {
        cResponseWebSrv response(this->connection, this->auth->Session(), this->daemonParameter);
        return response.toFile(url);
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
