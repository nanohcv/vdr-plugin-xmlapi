#include "cResponseStreamControl.h"
#include "globals.h"

cResponseStreamControl::cResponseStreamControl(struct MHD_Connection *connection, cSession *session, cDaemonParameter *daemonParameter)
	: cResponseHandler(connection, session, daemonParameter) {};

int cResponseStreamControl::toXml() {

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
