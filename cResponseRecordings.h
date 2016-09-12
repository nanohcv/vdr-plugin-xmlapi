
#ifndef CRESPONSERECORDINGS_H
#define CRESPONSERECORDINGS_H

#include "cResponseHandler.h"
#include <vdr/recording.h>

class cResponseRecordings : public cResponseHandler {
private:
	string xml;
	cRecording *rec = NULL;
	const char *action = NULL;
	const char *recfile = NULL;
	void recordingsAction();
	void deletedRecordingsAction();
	void recordingsToXml(bool deleted = false);
public:
	cResponseRecordings(struct MHD_Connection *connection, cSession *session, cDaemonParameter *daemonParameter);
	virtual ~cResponseRecordings();
	int toXml(bool deleted = false);
	int deletedToXml();
};

#endif /* CRESPONSERECORDINGS_H */
