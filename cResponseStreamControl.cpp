#include "cResponseStreamControl.h"
#include "globals.h"

cResponseStreamControl::cResponseStreamControl(struct MHD_Connection *connection, cSession *session, cDaemonParameter *daemonParameter)
	: cResponseHandler(connection, session, daemonParameter) {};

int cResponseStreamControl::toXml() {

    if(!this->user->Rights().Streaming()) {
        dsyslog("xmlapi: The user %s doesn't have the permission to do any action on /streamcontrol.xml", this->user->Name().c_str());
        return this->handle403Error();
    }

    const char* removeid = MHD_lookup_connection_value(this->connection, MHD_GET_ARGUMENT_KIND, "remove");
    if(removeid != NULL)
    {
        int streamid = atoi(removeid);
        StreamControl->RemoveStream(streamid);
    }
    string xml = StreamControl->GetStreamsXML();
    char *page = (char *)malloc((xml.length() + 1) *sizeof(char));
    strcpy(page, xml.c_str());

    return this->create(strlen (page), (void *) page, MHD_RESPMEM_MUST_FREE)
    		->header("Content-Type", "text/xml")
			->header("Cache-Control", "no-cache")
			->cors()
			->flush();
};
