
#ifndef CRESPONSELOGO_H
#define CRESPONSELOGO_H

#include "cResponseHandler.h"

class cResponseLogo : public cResponseHandler {
public:
	cResponseLogo(struct MHD_Connection *connection, cSession *session, cDaemonParameter *daemonParameter);
	int toImage(const char* url);
};

#endif /* CRESPONSELOGO_H */
