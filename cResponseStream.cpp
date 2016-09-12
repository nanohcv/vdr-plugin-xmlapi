#include "cResponseStream.h"

cResponseStream::cResponseStream(struct MHD_Connection *connection, cSession *session, cDaemonParameter *daemonParameter)
	: cResponseHandler(connection, session, daemonParameter), presets(this->config.GetPresetsFile()) {

	this->input = "";
	this->streamid = new int;
};

cResponseStream::~cResponseStream() {

	this->url = NULL;
	this->cstr_preset = NULL;
	this->chid = NULL;
	this->recfile = NULL;
	this->mimeType = NULL;
	this->streamid = NULL;
}

int cResponseStream::toStream(const char *url) {

    if(!this->user->Rights().Streaming()) {
        dsyslog("xmlapi: The user %s doesn't have the permission to access streams", this->user->Name().c_str());
        return this->handle403Error();
    }

    this->url = url;

    if (this->initStream() == 0) {
    	return this->stream();
    }

    return this->handle403Error();
};

int cResponseStream::initStream() {

	this->cstr_preset = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "preset");
    if(this->cstr_preset == NULL) {
        esyslog("xmlapi: stream -> No preset given!");
        return this->handle404Error();
    }
    cPreset preset = this->presets[(string)this->cstr_preset];

    string presetName = this->cstr_preset;

    string ext = ((string)this->url).substr(((string)this->url).find_last_of("."));

    if (ext != preset.Extension()) {
        esyslog("xmlapi: Url %s doesn't end with stream%s", url, preset.Extension().c_str());
        return this->handle404Error();
    }
    this->mimeType = preset.MimeType().c_str();

    chid = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "chid");
    this->recfile = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "filename");

    if(this->chid == NULL && this->recfile == NULL)
    {
        esyslog("xmlapi: stream -> No input given!");
        return this->handle404Error();
    }

    int starttime = 0;
    if (this->recfile != NULL) {

        const char* cstr_start = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "start");
        if(cstr_start != NULL) {
            starttime = atoi(cstr_start);
        }
    }

    if (this->initInput() == 0) {

        cStream *stream = new cStream(this->config.GetFFmpeg(), preset, this->conInfo);
        if(this->config.GetWaitForFFmpeg()) {
            StreamControl->WaitingForStreamsByUserAgentAndIP(this->conInfo["ClientIP"], this->conInfo["User-Agent"]);
            sleep(1);
        }

        *this->streamid = StreamControl->AddStream(stream);

        if(!stream->StartFFmpeg(this->input, starttime))
        {
            StreamControl->RemoveStream(*this->streamid);
            delete this->streamid;
            return this->handle404Error();
        }
        dsyslog("xmlapi: Stream started");

    }

	return 0;
};

int cResponseStream::initInput() {

    if (this->chid != NULL) {

        dsyslog("xmlapi: request %s?chid=%s&preset=%s", this->url, this->chid, this->cstr_preset);
        tChannelID id = tChannelID::FromString(chid);
        if(!id.Valid()) {
            esyslog("xmlapi: stream -> invalid chid given");
            return this->handle404Error();
        }
        string channelId(chid);
        this->input = this->config.GetStreamdevUrl() + channelId + ".ts";
    }

    if (this->recfile != NULL) {

        dsyslog("xmlapi: request %s?filename=%s&preset=%s", url, this->recfile, this->cstr_preset);
        cRecording *rec = Recordings.GetByName(recfile);
        if(rec == NULL) {
            dsyslog("xmlapi: No recording found with file name '%s'", recfile);
            return this->handle404Error();
        }
        string recfiles = "'" + string(rec->FileName()) + "/'*.ts";
        this->input = "concat:$(ls -1 " + recfiles + " | perl -0pe 's/\\n/|/g;s/\\|$//g')";
    }

	return 0;
};

int cResponseStream::stream() {

    return this->create(MHD_SIZE_UNKNOWN, 8*1024, &cResponseStream::stream_reader, this->streamid, &cResponseStream::clear_stream)
		->header("Content-Type", this->mimeType)
		->header("Cache-Control", "no-cache")
		->cors()
		->flush();
};

cResponseStream *cResponseStream::create(uint64_t size,
		   size_t block_size,
		   MHD_ContentReaderCallback crc, void *crc_cls,
		   MHD_ContentReaderFreeCallback crfc) {

	this->response = MHD_create_response_from_callback (
			size,
			block_size,
			crc,
			crc_cls,
			crfc
	);
	if (this->session != NULL) {
		this->header(MHD_HTTP_HEADER_SET_COOKIE, this->session->Cookie().c_str());
	}
	return this;
};


ssize_t cResponseStream::stream_reader(void* cls, uint64_t pos, char* buf, size_t max) {
    int *streamid = (int *)cls;
    StreamControl->Mutex.Lock();
    cStream *stream = (cStream*)StreamControl->GetStream(*streamid);
    ssize_t size = stream->Read(buf, max);
    StreamControl->Mutex.Unlock();
    return size;
}

void cResponseStream::clear_stream(void* cls) {
    int *streamid = (int *)cls;
    StreamControl->Mutex.Lock();
    cStream *stream = (cStream*)StreamControl->GetStream(*streamid);
    if(stream != NULL)
    {
        stream->StopFFmpeg();
        StreamControl->RemoveStream(*streamid);
    }
    StreamControl->Mutex.Unlock();
    delete streamid;
    dsyslog("xmlapi: Stream stopped");
}
