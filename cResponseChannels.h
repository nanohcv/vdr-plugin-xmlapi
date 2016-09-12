
#ifndef CRESPONSECHANNELS_H
#define CRESPONSECHANNELS_H

#include "cResponseHandler.h"

class cResponseChannels : public cResponseHandler {
private:
	string channelsToXml();
public:
	cResponseChannels(struct MHD_Connection *connection, cSession *session, cDaemonParameter *daemonParameter);
	int toXml();
};

#endif /* CRESPONSECHANNELS_H */
