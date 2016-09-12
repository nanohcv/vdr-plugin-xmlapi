#include "cResponseLogo.h"

cResponseLogo::cResponseLogo(struct MHD_Connection *connection, cSession *session, cDaemonParameter *daemonParameter)
	: cResponseHandler(connection, session, daemonParameter) {};

int cResponseLogo::toImage(const char* url) {

    struct stat sbuf;
    int fd;
    string logofile = this->config.GetConfigDir() + url;

    if ( (-1 == (fd = open (logofile.c_str(), O_RDONLY))) ||
       (0 != fstat (fd, &sbuf)) ) {
        if (fd != -1)
            close (fd);
        logofile = this->config.GetConfigDir() + "/logos/default-logo.png";
        if ( (-1 == (fd = open (logofile.c_str(), O_RDONLY))) ||
            (0 != fstat (fd, &sbuf)) ) {
             if (fd != -1)
                 close (fd);
             return this->handle404Error();
        }
    }

    return this->create(sbuf.st_size, fd)
    		->header("Content-Type", "image/png")
			->cors()
			->flush();
};
