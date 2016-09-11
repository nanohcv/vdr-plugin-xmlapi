#include "cResponseTimer.h"
#include "globals.h"

cResponseTimer::cResponseTimer(struct MHD_Connection *connection, cSession *session, cDaemonParameter *daemonParameter)
	: cResponseHandler(connection, session, daemonParameter) {

	this->xml = "";
	this->action = MHD_lookup_connection_value(this->connection, MHD_GET_ARGUMENT_KIND, "action");
};

int cResponseTimer::toXml() {

    if(this->action != NULL) {

    	const cUser *user = SessionControl->GetUserBySessionId(this->session->GetSessionId());

        this->xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
        if(!user->Rights().Timers()) {
            dsyslog("xmlapi: The user %s don't have the permission to do any action on /timers.xml", user->Name().c_str());
            return this->handle403Error();
        }
        if(0 == strcmp(action, "delete")) {
        	this->del();
        }
        else if(0 == strcmp(action, "onoff")) {
        	this->toggle();
        }
        else if(0 == strcmp(action, "add")) {

        	this->add();
        }
        else {
            this->xml += "<unknownaction>" + string(action) + "</unknownaction>\n";
        }
    } else {
    	this->timersToXml();
    }

    char *page = (char *)malloc((this->xml.length() + 1) *sizeof(char));
    strcpy(page, this->xml.c_str());

    return this->create(strlen (page), (void *) page,  MHD_RESPMEM_MUST_FREE)
		->header("Content-Type", "text/xml")
		->cors()
		->flush();
};

void cResponseTimer::timersToXml() {

    this->xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    this->xml += "<timers>\n";

    for(int i=0; i<Timers.Count(); i++) {
        cTimer *timer = Timers.Get(i);
        ostringstream builder;
        builder << string(timer->Channel()->GetChannelID().ToString()) << ":" << timer->WeekDays() << ":"
				<< timer->Day() << ":" << timer->Start() << ":" << timer->Stop();
        string id = builder.str();
        string chid = string(timer->Channel()->GetChannelID().ToString());
        string chname = string(timer->Channel()->Name() ? timer->Channel()->Name() : "");
        string name = string(timer->File() ? timer->File() : "");
        string aux = string(timer->Aux() ? timer->Aux() : "");
        string flags = uint32ToString(timer->Flags());
        string weekdays = intToString(timer->WeekDays());
        string day = timeToString(timer->Day());
        string start = intToString(timer->Start());
        string stop = intToString(timer->Stop());
        string priority = intToString(timer->Priority());
        string lifetime = intToString(timer->Lifetime());
        xmlEncode(id);
        xmlEncode(chid);
        xmlEncode(chname);
        xmlEncode(name);
        xmlEncode(aux);

        this->xml += "    <timer id=\"" + id + "\">\n";
        this->xml += "        <channelid>" + chid + "</channelid>\n";
        this->xml += "        <channelname>" + chname + "</channelname>\n";
        this->xml += "        <name>" + name + "</name>\n";
        this->xml += "        <aux>" + aux + "</aux>\n";
        this->xml += "        <flags>" + flags + "</flags>\n";
        this->xml += "        <weekdays>" + weekdays + "</weekdays>\n";
        this->xml += "        <day>" + day + "</day>\n";
        this->xml += "        <start>" + start + "</start>\n";
        this->xml += "        <stop>" + stop + "</stop>\n";
        this->xml += "        <priority>" + priority + "</priority>\n";
        this->xml += "        <lifetime>" + lifetime + "</lifetime>\n";
        this->xml += "    </timer>\n";
    }

    this->xml += "</timers>\n";
};


cTimer *cResponseTimer::GetTimer(const char* tid) {
    if(tid == NULL)
        return NULL;
    for(int i=0; i<Timers.Count();i++)
    {
        cTimer *timer = Timers.Get(i);
        string id = string(tid);
        vector<string> parts = split(id, ':');
        if(parts.size() != 5) {
            return NULL;
        }
        tChannelID cid = tChannelID::FromString(parts[0].c_str());
        if(!cid.Valid())
            return NULL;
        int wdays = atoi(parts[1].c_str());
        time_t day = atol(parts[2].c_str());
        int start = atoi(parts[3].c_str());
        int stop = atoi(parts[4].c_str());
        if(timer->Channel()->GetChannelID() == cid &&
           timer->WeekDays() == wdays &&
           timer->Day() == day &&
           timer->Start() == start &&
           timer->Stop() == stop) {
            return timer;
        }
    }
    return NULL;
}

const cEvent * cResponseTimer::GetEvent(tChannelID channelid, tEventID eid) {
    cSchedulesLock lock;
    const cSchedules *schedules = cSchedules::Schedules(lock);
    const cSchedule *schedule = schedules->GetSchedule(channelid);
    if(schedule == NULL)
        return NULL;
    const cEvent *event = schedule->GetEvent(eid);
    return event;
}

