
#ifndef CRESPONSEHLSSTREAM_H
#define CRESPONSEHLSSTREAM_H

#include "cResponseHandler.h"
#include "cHlsPreset.h"
#include "cHlsPresets.h"
#include <vdr/recording.h>
#include "globals.h"

class cResponseHlsStream : public cResponseHandler {
private:
	cHlsPresets presets;
	string file;
	int initM3u8();
	const char* url = NULL;
	const char* cstr_preset = NULL;
	const char* chid = NULL;
	const char* recfile = NULL;
    string streamName;
    string input;
    int starttime;

public:
	cResponseHlsStream(struct MHD_Connection *connection, cSession *session, cDaemonParameter *daemonParameter);
	int respond(const char *url);
	int toM3u8();
	int toStream(const char *url);
	int toTs();
};

#endif /* CRESPONSEHLSSTREAM_H */
