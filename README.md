# vdr-plugin-xmlapi
VDR Plugin for the VDR Windows Phone / Windows 10 Uwp App

### 1. Requirements
#### 1.1 Libraries
- libmicrohttpd (Ubuntu: sudo apt-get install libmicrohttpd10)
- libmicrohttpd headers (Ubuntu: sudo apt-get install libmicrohttpd-dev)

#### 1.2 VDR-Plugins
- streamdev-server

#### 1.3 Others
- FFmpeg


### 2. Install
*Comming soon...*


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


You can change the user name in this line:

    UserName=xmlapi


You can change the default random password in this line:

    Password=z_odRd*Q1L%Z


To disable Basic-Authentication set user name and password to nothing. 
It should look like this:

    UserName=
    Password=


If FFMpeg can't be found in the global search PATH then you can set the 
path to your ffmpeg binray with the following parameter:

    FFMPEG=/path/to/your/ffmpeg


The plugin use presets for transcoding. The presets can be configured in the 
presets.ini which was created on first start in the plugin config folder of the 
vdr plugin configuration folde. For information about the presets, read section
4. The path to the file can set with this parameter:

    Presets=/var/lib/vdr/plugins/xmlapi/presets.ini


If the streamdev-server plugin doesnt run on port 3000 change the following
parameter. Make sure that the url ends with a slash.

    StreamdevUrl=http://127.0.0.1:3000/


### 4. Presets - presets.ini
To transcode a channel you have to open the url 
http://server:port/stream{extenstion}?preset={preset}&chid={channelId}
The ffmpeg parameter -i must be set to {infile} and the output file must be set
to pipe:1

The presets.ini should look like this:

    [preset name]
    Cmd=your ffmepg parameter
    MimeType=your Mime Type
    Ext=your file extension


An short (not working) examle looks like this:

    [Low]
    Cmd=-i "{infile}" -f mpegts -vcodec libx264 -bufsize 1400k -maxrate 700k -acodec libmp3lame -ab 64k -ar 44100 pipe:1
    MimeType=video/mpeg
    Ext=.ts


For more information read the FFMPEG documentation. I recommend to copy a 
default preset and adjust the settings.
