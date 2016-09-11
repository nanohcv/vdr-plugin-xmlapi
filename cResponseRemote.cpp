#include "cResponseRemote.h"
#include "globals.h"

cResponseRemote::cResponseRemote(struct MHD_Connection *connection, cSession *session, cDaemonParameter *daemonParameter)
	: cResponseHandler(connection, session, daemonParameter) {

	this->initRemoteKeys();
};

int cResponseRemote::toXml() {

    if(!this->user->Rights().RemoteControl()) {
        dsyslog("xmlapi: The user %s don't have the permission to send a remote command", this->user->Name().c_str());
        return this->handle403Error();
    }

    string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    const char* ckey = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "key");
    if(ckey == NULL) {
        return this->handle404Error();
    }
    string key(ckey);
    map<string, eKeys>::iterator it = this->remoteKeys.find(key);
    if(it != this->remoteKeys.end()) {
        cRemote::Put(it->second);
        xml += "<status>Ok</status>\n";
    } else {
        return this->handle404Error();
    }

    char *page = (char *)malloc((xml.length() + 1) *sizeof(char));
    strcpy(page, xml.c_str());

    this->create(strlen (page), (void *) page,  MHD_RESPMEM_MUST_FREE)
		->header("Content-Type", "text/xml")
		->cors();

	return this->flush();
};

void cResponseRemote::initRemoteKeys() {
    this->remoteKeys.insert(pair<string, eKeys>("up", kUp));
    this->remoteKeys.insert(pair<string, eKeys>("down", kDown));
    this->remoteKeys.insert(pair<string, eKeys>("menu", kMenu));
    this->remoteKeys.insert(pair<string, eKeys>("ok", kOk));
    this->remoteKeys.insert(pair<string, eKeys>("back", kBack));
    this->remoteKeys.insert(pair<string, eKeys>("left", kLeft));
    this->remoteKeys.insert(pair<string, eKeys>("right", kRight));
    this->remoteKeys.insert(pair<string, eKeys>("red", kRed));
    this->remoteKeys.insert(pair<string, eKeys>("green", kGreen));
    this->remoteKeys.insert(pair<string, eKeys>("yellow", kYellow));
    this->remoteKeys.insert(pair<string, eKeys>("blue", kBlue));
    this->remoteKeys.insert(pair<string, eKeys>("0", k0));
    this->remoteKeys.insert(pair<string, eKeys>("1", k1));
    this->remoteKeys.insert(pair<string, eKeys>("2", k2));
    this->remoteKeys.insert(pair<string, eKeys>("3", k3));
    this->remoteKeys.insert(pair<string, eKeys>("4", k4));
    this->remoteKeys.insert(pair<string, eKeys>("5", k5));
    this->remoteKeys.insert(pair<string, eKeys>("6", k6));
    this->remoteKeys.insert(pair<string, eKeys>("7", k7));
    this->remoteKeys.insert(pair<string, eKeys>("8", k8));
    this->remoteKeys.insert(pair<string, eKeys>("9", k9));

    this->remoteKeys.insert(pair<string, eKeys>("info", kInfo));
    this->remoteKeys.insert(pair<string, eKeys>("play", kPlay));
    this->remoteKeys.insert(pair<string, eKeys>("pause", kPause));
    this->remoteKeys.insert(pair<string, eKeys>("stop", kStop));
    this->remoteKeys.insert(pair<string, eKeys>("record", kRecord));
    this->remoteKeys.insert(pair<string, eKeys>("fastfwd", kFastFwd));
    this->remoteKeys.insert(pair<string, eKeys>("fastrew", kFastRew));
    this->remoteKeys.insert(pair<string, eKeys>("next", kNext));
    this->remoteKeys.insert(pair<string, eKeys>("prev", kPrev));
    this->remoteKeys.insert(pair<string, eKeys>("power", kPower));
    this->remoteKeys.insert(pair<string, eKeys>("chanup", kChanUp));
    this->remoteKeys.insert(pair<string, eKeys>("chandn", kChanDn));
    this->remoteKeys.insert(pair<string, eKeys>("chanprev", kChanPrev));
    this->remoteKeys.insert(pair<string, eKeys>("volup", kVolUp));
    this->remoteKeys.insert(pair<string, eKeys>("voldn", kVolDn));
    this->remoteKeys.insert(pair<string, eKeys>("mute", kMute));
    this->remoteKeys.insert(pair<string, eKeys>("audio", kAudio));
    this->remoteKeys.insert(pair<string, eKeys>("subtitles", kSubtitles));
    this->remoteKeys.insert(pair<string, eKeys>("schedule", kSchedule));
    this->remoteKeys.insert(pair<string, eKeys>("channels", kChannels));
    this->remoteKeys.insert(pair<string, eKeys>("timers", kTimers));
    this->remoteKeys.insert(pair<string, eKeys>("recordings", kRecordings));
    this->remoteKeys.insert(pair<string, eKeys>("setup", kSetup));
    this->remoteKeys.insert(pair<string, eKeys>("commands", kCommands));
    this->remoteKeys.insert(pair<string, eKeys>("user0", kUser0));
    this->remoteKeys.insert(pair<string, eKeys>("user1", kUser1));
    this->remoteKeys.insert(pair<string, eKeys>("user2", kUser2));
    this->remoteKeys.insert(pair<string, eKeys>("user3", kUser3));
    this->remoteKeys.insert(pair<string, eKeys>("user4", kUser4));
    this->remoteKeys.insert(pair<string, eKeys>("user5", kUser5));
    this->remoteKeys.insert(pair<string, eKeys>("user6", kUser6));
    this->remoteKeys.insert(pair<string, eKeys>("user7", kUser7));
    this->remoteKeys.insert(pair<string, eKeys>("user8", kUser8));
    this->remoteKeys.insert(pair<string, eKeys>("user9", kUser9));
    this->remoteKeys.insert(pair<string, eKeys>("none", kNone));
}
