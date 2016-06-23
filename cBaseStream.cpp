/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cBaseStream.cpp
 * Author: karl
 * 
 * Created on 23. Juni 2016, 15:44
 */

#include <signal.h>
#include <wait.h>
#include <vdr/tools.h>
#include <vdr/thread.h>
#include "cBaseStream.h"

cBaseStream::cBaseStream(string ffmpegCmd, map<string, string> conInfo, bool hls)
    : cmd(ffmpegCmd), connectionInfo(conInfo), pid(-1), f(NULL),
      isHlsStream(hls) {
}

cBaseStream::cBaseStream(const cBaseStream& src) 
    : cmd(src.cmd), connectionInfo(src.connectionInfo), pid(src.pid),
      f(src.f), isHlsStream(src.isHlsStream) {
}

cBaseStream::~cBaseStream() {
    this->Close();
}

string cBaseStream::GetClientIP() {
    return this->connectionInfo["ClientIP"];
}

string cBaseStream::GetUserAgent() {
    return this->connectionInfo["User-Agent"];
}

pid_t cBaseStream::GetPid() {
    return this->pid;
}

bool cBaseStream::IsHlsStream() {
    return this->isHlsStream;
}

bool cBaseStream::Open(const char* Command, const char* Mode) {
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

int cBaseStream::Close() {
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