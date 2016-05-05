/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cDaemonParameter.cpp
 * Author: karl
 * 
 * Created on 5. Mai 2016, 07:15
 */

#include "cDaemonParameter.h"

cDaemonParameter::cDaemonParameter(cPluginConfig config, int port)
    : pluginConfig(config), daemonPort(port) {
}

cDaemonParameter::cDaemonParameter(const cDaemonParameter& src) 
    : pluginConfig(src.pluginConfig), daemonPort(src.daemonPort) {
}

cDaemonParameter::~cDaemonParameter() {
}

cDaemonParameter& cDaemonParameter::operator =(const cDaemonParameter& src) {
    if(this != &src)
    {
        this->pluginConfig = src.pluginConfig;
        this->daemonPort = src.daemonPort;
    }
    return *this;
}

cPluginConfig cDaemonParameter::GetPluginConfig() {
    return this->pluginConfig;
}

int cDaemonParameter::GetDaemonPort() {
    return this->daemonPort;
}
