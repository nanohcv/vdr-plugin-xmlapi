/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cDaemonParameter.h
 * Author: karl
 *
 * Created on 5. Mai 2016, 07:15
 */

#ifndef CDAEMONPARAMETER_H
#define CDAEMONPARAMETER_H

#include "cPluginConfig.h"

class cDaemonParameter {
public:
    cDaemonParameter(cPluginConfig config, int port);
    cDaemonParameter(const cDaemonParameter& src);
    virtual ~cDaemonParameter();
    
    cDaemonParameter& operator = (const cDaemonParameter& src);
    
    cPluginConfig GetPluginConfig();
    int GetDaemonPort();
    
private:
    cPluginConfig pluginConfig;
    int daemonPort;

};

#endif /* CDAEMONPARAMETER_H */

