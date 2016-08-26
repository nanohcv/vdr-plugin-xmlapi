# vdr-plugin-xmlapi
VDR Plugin for the VDR Windows Phone / Windows 10 Uwp App

### 1. Requirements
#### 1.1 Libraries
- libmicrohttpd (Ubuntu: sudo apt-get install libmicrohttpd10)
- libmicrohttpd headers (Ubuntu: sudo apt-get install libmicrohttpd-dev)


#### 1.2 VDR-Plugins
- streamdev-server

    If encrypted channels are used, it is highly recommended to install streamdev
    with the following patches

    https://projects.vdr-developer.org/issues/2220

#### 1.3 Others
- FFmpeg


### 2. Install
If you installed vdr from source then follow the instructions from vdr wiki.
https://www.linuxtv.org/vdrwiki/index.php/Plugin_Installation

On Ubuntu (16.04):
Install requirements:

    sudo apt-get install vdr-dev vdr-plugin-streamdev-server libmicrohttpd10 libmicrohttpd-dev build-essential git ffmpeg

Download, compile and install the xmlapi plugin:

    cd ~
    git clone -b dev https://github.com/nanohcv/vdr-plugin-xmlapi.git
    cd vdr-plugin-xmlapi
    make
    sudo make install

Start plugin:

    printf "[xmlapi]\n" | sudo tee /etc/vdr/conf.avail/xmlapi.conf
    sudo ln -s /etc/vdr/conf.avail/xmlapi.conf /etc/vdr/conf.d/50-xmlapi.conf
    sudo systemctl restart vdr

Config file(s) should be found here:

    /var/lib/vdr/plugins/xmlapi/



### 3. Configuration Files
If the plugin does not found a configuration file, a xmlapi.conf file 
is created in the xmlapi subfolder of the VDR configuration plugin directory 
on first start.

The file has the following parameters.
Change the HTTP-Port in this line:

    HttpPort=10080


Change the HTTPS-Port in this line:

    HttpsPort=10443


If you want to enable SSL then set UseHttps to 1:
Make sure that you also set SSLKeyFile and SSLCertFile otherwise the 
UseHttps Parameter is ignored

    UseHttps=1
 

If you only want to activate SSL-Daemon, set HttpsOnly to 1. Make sure that 
UseHttps is also activated. Otherwise this parameter is ignored.

    HttpsOnly=0


If UseHttps is set to 1, then you have to set the path to the SSL Key file 
in the following line:

    SSLKeyFile=/path/to/your/server.key


If UseHttps is set to 1, then you have to set the path to the SSL Cert file 
in the following line:

    SSLCertFile=/path/to/your/server.pem

To change the path to users.ini, change the following line:

    Users=/var/lib/vdr/plugins/xmlapi/users.ini

By default only on transcoded stream can be started. If another client connect
to the plugin and request a stream, the plugin wait until all other ffmpeg 
processes, started by the plugin, are closed. You can change this by setting the
following parameter: (I recommend this only if you have more than one tuner.)

    WaitForFFmpeg=0


The plugin use presets for transcoding. The presets can be configured in the 
presets.ini and hls_presets.ini (for hls streaming) which created on first start in the plugin config folder of the 
vdr plugin configuration folder. For information about the presets, read section 4.
The path for the preset files can set with the following parameter:

    Presets=/var/lib/vdr/plugins/xmlapi/presets.ini
    HlsPresets=/var/lib/vdr/plugins/xmlapi/hls_presets.ini


If the streamdev-server plugin doesnt run on port 3000 change the following
parameter. Make sure that the url ends with a slash.

    StreamdevUrl=http://127.0.0.1:3000/


#### 3.1 Users - users.ini
Users can be configured in the users.ini file.
The file has two sections. The first section is called "AdminUsers" and the second called "Users".
AdminUsers can control streams, Users not. (not really implemented yet)
The file should look like this. Make sure that no spaces between username, "=" and password are added.

    [AdminUsers]
    xmlapi=abcd1234

    [Users]
    guest=4321dcba
  

#### 3.2 Presets - presets.ini
To transcode a channel you have to open the url 
http://server:port/stream{extenstion}?preset={preset}&chid={channelId}
The ffmpeg parameter -i must be set to {infile} and the output file must be set
to pipe:1
If the requested url contains the &start= parameter the placeholder {start} in the preset
is replaced by "-ss position"

The presets.ini should look like this:

    [preset name]
    Cmd=your ffmepg command with parameters
    MimeType=your Mime Type
    Ext=your file extension


