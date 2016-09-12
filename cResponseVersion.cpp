#include "cResponseVersion.h"

cResponseVersion::cResponseVersion(struct MHD_Connection *connection, cSession *session, cDaemonParameter *daemonParameter)
	: cResponseHandler(connection, session, daemonParameter) {};

int cResponseVersion::toXml() {

	cPluginConfig config = this->daemonParameter->GetPluginConfig();

    string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
                 "<plugin>\n"
                 "    <name>" + config.GetPluginName() + "</name>\n"
                 "    <version>" + config.GetVersion() + "</version>\n"
                 "</plugin>\n";

    char *page = (char *)malloc((xml.length() + 1) *sizeof(char));
    strcpy(page, xml.c_str());

    this->create(strlen (page), (void *) page,  MHD_RESPMEM_MUST_FREE)
		->header("Content-Type", "text/xml")
		->cors();

	return this->flush();
};
