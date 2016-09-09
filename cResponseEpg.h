
#ifndef CRESPONSEEPG_H
#define CRESPONSEEPG_H

#include "cResponseHandler.h"
#include <vdr/epg.h>

class cResponseEpg : public cResponseHandler {
private:
	string eventsToXml(const char* chid, const char *at);
	string searchEventsToXml(const char* chid, string search, string options);
public:
	cResponseEpg(struct MHD_Connection *connection, cSession *session, cDaemonParameter *daemonParameter);
	int toXml();
};

#endif /* CRESPONSEEPG_H */
