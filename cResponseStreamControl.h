
#ifndef CRESPONSESTREAMCONTROL_H
#define CRESPONSESTREAMCONTROL_H

#include "cResponseHandler.h"

class cResponseStreamControl : public cResponseHandler {
public:
	cResponseStreamControl(struct MHD_Connection *connection, cSession *session, cDaemonParameter *daemonParameter);
	int toXml();
};

#endif /* CRESPONSESTREAMCONTROL_H */