An short (not working) examle looks like this:

    [Low]
    Cmd=ffmpeg {start} -i "{infile}" -f mpegts -vcodec libx264 -bufsize 1400k -maxrate 700k -acodec libmp3lame -ab 64k -ar 44100 pipe:1
    MimeType=video/mpeg
    Ext=.ts


For more information read the FFMPEG documentation. I recommend to copy a 
default preset and adjust the settings.

#### 3.3 Presets - hls_presets.ini
For HLS-Streaming the hls_presets.ini is used.
The ffmpeg parameter -i must be set to {infile} and the output format must be
"-f hls -hls_time 2 -hls_list_size 5 -hls_wrap 5 -hls_segment_filename '{hls_tmp_path}/{streamid}-%d.ts' {hls_tmp_path}/stream.m3u8"
Do not touch "-hls_segment_filename '{hls_tmp_path}/{streamid}-%d.ts' {hls_tmp_path}/stream.m3u8" !!
The parameters "-hls_time 2 -hls_list_size 5 -hls_wrap 5" are optional but highly recommended.

The hls_presets.ini should look like this:

    [preset name]
    Cmd=your ffmepg command with parameters
    StreamTimeout=Time (in seconds) after the stream is terminated, if no client access the stream anymore.
    MinSegments=Minimum number of segments which need to be created before a client can access the stream.m3u8


An short (not working) examle looks like this:

    [Low]
    Cmd=ffmpeg {start} -i "{infile}" -vcodec libx264 -bufsize 1400k -maxrate 700k -acodec libmp3lame -ab 64k -ar 44100 -f hls -hls_time 2 -hls_list_size 5 -hls_wrap 5 -hls_segment_filename '{hls_tmp_path}/{streamid}-%d.ts' {hls_tmp_path}/stream.m3u8
    StreamTimeout=5
    MinSegments=3



### 4. API

The plugin provides a simple API based on xml files.
All requests must be GET Requests

#### 4.1 Plugin name and version

    http(s)://<server-ip>:<port>/version.xml


#### 4.2 Channels

Get channels with groups, channel id, name, shortname and logos. The logos must
be placed in the subfolder "logos" in the plugin config directory.
For example: /var/lib/vdr/plugins/xmlapi/logos/arte.png
The logo must have the exact name (in lower case) like the channel. If the 
channel contains a slash "/", the slash must be replaced with "-" in the logo 
file name.

    http(s)://<server-ip>:<port>/channels.xml

Logos:

    http(s)://<server-ip>:<port>/logos/<channelname>.png


#### 4.3 Epg

To get whole EPG-Date just open the following url (Warning: this takes a long time)

    http(s)://<server-ip>:<port>/epg.xml


To get EPG-Data from a channel, you have to open this api:

    http(s)://<server-ip>:<port>/epg.xml?chid=<channel-id>

Parameter:

- "chid" (mandory) -> a channel id from the channels.xml
- "at" -> Values: now, next, [unix-timestamp]

Example:

    http(s)://<server-ip>:<port>/epg.xml?chid=C-61441-10014-11120&at=1455906892


You can search the EPG with the following Parameters:

