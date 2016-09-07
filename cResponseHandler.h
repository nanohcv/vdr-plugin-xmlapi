
#ifndef CRESPONSEHANDLER_H
#define CRESPONSEHANDLER_H

#include <string>
#include <microhttpd.h>
#include "cSession.h"
#include "cUser.h"

using namespace std;

class cResponseHandler {
private:
	struct MHD_Response *response;
	cSession *session;
	cUser *user;
	void destroyResponse();
protected:
	struct MHD_Connection *connection;
public:
	cResponseHandler();
    virtual ~cResponseHandler();
	cResponseHandler *setConnection(struct MHD_Connection *connection);
	cResponseHandler *setSession(cSession *session);
	cResponseHandler *setUser(cUser *user);

	cResponseHandler *create(size_t size, void *buffer, enum MHD_ResponseMemoryMode mode);
	cResponseHandler *header(const char *header, const char *content);
	cResponseHandler *cors();
	int flush();
	cSession *getSession();
	cUser *getUser();
};

#endif  /* CRESPONSEHANDLER_H */
