
#ifndef CRESPONSETIMER_H
#define CRESPONSETIMER_H

#include "cResponseHandler.h"
#include <vdr/timers.h>
#include <sstream>

class cResponseTimer : public cResponseHandler {
private:
	string xml;
	const char *action = NULL;

	void timersToXml();
	cTimer *GetTimer(const char* tid);
	const cEvent *GetEvent(tChannelID channelid, tEventID eid);
	bool deleteTimer(const char* tid);
	bool onOffTimer(const char* tid);
	void add();
	void del();
	void toggle();
	bool addTimer(const char *channelid, const char* eventid);
	bool addTimer(const char* channelid, const char* name,
	        const char* aux, const char* cstr_flags, const char* cstr_weekdays,
	        const char* cstr_day, const char* cstr_start, const char* cstr_stop,
	        const char* cstr_priority, const char* cstr_lifetime);
public:
	cResponseTimer(struct MHD_Connection *connection, cSession *session, cDaemonParameter *daemonParameter);
	int toXml();
};

#endif /* CRESPONSETIMER_H */
