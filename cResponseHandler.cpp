#include "cResponseHandler.h"

cResponseHandler::cResponseHandler() {

	this->connection = NULL;
	this->session = NULL;
	this->user = NULL;
	this->response = NULL;
};

cResponseHandler::~cResponseHandler() {

	this->session = NULL;
	this->user = NULL;
	this->destroyResponse();
	delete this->session;
	delete this->user;
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
	return this;
}

cResponseHandler *cResponseHandler::header(const char *header, const char *content) {

	MHD_add_response_header (this->response, header, content);
	return this;
};

cResponseHandler *cResponseHandler::cors() {

	this->header("Access-Control-Allow-Origin", "*")
		->header("Access-Control-Allow-Headers", "Authorization");

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
