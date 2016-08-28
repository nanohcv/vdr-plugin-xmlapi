/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cUser.h
 * Author: karl
 *
 * Created on 29. Mai 2016, 08:33
 */

#ifndef CUSER_H
#define CUSER_H

#include <string>
#include "cRights.h"

using namespace std;

class cUser {
public:
    cUser();
    cUser(string name, string password, cRights rights);
    cUser(const cUser& src);
    virtual ~cUser();
    
    cUser& operator = (const cUser& src);
    
    string Name();
    string Password();
    cRights Rights();
    
private:
    string name;
    string password;
    cRights rights;

};

#endif /* CUSER_H */