void cResponseTimer::del() {

    const char *tid = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "id");
    bool deleted = this->deleteTimer(tid);
    if(deleted)
    	this->xml += "<deleted>true</deleted>\n";
    else
    	this->xml += "<deleted>false</deleted>\n";
};

bool cResponseTimer::deleteTimer(const char* tid) {
    cTimer *timer = this->GetTimer(tid);
    if(timer == NULL)
        return false;
    Timers.Del(timer);
    Timers.SetModified();
    return true;
}

void cResponseTimer::toggle() {
    const char *tid = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "id");
    bool onOff = this->onOffTimer(tid);
    if(onOff) {
    	this->xml += "<onoff>successful</onoff>\n";
    }
    else {
    	this->xml += "<onoff>failed</onoff>\n";
    }
};

bool cResponseTimer::onOffTimer(const char* tid) {
    cTimer *timer = this->GetTimer(tid);
    if(timer == NULL)
        return false;
    timer->OnOff();
    return true;
}

void cResponseTimer::add() {

    const char *eventid = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "eventid");
    const char *channelid = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "chid");
    const char *name = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "name");
    const char *aux = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "aux");
    const char *cstr_flags = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "flags");
    const char *cstr_weekdays = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "weekdays");
    const char *cstr_day = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "day");
    const char *cstr_start = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "start");
    const char *cstr_stop = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "stop");
    const char *cstr_priority = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "priority");
    const char *cstr_lifetime = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "lifetime");
    if(eventid != NULL && channelid != NULL &&
            name == NULL && aux == NULL && cstr_flags == NULL &&
            cstr_weekdays == NULL && cstr_day == NULL && cstr_start == NULL &&
            cstr_stop == NULL && cstr_priority == NULL && cstr_lifetime == NULL) {
        bool added = this->addTimer(channelid, eventid);
        if(added)
        	this->xml += "<added>true</added>\n";
        else
        	this->xml += "<added>false</added>\n";
    }
    else if(channelid != NULL && name != NULL && aux != NULL &&
            cstr_flags != NULL && cstr_weekdays != NULL &&
            cstr_day != NULL && cstr_start != NULL && cstr_stop != NULL &&
            cstr_priority != NULL && cstr_lifetime != NULL && eventid == NULL) {
        bool added = this->addTimer(channelid, name, aux, cstr_flags,
                cstr_weekdays, cstr_day, cstr_start, cstr_stop,
                cstr_priority, cstr_lifetime);
        if(added)
        	this->xml += "<added>true</added>\n";
        else
        	this->xml += "<added>false</added>\n";
    }
    else {
    	this->xml += "<error>incorrect parameters</error>\n";
    }
};

bool cResponseTimer::addTimer(const char *channelid, const char* eventid) {
    if(eventid == NULL || channelid == NULL)
        return false;
    tEventID eid = (tEventID)strtoul(eventid, NULL, 10);
    tChannelID cid = tChannelID::FromString(channelid);
    if(!cid.Valid())
        return false;
    const cEvent *event = this->GetEvent(cid, eid);
    if(event == NULL)
        return false;
    if(Timers.GetMatch(event) != NULL)
        return false;
    cTimer *timer = new cTimer(event);
    Timers.Add(timer);
    Timers.SetModified();
    return true;
}

bool cResponseTimer::addTimer(const char* channelid, const char* name,
        const char* aux, const char* cstr_flags, const char* cstr_weekdays,
        const char* cstr_day, const char* cstr_start, const char* cstr_stop,
        const char* cstr_priority, const char* cstr_lifetime) {
    tChannelID cid = tChannelID::FromString(channelid);
    if(!cid.Valid())
        return false;
    cChannel *channel = Channels.GetByChannelID(cid);
    if(channel == NULL)
        return false;
    unsigned int flags = (unsigned int)atoi(cstr_flags);
    int weekdays = atoi(cstr_weekdays);
    time_t day = (time_t)atol(cstr_day);
    int start = atoi(cstr_start);
    int stop = atoi(cstr_stop);
    int priority = atoi(cstr_priority);
    int lifetime = atoi(cstr_lifetime);
    cTimer *timer = new cTimer(false, false, channel);
    timer->SetFlags(flags);
    timer->SetFile(name);
    timer->SetAux(aux);
    timer->SetWeekDays(weekdays);
    timer->SetDay(day);
    timer->SetStart(start);
    timer->SetStop(stop);
    timer->SetPriority(priority);
    timer->SetLifetime(lifetime);
    if(Timers.GetTimer(timer) != NULL)
        return false;
    Timers.Add(timer);
    Timers.SetModified();
    return true;
}
