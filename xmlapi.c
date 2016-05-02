/*
 * xmlapi.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include <vdr/plugin.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include "cPluginConfig.h"
#include "cWebServer.h"

static const char *VERSION        = "1.0.1";
static const char *DESCRIPTION    = "Enter description for 'xmlapi' plugin";
/*
static const char *MAINMENUENTRY  = "Xmlapi";
*/

class cPluginXmlapi : public cPlugin {
private:
    // Add any member variables or functions you may need here.
    cWebServer *srv;
    
public:
    cPluginXmlapi(void);
    virtual ~cPluginXmlapi();
    virtual const char *Version(void) { return VERSION; }
    virtual const char *Description(void) { return DESCRIPTION; }
/*
    virtual const char *CommandLineHelp(void);
    virtual bool ProcessArgs(int argc, char *argv[]);
*/
/*
    virtual bool Initialize(void);
*/
    virtual bool Start(void);
    virtual void Stop(void);
/*
    virtual void Housekeeping(void);
    virtual void MainThreadHook(void);
*/
    virtual cString Active(void);
/*
    virtual time_t WakeupTime(void);
    virtual const char *MainMenuEntry(void) { return MAINMENUENTRY; }
    virtual cOsdObject *MainMenuAction(void);
    virtual cMenuSetupPage *SetupMenu(void);
    virtual bool SetupParse(const char *Name, const char *Value);
    virtual bool Service(const char *Id, void *Data = NULL);
    virtual const char **SVDRPHelpPages(void);
    virtual cString SVDRPCommand(const char *Command, const char *Option, int &ReplyCode);
*/
};

cPluginXmlapi::cPluginXmlapi(void)
{
    // Initialize any member variables here.
    // DON'T DO ANYTHING ELSE THAT MAY HAVE SIDE EFFECTS, REQUIRE GLOBAL
    // VDR OBJECTS TO EXIST OR PRODUCE ANY OUTPUT!
    srv = NULL;
}

cPluginXmlapi::~cPluginXmlapi()
{
    // Clean up after yourself!
    delete srv;
}

/*
const char *cPluginXmlapi::CommandLineHelp(void)
{
    // Return a string that describes all known command line options.
    return NULL;
}

bool cPluginXmlapi::ProcessArgs(int argc, char *argv[])
{
    // Implement command line argument processing here if applicable.
    int c;
    opterr = 0;
    while ((c = getopt (argc, argv, "c:")) != -1)
    {
        switch (c)
        {
            case 'c':
                conf_file = optarg;
                break;
            case '?':
                if (optopt == 'c')
                    esyslog("xmlapi: Option -%c requires an argument.", optopt);
                else if (isprint (optopt))
                    esyslog("xmlapi: Unknown option `-%c'.", optopt);
                else
                    esyslog("xmlapi: Unknown option character `\\x%x'.", optopt);
                return false;
            default:
                return false;
        }
    }
    dsyslog("xmlapi: %s", conf_file);
    return true;
}
*/

/*
bool cPluginXmlapi::Initialize(void)
{
    // Initialize any background activities the plugin shall perform.
    return true;
}
*/

bool cPluginXmlapi::Start(void)
{
    cPluginConfig config(ConfigDirectory(Name()), Name(), VERSION);
    dsyslog("xmlapi: Config-File=%s", config.GetConfigFile().c_str());
    dsyslog("xmlapi: Preset.ini=%s", config.GetPresetsFile().c_str());
    dsyslog("xmlapi: HTTP-Port=%d, HTTPs-Port=%d, Use HTTPs=%d, HTTPS only=%d", 
                     config.GetHttpPort(), config.GetHttpsPort(), 
                     config.GetUseHttps(), config.GetHttpsOnly());
    dsyslog("xmlapi: User name=%s", config.GetUserName().c_str());
    if(srv == NULL)
    {
        srv = new cWebServer(config);
        return srv->Start();
    }
    return false;
}

void cPluginXmlapi::Stop(void)
{
    // Stop any background activities the plugin is performing.
    if(srv != NULL)
    {
        srv->Stop();
    }
}

/*
void cPluginXmlapi::Housekeeping(void)
{
    // Perform any cleanup or other regular tasks.
}

void cPluginXmlapi::MainThreadHook(void)
{
    // Perform actions in the context of the main program thread.
    // WARNING: Use with great care - see PLUGINS.html!
}
*/

cString cPluginXmlapi::Active(void)
{
    // Return a message string if shutdown should be postponed
    return NULL;
}

/*
time_t cPluginXmlapi::WakeupTime(void)
{
    // Return custom wakeup time for shutdown script
    return 0;
}

cOsdObject *cPluginXmlapi::MainMenuAction(void)
{
    // Perform the action when selected from the main VDR menu.
    return NULL;
}

cMenuSetupPage *cPluginXmlapi::SetupMenu(void)
{
    // Return a setup menu in case the plugin supports one.
    return NULL;
}

bool cPluginXmlapi::SetupParse(const char *Name, const char *Value)
{
    // Parse your own setup parameters and store their values.
    return false;
}

bool cPluginXmlapi::Service(const char *Id, void *Data)
{
    // Handle custom service requests from other plugins
    return false;
}

const char **cPluginXmlapi::SVDRPHelpPages(void)
{
    // Return help text for SVDRP commands this plugin implements
    return NULL;
}

cString cPluginXmlapi::SVDRPCommand(const char *Command, const char *Option, int &ReplyCode)
{
    // Process SVDRP commands this plugin implements
    return NULL;
}
*/

VDRPLUGINCREATOR(cPluginXmlapi); // Don't touch this!