- "search" -> the search string (The case doesn't matter)
- "options" -> A combination of "T", "S" and "D". T = search in Title, S = Search in Short text, "D" = Search in description

Example (Search for "Tatort" in Title and Description):

    http(s)://<server-ip>:<port>/epg.xml?search=Tatort&options=TD

To search only in the EPG of a channel you can add the channel id like this:

    http(s)://<server-ip>:<port>/epg.xml?search=Tatort&options=TD&chid=C-61441-10014-11120



#### 4.4 Recordings

You can get a list of all recordings with the following command:

    http(s)://<server-ip>:<port>/recordings.xml

To delete a recording you can use the following parameters

- "action" must be "delete"
- "filename" = the filename (url encoded) of the recording provided by the recordings.xml

Example:

    http(s)://<server-ip>:<port>/recordings.xml?filename=%2Fsrv%2FVideos%2FAufnahmen%2FTest2%2F2016-05-14.18.20.2-0.rec&action=delete

If a recording is deleted, it is not removed from hard disk. Deleted recordings can be found in the delete recording list.

    http(s)://<server-ip>:<port>/deletedrecordings.xml

To undelete or remove a recording you can use the following parameters

- "action" = "undelete or "remove"
- "filename" =  the filename (url encoded) of the recording provided by the deletedrecordings.xml

Example:

    http(s)://<server-ip>:<port>/recordings.xml?filename=%2Fsrv%2FVideos%2FAufnahmen%2FTest2%2F2016-05-14.18.20.2-0.del&action=remove


#### 4.5 Timers

To get all timers you can use the following api:

    http(s)://<server-ip>:<port>/timers.xml

To add a timer from a event, you can use the following parameters (all mandory):

- "action" -> must be "add"
    - "chid" -> a channel id
    - "eventid" -> a eventid from the channel events

Example:

    http(s)://<server-ip>:<port>/timers.xml?action=add&chid=C-1-1051-11100&eventid=20000

To add a new manual timer, you can use the following parameters (all mandory):

- "action" -> must be "add"
    - "chid" -> a channel id
    - "name" -> the timer name
    - "aux" -> a description
    - "flags" -> flags (see vdr description) 1 = active, 5 = active and use vps
    - "weekdays" -> The week days
    - "day" -> unix timestamp from a day at 0 a clock
    - "start" -> start time (eg. 1130 for 11:30)
    - "stop" -> stop time (eg. 1300 for 13:00)
    - "priority" -> priority
    - "lifetime" -> lifetime (99 for no lifetime)

Example:

    http(s)://<server-ip>:<port>/timers.xml?action=add&chid=C-1-1079-10351&name=Test&aux=Beschreibung&flags=1&weekdays=0&day=1463436000&start=1230&stop=1300&priority=50&lifetime=99


#### 4.6 Remote control

There are two APIs to control your VDR.
To switch to a channel use the following API:

    http(s)://<server-ip>:<port>/switch.xml?chid=<channel-id>

To send a remote control command use the following API

    http(s)://<server-ip>:<port>/remote.xml?key=<key>

Possible values for key are:

- up
- down
- menu
- ok
- back
- left
- right
- red
- green
- yellow
- blue
- 0
- 1
- 2
- 3
- 4
- 5
- 6
- 7
- 8
- 9
- info
- play
- pause
- stop
- record
- fastfwd
- fastrew
- next
- prev
- power
- chanup
- chandn
- chanprev
- volup
- voldn
- mute
- audio
- subtitles
- schedule
- channels
- timers
- recordings
- setup
- commands
- user0
- user1
- user2
- user3
- user4
- user5
- user6
- user7
- user8
- user9
- none



#### 4.7 Presets.ini

You can access the presets.ini with the following command:

    http(s)://<server-ip>:<port>/presets.ini

To get the hls_presets.ini, use the following command:

    http(s)://<server-ip>:<port>/presets.ini?hls=1


#### 4.8 Transcoded Streams

For streaming a channel you need a channel id and a preset (section from presets.ini).
You also need the extension (Ext= from presets.ini) from your choosen preset.

    http(s)://<server-ip>:<port>/stream{ext}?chid={channel-id}&preset={preset}

Parameter:

- "chid" (mandory) -> a channel id from the channels.xml
- "preset" (mandory) -> a preset from the presets.ini ([Section])

Example:

    http(s)://<server-ip>:<port>/stream.ts?chid=C-61441-10014-11120&preset=Low


You can also stream a recording:

    http(s)://<server-ip>:<port>/recstream{ext}?filename={filename from recordings.xml}&preset={preset}&start={jump to position in seconds}

Parameter:

- "filename" (mandory) -> the filename (url encoded) from the recordings.xml api
- "preset" (mandory) -> a preset from the presets.ini ([Section])
- "start" -> Jump to position (in seconds)

Example:
    
    http(s)://<server-ip>:<port>/recstream.ts?filename=%2Fsrv%2FVideos%2FAufnahmen%2FTest2%2F2016-05-14.18.20.2-0.rec&preset=Mid&start=300


It is also possible to stream via HLS protocol (HTTP Live Stream). Parameters are the same as above.

Example for channel stream:
    
    http(s)://<server-ip>:<port>/hls/stream.m3u8?chid=S19.2E-1-1019-10301&preset=Mid

Example for recording stream:

    http(s)://<server-ip>:<port>/hls/stream.m3u8?filename=%2Fsrv%2FVideos%2FAufnahmen%2FTest2%2F2016-05-14.18.20.2-0.rec&preset=Mid&start=300



#### 4.9 Stream control

To view the currently active streams or stop/remove a stream, you can use this API.

View:

    http(s)://<server-ip>:<port>/streamcontrol.xml

Stop/remove a stream:

    http(s)://<server-ip>:<port>/streamcontrol.xml?remove={streamid}
 
