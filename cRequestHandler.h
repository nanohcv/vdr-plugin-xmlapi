/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   cRequestHandler.h
 * Author: karl
 *
 * Created on 7. Februar 2016, 12:10
 */
#include <microhttpd.h>
#include <unistd.h>
#include <map>
#include <string>
#include <vdr/timers.h>
#include "cDaemonParameter.h"
#include "cPluginConfig.h"
#include "cPreset.h"
#include "cPresets.h"
#include "cUser.h"

#ifndef CREQUESTHANDLER_H
#define CREQUESTHANDLER_H

class cRequestHandler {
public:
    cRequestHandler(struct MHD_Connection *connection, cDaemonParameter *daemonParameter);
    virtual ~cRequestHandler();
    int HandleRequest(const char *url);
    void SetUser(cUser user);
private:
    struct MHD_Connection *connection;
    cDaemonParameter *daemonParameter;
    cPluginConfig config;
    cPresets presets;
    cUser user;
    

    int handleVersion();
    int handleStream(const char *url);
    int handleRecStream(const char *url);
    int handleStreamControl();
    int handleLogos(const char *url);
    int handlePresets();
    int handleChannels();
    string channelsToXml();
    int handleRecordings();
    int handleDeletedRecordings();
    string recordingsToXml(bool deleted = false);
    int handleTimers();
    string timersToXml();
    cTimer * GetTimer(const char *tid);
    const cEvent * GetEvent(tChannelID channelid, tEventID eid);
    bool deleteTimer(const char *tid);
    bool onOffTimer(const char *tid);
    bool addTimer(const char *channelid, const char *eventid);
    bool addTimer(const char *channelid, const char *name, const char *aux,
                    const char *cstr_flags, const char *cstr_weekdays,
                    const char *cstr_day, const char *cstr_start, const char *cstr_stop,
                    const char *cstr_priority, const char *cstr_lifetime);
    int handleEPG();
    string eventsToXml(const char *chid, const char *at);
    string searchEventsToXml(const char* chid, string search, string options);

    int handle404Error();

    static ssize_t stream_reader (void *cls, uint64_t pos, char *buf, size_t max);
    static void clear_stream(void *cls);

    std::map<std::string, std::string> conInfo;

};

#endif /* CREQUESTHANDLER_H */

