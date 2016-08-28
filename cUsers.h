/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cUsers.h
 * Author: karl
 *
 * Created on 29. Mai 2016, 08:48
 */

#ifndef CUSERS_H
#define CUSERS_H

#include <string>
#include <vector>
#include "cUser.h"
#include "helpers.h"

using namespace std;

class cUsers : public vector<cUser> {
public:
    cUsers();
    cUsers(const cUsers& src);
    virtual ~cUsers();
    
    void ReadFromINI(string userIniFile);
    
    bool MatchUser(char *name, char *password);
    cUser GetUser(char *name);
    
private:

};

#endif /* CUSERS_H */

