#include "cResponseHandler.h"
#include "cPluginConfig.h"

class cResponseVersion : public cResponseHandler {
public:
	cResponseVersion(struct MHD_Connection *connection);
	int toXml(cPluginConfig config);
};
