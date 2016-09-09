#include "cResponseVersion.h"

cResponseVersion::cResponseVersion(struct MHD_Connection *connection, cSession *session) {

	this->connection = connection;
	this->session = session;
};

int cResponseVersion::toXml(cPluginConfig config) {

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
