#include "cResponseSwitch.h"
#include "globals.h"

cResponseSwitch::cResponseSwitch(struct MHD_Connection *connection, cSession *session, cDaemonParameter *daemonParameter)
	: cResponseHandler(connection, session, daemonParameter) {};

int cResponseSwitch::toXml() {

	const cUser *user = SessionControl->GetUserBySessionId(this->session->GetSessionId());

    if(!user->Rights().RemoteControl()) {
        dsyslog("xmlapi: The user %s don't have the permission to switch to an channel", user->Name().c_str());
        return this->handle403Error();
    }

    string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    const char* chid = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "chid");
    if(chid == NULL) {
        return this->handle404Error();
    }
    tChannelID cid = tChannelID::FromString(chid);
    if(!cid.Valid()) {
        return this->handle404Error();
    }
    
#if VDRVERSNUM >= 20301
    LOCK_CHANNELS_READ;
    const cChannel *channel = Channels->GetByChannelID(cid);
#else
    cChannel *channel = Channels.GetByChannelID(cid);
#endif
    if(channel == NULL) {
        xml += "<status>\n";
        xml += "    <channel>" + string(chid) + "</channel>\n";
        xml += "    <switched>false</switched>\n";
        xml += "    <message>Channel not found</message>\n";
        xml += "</status>\n";
    } else {
        bool switched = cDevice::PrimaryDevice()->SwitchChannel(channel, true);
        if(switched) {
            xml += "<status>\n";
            xml += "    <channel>" + string(chid) + "</channel>\n";
            xml += "    <switched>true</switched>\n";
            xml += "    <message>Switched to channel</message>\n";
            xml += "</status>\n";
        } else {
            xml += "<status>\n";
            xml += "    <channel>" + string(chid) + "</channel>\n";
            xml += "    <switched>false</switched>\n";
            xml += "    <message>Switching to channel failed</message>\n";
            xml += "</status>\n";
        }
    }

    char *page = (char *)malloc((xml.length() + 1) *sizeof(char));
    strcpy(page, xml.c_str());

    this->create(strlen (page), (void *) page,  MHD_RESPMEM_MUST_FREE)
		->header("Content-Type", "text/xml")
		->cors();

	return this->flush();
};
