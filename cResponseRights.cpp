#include "cResponseRights.h"
#include "globals.h"

cResponseRights::cResponseRights(struct MHD_Connection *connection, cSession *session, cDaemonParameter *daemonParameter)
	: cResponseHandler(connection, session, daemonParameter) {};

int cResponseRights::toXml() {

	const cUser *user = SessionControl->GetUserBySessionId(this->session->GetSessionId());

    string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    xml += "<rights>\n";
    xml += "    <streaming>" + string(user->Rights().Streaming() ? "true" : "false") + "</streaming>\n";
    xml += "    <timers>" + string(user->Rights().Timers() ? "true" : "false") + "</timers>\n";
    xml += "    <recordings>" + string(user->Rights().Recordings() ? "true" : "false") + "</recordings>\n";
    xml += "    <remotecontrol>" + string(user->Rights().RemoteControl() ? "true" : "false") + "</remotecontrol>\n";
    xml += "    <streamcontrol>" + string(user->Rights().StreamControl() ? "true" : "false") + "</streamcontrol>\n";
    xml += "</rights>\n";

    char *page = (char *)malloc((xml.length() + 1) *sizeof(char));
    strcpy(page, xml.c_str());

    this->create(strlen (page), (void *) page,  MHD_RESPMEM_MUST_FREE)
		->header("Content-Type", "text/xml")
		->cors();

	return this->flush();
};
