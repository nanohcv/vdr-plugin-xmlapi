
#ifndef CRESPONSEWEBSERVER_H
#define CRESPONSEWEBSERVER_H

#include "cResponseHandler.h"
#include "cExtensionHeaders.h"

class cResponseWebSrv : public cResponseHandler {
private:
    cExtensionHeaders extHeaders;
public:
	cResponseWebSrv(struct MHD_Connection *connection, cSession *session, cDaemonParameter *daemonParameter);
	int toFile(const char* url);
};

#endif /* CRESPONSEWEBSERVER_H */
