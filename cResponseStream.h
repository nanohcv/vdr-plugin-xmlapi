
#ifndef CRESPONSESTREAM_H
#define CRESPONSESTREAM_H

#include "cResponseHandler.h"
#include "cPreset.h"
#include "cPresets.h"
#include "cStream.h"
#include <vdr/recording.h>
#include "globals.h"

class cResponseStream : public cResponseHandler {
private:
	cPresets presets;
	int initStream();
	int initInput();
	int stream();
	const char* url = NULL;
	const char* cstr_preset = NULL;
	const char* chid = NULL;
	const char* recfile = NULL;
	const char* mimeType = NULL;
	string input;
	int *streamid;

    static ssize_t stream_reader (void *cls, uint64_t pos, char *buf, size_t max);
    static void clear_stream(void *cls);

    cResponseStream *create(uint64_t size,
			   size_t block_size,
			   MHD_ContentReaderCallback crc, void *crc_cls,
			   MHD_ContentReaderFreeCallback crfc);
public:
	cResponseStream(struct MHD_Connection *connection, cSession *session, cDaemonParameter *daemonParameter);
	virtual ~cResponseStream();
	int toStream(const char *url);
};

#endif /* CRESPONSESTREAM_H */
