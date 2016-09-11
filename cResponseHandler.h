
#ifndef CRESPONSEHANDLER_H
#define CRESPONSEHANDLER_H

#include <string>
#include <map>
#include <algorithm>
#include <microhttpd.h>
#include <arpa/inet.h>
#include <vdr/channels.h>
#include "cDaemonParameter.h"
#include "cSession.h"
#include "cUser.h"
#include "cAuth.h"

using namespace std;

class cResponseHandler {
private:
	const cUser *user;
	void destroyResponse();
	void initConInfo();

protected:
	struct MHD_Response *response;
	struct MHD_Connection *connection;
	cSession *session;
	cDaemonParameter *daemonParameter;
	cPluginConfig config;
    map<string, string> conInfo;

public:
	cResponseHandler(struct MHD_Connection *connection, cSession *session, cDaemonParameter *daemonParameter);
    virtual ~cResponseHandler();
	cResponseHandler *setConnection(struct MHD_Connection *connection);
	cResponseHandler *setSession(cSession *session);
	cResponseHandler *setUser(cUser *user);

	cResponseHandler *create(size_t size, void *buffer, enum MHD_ResponseMemoryMode mode);
	cResponseHandler *create(size_t size, int fd);
	cResponseHandler *header(const char *header, const char *content);
	cResponseHandler *cors();
	cResponseHandler *cors(MHD_Response *response);
	int flush(int status = MHD_HTTP_OK);
	cSession *getSession();
	const cUser *getUser();
	map<string, string> GetConnectionInfo() { return this->conInfo; };
	int handle404Error();
	int handle403Error();
};

#endif  /* CRESPONSEHANDLER_H */
