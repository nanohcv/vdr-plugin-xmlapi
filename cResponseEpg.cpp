#include "cResponseEpg.h"

cResponseEpg::cResponseEpg(struct MHD_Connection *connection, cSession *session, cDaemonParameter *daemonParameter)
	: cResponseHandler(connection, session, daemonParameter) {};

int cResponseEpg::toXml() {

    string xml;
    const char* chid = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "chid");
    const char* at = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "at");
    const char* cstr_search = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "search");
    const char* cstr_options = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "options");

    if(chid != NULL && cstr_search == NULL)
    {
        xml = this->eventsToXml(chid, at);
    }
    else {
        string search = "";
        string options = "";
        if(cstr_search != NULL)
            search = string(cstr_search);
        if(cstr_options != NULL)
            options = string(cstr_options);
        xml = this->searchEventsToXml(chid, search, options);
    }
    char *page = (char *)malloc((xml.length()+1) * sizeof(char));
    strcpy(page, xml.c_str());
    return this->create(strlen (page), (void *) page, MHD_RESPMEM_MUST_FREE)
		->header("Content-Type", "text/xml")
		->cors()
		->flush();
};

string cResponseEpg::eventsToXml(const char* chid, const char *at) {

    string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    xml +=       "<events>\n";
    if(chid != NULL)
    {
        tChannelID cid = tChannelID::FromString(chid);
        if(cid.Valid()) {
            bool now = false;
            bool next = false;
            bool attime = false;
            time_t at_time = 0;
            if(at != NULL) {
                if (0 == strcmp(at, "now")) {
                    now = true;
                } else if (0 == strcmp(at, "next")) {
                    next = true;
                } else {
                    at_time = atol(at);
                    attime = true;
                }
            }
#if VDRVERSNUM >= 20301
            LOCK_SCHEDULES_READ;
            const cSchedules *schedules = Schedules;
#else
            cSchedulesLock lock;
            const cSchedules *schedules = cSchedules::Schedules(lock);
#endif
            const cSchedule *schedule = schedules->GetSchedule(cid);
            if(schedule != NULL) {
                const cList<cEvent> *events = schedule->Events();
                for(int i=0; i<events->Count(); i++) {
#if VDRVERSNUM >= 20301
                    const cEvent *event = NULL;
#else
                    cEvent *event = NULL;
#endif
                    if(now) {
                        event = const_cast<cEvent *>(schedule->GetPresentEvent());
                        i = events->Count();
                    } else if (next) {
                        event = const_cast<cEvent *>(schedule->GetFollowingEvent());
                        i = events->Count();
                    } else if (attime) {
                        event = const_cast<cEvent *>(schedule->GetEventAround(at_time));
                        i = events->Count();
                    } else {
                        event = events->Get(i);
                    }
                    if(event == NULL) {
                        continue;
                    }
                    const char *t = event->Title();
                    const char *s = event->ShortText();
                    const char *d = event->Description();

                    string title;
                    if(t != NULL)
                        title = string(event->Title());
                    string shorttext;
                    if(s != NULL)
                        shorttext = string(event->ShortText());
                    string descr;
                    if(d != NULL)
                        descr = string(event->Description());
                    xmlEncode(title);
                    xmlEncode(shorttext);
                    xmlEncode(descr);
                    xml += "  <event id=\"" + uint32ToString(event->EventID()) + "\">\n";
                    xml += "    <channelid>" + string(event->ChannelID().ToString()) + "</channelid>\n";
                    xml += "    <title>" + title + "</title>\n";
                    xml += "    <shorttext>" + shorttext + "</shorttext>\n";
                    xml += "    <description>" + descr + "</description>\n";
                    xml += "    <start>" + timeToString(event->StartTime()) + "</start>\n";
                    xml += "    <stop>" + timeToString(event->EndTime()) + "</stop>\n";
                    xml += "    <duration>" + intToString(event->Duration()) + "</duration>\n";
                    xml += "  </event>\n";
                }
            }
        }
        else {
            dsyslog("xmlapi: epg.xml -> invalid channel id");
        }
    }
    xml += "</events>\n";
    return xml;
}

string cResponseEpg::searchEventsToXml(const char* chid, string search, string options) {
    string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    xml +=       "<events>\n";


    if(chid != NULL)
    {
        tChannelID cid = tChannelID::FromString(chid);
        if(!cid.Valid())
            chid = NULL;
    }
#if VDRVERSNUM >= 20301
    LOCK_SCHEDULES_READ;
    const cSchedules *schedules = Schedules;
#else
    cSchedulesLock lock;
    const cSchedules *schedules = cSchedules::Schedules(lock);
#endif
    for(int i=0; i<schedules->Count(); i++) {
        const cSchedule *schedule;
        if(chid != NULL)
        {
            tChannelID cid = tChannelID::FromString(chid);
            if(!cid.Valid())
                break;
            schedule = schedules->GetSchedule(cid);
            i = schedules->Count();
        }
        else {
            schedule = schedules->Get(i);
        }
        if(schedule != NULL) {
            const cList<cEvent> *events = schedule->Events();
            for(int j=0; j<events->Count(); j++) {
#if VDRVERSNUM >= 20301                
                const cEvent *event = events->Get(j);
#else
                cEvent *event = events->Get(j);
#endif
                if(event == NULL) {
                    continue;
                }
                const char *t = event->Title();
                const char *s = event->ShortText();
                const char *d = event->Description();

                string title;
                if(t != NULL)
                    title = string(event->Title());
                string shorttext;
                if(s != NULL)
                    shorttext = string(event->ShortText());
                string descr;
                if(d != NULL)
                    descr = string(event->Description());

                if(search != "") {
                    if(options == "" || options == "T")
                    {
                        if(!searchInString(title, search))
                            continue;
                    }
                    else if(options == "S") {
                        if(!searchInString(shorttext, search))
                            continue;
                    }
                    else if(options == "D") {
                        if(!searchInString(descr, search))
                            continue;
                    }
                    else if(options == "TS" || options == "ST") {
                        if( (!searchInString(title, search)) && (!searchInString(shorttext, search)) )
                            continue;
                    }
                    else if(options == "TD" || options == "DT") {
                        if( (!searchInString(title, search)) && (!searchInString(descr, search)) )
                            continue;
                    }
                    else if(options == "SD" || options == "DS") {
                        if( (!searchInString(shorttext, search)) && (!searchInString(descr, search)) )
                            continue;
                    }
                    else if(options == "TSD" || options == "TDS" || options == "SDT" || options == "STD" || options == "DTS" || options == "DST") {
                        if( (!searchInString(title, search)) && (!searchInString(shorttext, search)) && (!searchInString(descr, search)) )
                            continue;
                    }
                }
                xmlEncode(title);
                xmlEncode(shorttext);
                xmlEncode(descr);
                xml += "  <event id=\"" + uint32ToString(event->EventID()) + "\">\n";
                xml += "    <channelid>" + string(event->ChannelID().ToString()) + "</channelid>\n";
                xml += "    <title>" + title + "</title>\n";
                xml += "    <shorttext>" + shorttext + "</shorttext>\n";
                xml += "    <description>" + descr + "</description>\n";
                xml += "    <start>" + timeToString(event->StartTime()) + "</start>\n";
                xml += "    <stop>" + timeToString(event->EndTime()) + "</stop>\n";
                xml += "    <duration>" + intToString(event->Duration()) + "</duration>\n";
                xml += "  </event>\n";
            }
        }
    }


    xml += "</events>\n";
    return xml;
}
