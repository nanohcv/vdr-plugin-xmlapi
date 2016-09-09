
#ifndef CRESPONSEVERSION_H
#define CRESPONSEVERSION_H

#include "cResponseHandler.h"
#include "cPluginConfig.h"
#include "cSession.h"

class cResponseVersion : public cResponseHandler {
public:
	cResponseVersion(struct MHD_Connection *connection, cSession *session);
	int toXml(cPluginConfig config);
};

#endif /* CRESPONSEVERSION_H */
