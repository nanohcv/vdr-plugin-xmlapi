#include "cResponseHandler.h"

cResponseHandler::cResponseHandler(struct MHD_Connection *connection, cSession *session, cDaemonParameter *daemonParameter)
	: connection(connection), session(session), daemonParameter(daemonParameter), config(daemonParameter->GetPluginConfig())
{
	this->user = NULL;
	this->response = NULL;

	this->initConInfo();
};

cResponseHandler::~cResponseHandler() {

	this->session = NULL;
	this->user = NULL;
	this->destroyResponse();
	this->connection = NULL;
	this->daemonParameter = NULL;
};

cResponseHandler *cResponseHandler::setSession(cSession *session) {
	this->session = session;
	return this;
}

cResponseHandler *cResponseHandler::setUser(cUser *user) {
	this->user = user;
	return this;
}

cSession *cResponseHandler::getSession() {
	return this->session;
}

cUser *cResponseHandler::getUser() {
	return this->user;
}

cResponseHandler *cResponseHandler::create(size_t size, void *buffer, enum MHD_ResponseMemoryMode mode) {

	this->response = MHD_create_response_from_buffer(size, buffer, mode);
	this->header(MHD_HTTP_HEADER_SET_COOKIE, this->session->Cookie().c_str());
	return this;
}

cResponseHandler *cResponseHandler::header(const char *header, const char *content) {

	MHD_add_response_header (this->response, header, content);
	return this;
};

cResponseHandler *cResponseHandler::cors() {

	cPluginConfig config = this->daemonParameter->GetPluginConfig();

	this->header("Access-Control-Allow-Origin", config.GetCorsOrigin().c_str())
		->header("Access-Control-Allow-Headers", "Authorization")
		->header("Access-Control-Allow-Credentials", "true");

	return this;
};

int cResponseHandler::flush() {

    int ret = MHD_queue_response(this->connection, MHD_HTTP_OK, this->response);
    this->destroyResponse();
    return ret;
};

void cResponseHandler::destroyResponse() {
	if (this->response) {
		MHD_destroy_response (this->response);
		this->response = NULL;
	}
};

void cResponseHandler::initConInfo() {

    const MHD_ConnectionInfo *connectionInfo = MHD_get_connection_info (this->connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS);
    if (connectionInfo->client_addr->sa_family == AF_INET)
    {
        struct sockaddr_in *sin = (struct sockaddr_in *) connectionInfo->client_addr;
        char *ip = inet_ntoa(sin->sin_addr);
        if(ip != NULL)
        {
            this->conInfo.insert(pair<string,string>("ClientIP", string(ip)));
        }
    }
    const char *useragent = MHD_lookup_connection_value(this->connection, MHD_HEADER_KIND, "User-Agent");
    if(useragent != NULL)
    {
        this->conInfo.insert(pair<string,string>("User-Agent",string(useragent)));
    }
    const char *host = MHD_lookup_connection_value(this->connection, MHD_HEADER_KIND, "Host");
    if(host != NULL)
    {
        this->conInfo.insert(pair<string,string>("Host", string(host)));
    }
};