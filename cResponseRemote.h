
#ifndef CRESPONSEREMOTE_H
#define CRESPONSEREMOTE_H

#include "cResponseHandler.h"
#include <vdr/remote.h>

class cResponseRemote : public cResponseHandler {
private:
    map<string, eKeys> remoteKeys;
    void initRemoteKeys();
public:
	cResponseRemote(struct MHD_Connection *connection, cSession *session, cDaemonParameter *daemonParameter);
	int toXml();
};

#endif /* CRESPONSEREMOTE_H */
