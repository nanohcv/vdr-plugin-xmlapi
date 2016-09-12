
#ifndef CRESPONSERIGHTS_H
#define CRESPONSERIGHTS_H

#include "cResponseHandler.h"

class cResponseRights : public cResponseHandler {
public:
    cResponseRights(struct MHD_Connection *connection, cSession *session, cDaemonParameter *daemonParameter);
	int toXml();
};

#endif /* CRESPONSERIGHTS_H */
