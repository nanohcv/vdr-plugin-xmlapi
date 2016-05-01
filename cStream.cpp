/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cStream.cpp
 * Author: karl
 * 
 * Created on 1. Mai 2016, 07:38
 */

#include <microhttpd.h>
#include <unistd.h>
#include <wait.h>
#include <vdr/thread.h>
#include "cStream.h"

cStream::cStream(string ffmpegCmd, map<string, string> conInfo)
    : cmd(ffmpegCmd), connectionInfo(conInfo)
{
    this->pid = -1;
    this->f = NULL;
}

cStream::cStream(const cStream& src) {
    this->pid = src.pid;
    this->f = src.f;
    this->connectionInfo = src.connectionInfo;
    this->cmd = src.cmd;
}

cStream::~cStream() {
    this->Close();
}

cStream& cStream::operator =(const cStream& src) {
    if(this != &src)
    {
        this->pid = src.pid;
        this->f = src.f;
        this->connectionInfo = src.connectionInfo;
        this->cmd = src.cmd;
    }
    return *this;
}

string cStream::GetClientIP() {
    return this->connectionInfo["ClientIP"];
}

string cStream::GetUserAgent() {
    return this->connectionInfo["User-Agent"];
}

pid_t cStream::GetPid() {
    return this->pid;
}

bool cStream::StartFFmpeg() {
    if(this->f == NULL) {
        this->Open(cmd.c_str(), "r");
    }
    else {
        return false;
    }
    
    if(this->f == NULL) {
        esyslog("xmlapi: Cant start ffmpeg");
        return false;
    }
    
    return true;
}

void cStream::StopFFmpeg() {
    if(this->f != NULL) {
        this->Close();
        this->f = NULL;
    }
}

ssize_t cStream::Read(char* buf, size_t max) {
    ssize_t n = 0;
    if(this->f != NULL) {
        n = read(fileno(this->f), buf, max);
    }
    else {
        return MHD_CONTENT_READER_END_WITH_ERROR;
    }
    
    if (0 == n) {
        return MHD_CONTENT_READER_END_OF_STREAM;
    }
    if (n < 0) {
        return MHD_CONTENT_READER_END_WITH_ERROR;
    }
    return n;
}

bool cStream::Open(const char* Command, const char* Mode) {
    int fd[2];

    if (pipe(fd) < 0) {
        LOG_ERROR;
        return false;
    }
    if ((pid = fork()) < 0) { // fork failed
       LOG_ERROR;
       close(fd[0]);
       close(fd[1]);
       return false;
    }

    const char *mode = "w";
    int iopipe = 0;

    if (pid > 0) { // parent process
        if (strcmp(Mode, "r") == 0) {
           mode = "r";
           iopipe = 1;
        }
        close(fd[iopipe]);
        if ((f = fdopen(fd[1 - iopipe], mode)) == NULL) {
            LOG_ERROR;
            close(fd[1 - iopipe]);
            }
        return f != NULL;
    }
    else { // child process
        int iofd = STDOUT_FILENO;
        if (strcmp(Mode, "w") == 0) {
           iopipe = 1;
           iofd = STDIN_FILENO;
        }
        close(fd[iopipe]);
        if (dup2(fd[1 - iopipe], iofd) == -1) { // now redirect
            LOG_ERROR;
            close(fd[1 - iopipe]);
            _exit(-1);
        }
        else {
            int MaxPossibleFileDescriptors = getdtablesize();
            for (int i = STDERR_FILENO + 1; i < MaxPossibleFileDescriptors; i++)
                close(i); //close all dup'ed filedescriptors
            if (execl("/bin/sh", "sh", "-c", Command, NULL) == -1) {
               LOG_ERROR_STR(Command);
               close(fd[1 - iopipe]);
               _exit(-1);
               }
            }
        _exit(0);
    }
}

int cStream::Close() {
    int ret = -1;

    if (f) {
       fclose(f);
       f = NULL;
    }

    if (pid > 0) {
        int status = 0;
        int i = 5;
        while (i > 0) {
            ret = waitpid(pid, &status, WNOHANG);
            if (ret < 0) {
                if (errno != EINTR && errno != ECHILD) {
                   LOG_ERROR;
                   break;
                   }
                }
            else if (ret == pid)
               break;
            i--;
            cCondWait::SleepMs(100);
        }
        if (!i) {
            kill(pid, SIGKILL);
            ret = -1;
        }
        else if (ret == -1 || !WIFEXITED(status))
           ret = -1;
        pid = -1;
    }

    return ret;
}
