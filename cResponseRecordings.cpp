#include "cResponseRecordings.h"
#include "globals.h"

cResponseRecordings::cResponseRecordings(struct MHD_Connection *connection, cSession *session, cDaemonParameter *daemonParameter)
	: cResponseHandler(connection, session, daemonParameter) {

	this->xml = "";
};

cResponseRecordings::~cResponseRecordings() {

	this->rec = NULL;
}

int cResponseRecordings::toXml(bool deleted) {

    this->recfile = MHD_lookup_connection_value(this->connection, MHD_GET_ARGUMENT_KIND, "filename");
    this->action = MHD_lookup_connection_value(this->connection, MHD_GET_ARGUMENT_KIND, "action");

    if(recfile != NULL && action != NULL) {

        if(!this->user->Rights().Recordings()) {
            dsyslog("xmlapi: The user %s doesn't have the permission to do any action on /recordings.xml", this->user->Name().c_str());
            return this->handle403Error();
        }
#if VDRVERSNUM >= 20301
        LOCK_RECORDINGS_WRITE;
        LOCK_DELETEDRECORDINGS_WRITE;
        cRecordings *recs = deleted ? DeletedRecordings : Recordings;
#else
    	cRecordings *recs = deleted ? &DeletedRecordings : &Recordings;
#endif
        this->rec = recs->GetByName(this->recfile);
        if(this->rec == NULL) {
            return this->handle404Error();
        }
    }

	if (deleted && this->action != NULL) {

		this->deletedRecordingsAction();

	} else if (this->action != NULL){

		this->recordingsAction();

	} else {
		this->recordingsToXml(deleted);
	}

    char *page = (char *)malloc((this->xml.length() + 1) *sizeof(char));
    strcpy(page, this->xml.c_str());
    return this->create(strlen (page), (void *) page, MHD_RESPMEM_MUST_FREE)
    		->header("Content-Type", "text/xml")
			->cors()
			->flush();
};

int cResponseRecordings::deletedToXml() {

	return this->toXml(true);
}

void cResponseRecordings::recordingsAction() {

	this->xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
	this->xml += "<actions>\n";
	if(0 == strcmp(this->action, "delete")) {
#if VDRVERSNUM >= 20301
        LOCK_RECORDINGS_WRITE;
        LOCK_DELETEDRECORDINGS_WRITE;
#endif
		if(this->rec->Delete())
		{
#if VDRVERSNUM >= 20301
            Recordings->Update();
            DeletedRecordings->Update();
#else
        	Recordings.Update();
        	DeletedRecordings.Update();
#endif
			this->xml += "    <delete>true</delete>\n";
		}
		else {
			this->xml += "    <delete>false</delete>\n";
		}
	}
	else {
		this->xml += "    <unknown>" + string(this->action) + "</unknown>\n";
	}
	this->xml += "</actions>\n";
};

void cResponseRecordings::deletedRecordingsAction() {

	this->xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
	this->xml += "<actions>\n";
    if(0 == strcmp(this->action, "undelete")) {
#if VDRVERSNUM >= 20301
        LOCK_RECORDINGS_WRITE;
        LOCK_DELETEDRECORDINGS_WRITE;
#endif
        if(this->rec->Undelete())
        {
#if VDRVERSNUM >= 20301
            Recordings->Update();
            DeletedRecordings->Update();
#else
        	Recordings.Update();
        	DeletedRecordings.Update();
#endif
        	this->xml += "    <undelete>true</undelete>\n";
        }
        else {
        	this->xml += "    <undelete>false</undelete>\n";
        }
    }
    else if(0 == strcmp(this->action, "remove")) {
#if VDRVERSNUM >= 20301
        LOCK_RECORDINGS_WRITE;
        LOCK_DELETEDRECORDINGS_WRITE;
#endif
        if(this->rec->Remove())
        {
#if VDRVERSNUM >= 20301
            DeletedRecordings->Update();
#else
        	DeletedRecordings.Update();
#endif
        	this->xml += "    <remove>true</remove>\n";
        }
        else {
        	this->xml += "    <remove>false</remove>\n";
        }
    }
    else {
    	this->xml += "    <unknown>" + string(this->action) + "</unknown>\n";
    }
    this->xml += "</actions>\n";
};

void cResponseRecordings::recordingsToXml(bool deleted) {

	this->xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
	this->xml += "<recordings>\n";
#if VDRVERSNUM >= 20301
    LOCK_RECORDINGS_READ;
    LOCK_DELETEDRECORDINGS_READ;
    const cRecordings *recs = deleted ? DeletedRecordings : Recordings;
#else
    cRecordings *recs = deleted ? &DeletedRecordings : &Recordings;
#endif
	for(int i=0; i<recs->Count(); i++)
	{
#if VDRVERSNUM >= 20301
		const cRecording *rec = recs->Get(i);
#else
        cRecording *rec = recs->Get(i);
#endif
		const cRecordingInfo *info = rec->Info();
		string name = string(rec->Name() ? rec->Name() : "");
		string filename = string(rec->FileName() ? rec->FileName() : "");
		string title = string(rec->Title() ? rec->Title() : "");
		string inuse = rec->IsInUse() > 0 ? "true" : "false";
		string duration = intToString(rec->LengthInSeconds());
		string filesize = intToString(rec->FileSizeMB());
		string deleted = timeToString(rec->Deleted());
		string ichannelid = string(info->ChannelID().ToString());
		string ichannelname = string(info->ChannelName() ? info->ChannelName() : "");
		string ititle = string(info->Title() ? info->Title() : "");
		string ishorttext = string(info->ShortText() ? info->ShortText() : "");
		string idescription = string(info->Description() ? info->Description() : "");
		xmlEncode(name);
		xmlEncode(filename);
		xmlEncode(title);
		xmlEncode(ichannelid);
		xmlEncode(ichannelname);
		xmlEncode(ititle);
		xmlEncode(ishorttext);
		xmlEncode(idescription);
		this->xml += "    <recording>\n";
		this->xml += "        <name>" + name + "</name>\n";
		this->xml += "        <filename>" + filename + "</filename>\n";
		this->xml += "        <title>" + title + "</title>\n";
		this->xml += "        <inuse>" + inuse + "</inuse>\n";
		this->xml += "        <size>" + filesize + "</size>\n";
		this->xml += "        <duration>" + duration + "</duration>\n";
		this->xml += "        <deleted>" + deleted + "</deleted>\n";
		this->xml += "        <infos>\n";
		this->xml += "            <channelid>" + ichannelid + "</channelid>\n";
		this->xml += "            <channelname>" + ichannelname + "</channelname>\n";
		this->xml += "            <title>" + ititle + "</title>\n";
		this->xml += "            <shorttext>" + ishorttext + "</shorttext>\n";
		this->xml += "            <description>" + idescription + "</description>\n";
		this->xml += "        </infos>\n";
		this->xml += "    </recording>\n";
	}
    this->xml += "</recordings>\n";
}
