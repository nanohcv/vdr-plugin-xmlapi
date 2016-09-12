#include "cAuth.h"
#include "globals.h"

cAuth::cAuth(struct MHD_Connection *connection, cDaemonParameter *daemonParameter)
	: connection(connection),
	  daemonParameter(daemonParameter),
	  config(daemonParameter->GetPluginConfig()) {

	this->session = NULL;
};

cAuth::~cAuth() {

	this->connection = NULL;
	this->session = NULL;
};

bool cAuth::authSession() {

	bool hasSession = false;
	const char *cookie = NULL;
	cookie = MHD_lookup_connection_value (this->connection,
						MHD_COOKIE_KIND,
						"vdr-plugin-xmlapi_sessionid");


	if (cookie != NULL) {

		SessionControl->Mutex.Lock();
		string sessionid(cookie);
		cSession* session = SessionControl->GetSessionBySessionId(sessionid);

		if (session != NULL && !session->IsExpired()) {
			const cUser *sessionUser = SessionControl->GetUserBySessionId(sessionid);
			if(sessionUser != NULL) {
				this->user = this->config.GetUsers().GetUser(sessionUser->Name().c_str());
				this->session = session;
				this->session->UpdateStart();
				hasSession = true;
				dsyslog("xmlapi: authSession() -> authenticated user %s", this->user.Name().c_str());
			}
		}
		SessionControl->Mutex.Unlock();
	}

	return hasSession;
};

bool cAuth::authBasic() {

	bool validUser = true;
	const char* createAction = "1200";
    long lifetime = atol(createAction);

    if(!this->config.GetUsers().empty()) {
        char *user = NULL;
        char *pass = NULL;
        user = MHD_basic_auth_get_username_password (this->connection, &pass);
        validUser = user != NULL && this->config.GetUsers().MatchUser(user, pass);
        if (validUser) {
        	this->user = this->config.GetUsers().GetUser(user);
        	cSession session = SessionControl->AddSession(this->user, lifetime);
        	this->session = SessionControl->GetSessionBySessionId(session.GetSessionId());
			dsyslog("xmlapi: authBasic() -> authenticated user %s", this->user.Name().c_str());
        }
        if (user != NULL) free (user);
        if (pass != NULL) free (pass);
    } else {
    	cSession session = SessionControl->AddSession(this->user, lifetime);
    	this->user = this->config.GetUsers().GetUser("Anonymous");
    	this->session = SessionControl->GetSessionBySessionId(session.GetSessionId());
    }
    return validUser;
};

bool cAuth::authenticated() {

	return this->authSession() || this->authBasic();
};
