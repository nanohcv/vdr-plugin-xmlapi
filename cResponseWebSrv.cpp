#include "cResponseWebSrv.h"

cResponseWebSrv::cResponseWebSrv(struct MHD_Connection *connection, cSession *session, cDaemonParameter *daemonParameter)
	: cResponseHandler(connection, session, daemonParameter), extHeaders(this->config.GetWebSrvHeadersFile()) {};

int cResponseWebSrv::toFile(const char* url) {

    string file = this->config.GetWebSrvRoot() + string(url).substr(7);
    if(endswith(file.c_str(), "/"))
        file += "index.html";
    int fd;
    struct stat sbuf;
    if ( (-1 == (fd = open (file.c_str(), O_RDONLY))) ||
        (0 != fstat (fd, &sbuf)) ) {
         if (fd != -1)
             close (fd);
         return this->handle404Error();
    }
    if(S_ISDIR(sbuf.st_mode)) {
        if(fd != -1)
            close(fd);
        return this->handle404Error();
    }

    string fileExt = "";
    size_t pos = file.find_last_of(".");
    if(pos != string::npos) {
        if(pos + 1 < file.length()) {
            fileExt = file.substr(file.find_last_of(".") + 1);
        }
    }

    dsyslog("xmlapi: Request file %s with extension %s", file.c_str(), fileExt.c_str());

    this->create(sbuf.st_size, fd);

    vector<cResponseHeader> headers = this->extHeaders[fileExt];
    for(vector<cResponseHeader>::iterator it = headers.begin(); it != headers.end(); ++it) {
        this->header(it->Key().c_str(), it->Value().c_str());
    }
    return this->cors()->flush();
};
