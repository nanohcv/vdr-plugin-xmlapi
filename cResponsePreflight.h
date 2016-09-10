
#ifndef CRESPONSEPREFLIGHT_H
#define CRESPONSEPREFLIGHT_H

#include "cResponseHandler.h"

class cResponsePreflight : public cResponseHandler {
public:
	cResponsePreflight(struct MHD_Connection *connection, cSession *session, cDaemonParameter *daemonParameter);
	int toText();
};

#endif /* CRESPONSEPREFLIGHT_H */
