
#ifndef CRESPONSESWITCH_H
#define CRESPONSESWITCH_H

#include "cResponseHandler.h"
#include <vdr/device.h>
#include <vdr/channels.h>

class cResponseSwitch : public cResponseHandler {
public:
	cResponseSwitch(struct MHD_Connection *connection, cSession *session, cDaemonParameter *daemonParameter);
	int toXml();
};

#endif /* CRESPONSESWITCH_H */
