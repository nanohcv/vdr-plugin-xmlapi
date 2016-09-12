#include "cResponseChannels.h"

using namespace std;

cResponseChannels::cResponseChannels(struct MHD_Connection *connection, cSession *session, cDaemonParameter *daemonParameter)
	: cResponseHandler(connection, session, daemonParameter) {};

int cResponseChannels::toXml() {

	cPluginConfig config = this->daemonParameter->GetPluginConfig();


    string xml = this->channelsToXml();
    char *page = (char *)malloc((xml.length() + 1) *sizeof(char));
    strcpy(page, xml.c_str());

    return this->create(strlen (page), (void *) page, MHD_RESPMEM_MUST_FREE)
    		->header("Content-Type", "text/xml")
			->cors()
			->flush();
};

string cResponseChannels::channelsToXml() {
    string logourl = "logos/";
    if(!this->config.RelativeLogoUrl()) {
        string host = this->conInfo["Host"];
        if(host != "")
        {
            if(this->daemonParameter->GetDaemonPort() == this->config.GetHttpsPort())
            {
                logourl = "https://" + host + "/logos/";
            }
            else
            {
                logourl = "http://" + host + "/logos/";
            }
        }
    }
    string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    xml +=       "<groups>\n";
    string group = "Unsorted";
    bool firstgroup = true;
    for(int i=0; i<Channels.Count(); i++) {
        cChannel *channel = Channels.Get(i);
        if(channel->GroupSep() && firstgroup) {
            group = channel->Name();
            xml += "    <group name=\"" + group + "\">\n";
            firstgroup = false;
            continue;
        }
        if(channel->GroupSep() && !firstgroup) {
            xml += "    </group>\n";
            group = channel->Name();
            xml += "    <group name=\"" + group + "\">\n";
            continue;
        }
        if(!channel->GroupSep() && i == 0) {
            xml += "    <group name=\"" + group + "\">\n";
            firstgroup = false;
        }
        xml += "        <channel id=\"" +
                string(channel->GetChannelID().ToString()) +
                "\">\n";
        if(channel->Vpid() == 0 || channel->Vpid() == 1) {
            xml += "        <isradio>true</isradio>\n";
        }
        else
        {
            xml += "        <isradio>false</isradio>\n";
        }
        string name = channel->Name();
        string shortname = channel->ShortName();
        string logo = name;
        transform(logo.begin(), logo.end(), logo.begin(), ::tolower);
        replace(logo.begin(), logo.end(), '/', '-');
        logo = urlEncode(logo);
        xmlEncode(name);
        xmlEncode(shortname);
        xmlEncode(logo);
        xml += "            <name>" + name + "</name>\n";
        xml += "            <shortname>" + shortname + "</shortname>\n";
        xml += "            <logo>" + logourl + logo + ".png</logo>\n";
        xml += "        </channel>\n";
    }
    xml += "    </group>\n";
    xml += "</groups>\n";
    return xml;
};
