
#ifndef CRESPONSEVERSION_H
#define CRESPONSEVERSION_H

#include "cResponseHandler.h"

class cResponseVersion : public cResponseHandler {
public:
	cResponseVersion(struct MHD_Connection *connection, cSession *session, cDaemonParameter *daemonParameter);
	int toXml();
};

#endif /* CRESPONSEVERSION_H */
