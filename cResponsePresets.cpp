#include "cResponsePresets.h"

cResponsePresets::cResponsePresets(struct MHD_Connection *connection, cSession *session, cDaemonParameter *daemonParameter)
	: cResponseHandler(connection, session, daemonParameter),
	  presets(daemonParameter->GetPluginConfig().GetPresetsFile()),
      hlsPresets(daemonParameter->GetPluginConfig().GetHlsPresetsFile()) {};

int cResponsePresets::toIni() {

    const char* hls = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "hls");

    string ini;

    if(hls == NULL) {
        if(this->presets.GetPresetNames().size() != 0)
        {
            vector<string> presetNames = this->presets.GetPresetNames();
            for(vector<string>::iterator it = presetNames.begin(); it != presetNames.end(); ++it) {
                cPreset preset = this->presets[*it];
                ini += "[" + *it + "]\n";
                ini += "Cmd=" + preset.GetCmd() + "\n";
                ini += "MimeType=" + preset.MimeType() + "\n";
                ini += "Ext=" + preset.Extension() + "\n\n";
            }
        } else {
            cPreset dp = presets.GetDefaultPreset();
            ini += "[Default]\n";
            ini += "Cmd=" + dp.GetCmd() + "\n";
            ini += "MimeType=" + dp.MimeType() + "\n";
            ini += "Ext=" + dp.Extension() + "\n\n";
        }
    } else {
        if(this->hlsPresets.GetPresetNames().size() != 0) {
            vector<string> presetNames = this->hlsPresets.GetPresetNames();
            for(vector<string>::iterator it = presetNames.begin(); it != presetNames.end(); ++it) {
                cHlsPreset preset = this->hlsPresets[*it];
                ini += "[" + *it + "]\n";
                ini += "Cmd=" + preset.Cmd() + "\n";
                ini += "StreamTimeout=" + intToString(preset.StreamTimeout()) + "\n";
                ini += "MinSegments=" + intToString(preset.MinSegments()) + "\n\n";
            }
        } else {
            cHlsPreset p = hlsPresets.GetDefaultPreset();
            ini += "[Default]\n";
            ini += "Cmd=" + p.Cmd() + "\n";
            ini += "StreamTimeout=" + intToString(p.StreamTimeout()) + "\n";
            ini += "MinSegments=" + intToString(p.MinSegments()) + "\n\n";
        }
    }
    char *page = (char *)malloc((ini.length() + 1) * sizeof(char));
    strcpy(page, ini.c_str());

    return this->create(strlen (page), (void *) page, MHD_RESPMEM_MUST_FREE)
		->header("Content-Type", "text/plain")
		->cors()
		->flush();
};
