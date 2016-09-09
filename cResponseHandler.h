
#ifndef CRESPONSEHANDLER_H
#define CRESPONSEHANDLER_H

#include <string>
#include <microhttpd.h>
#include "cDaemonParameter.h"
#include "cSession.h"
#include "cUser.h"
#include "cAuth.h"

using namespace std;

class cResponseHandler {
private:
	struct MHD_Response *response;
	cUser *user;
	void destroyResponse();
protected:
	struct MHD_Connection *connection;
	cSession *session;
	cDaemonParameter *daemonParameter;
public:
	cResponseHandler(struct MHD_Connection *connection, cSession *session, cDaemonParameter *daemonParameter);
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
