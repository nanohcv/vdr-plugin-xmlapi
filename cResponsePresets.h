
#ifndef CRESPONSEPRESETS_H
#define CRESPONSEPRESETS_H

#include "cResponseHandler.h"
#include "cPreset.h"
#include "cPresets.h"
#include "cHlsPreset.h"
#include "cHlsPresets.h"

class cResponsePresets : public cResponseHandler {
private:
    cPresets presets;
    cHlsPresets hlsPresets;
public:
	cResponsePresets(struct MHD_Connection *connection, cSession *session, cDaemonParameter *daemonParameter);
	int toIni();
};

#endif /* CRESPONSEPRESETS_H */
