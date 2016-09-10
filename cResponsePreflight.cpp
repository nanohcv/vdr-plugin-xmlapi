#include "cResponsePreflight.h"

cResponsePreflight::cResponsePreflight(struct MHD_Connection *connection, cSession *session, cDaemonParameter *daemonParameter)
	: cResponseHandler(connection, session, daemonParameter) {};

int cResponsePreflight::toText() {

    struct MHD_Response *response;
    int ret;
	const char *page = "OK";

	response = MHD_create_response_from_buffer (strlen (page), (void *) page, MHD_RESPMEM_PERSISTENT);
	MHD_add_response_header (response, "Allow", "GET");
	this->cors(response);
	ret = MHD_queue_response (this->connection, MHD_HTTP_OK, response);
	MHD_destroy_response (response);
	return ret;
};
