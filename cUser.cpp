/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cUser.cpp
 * Author: karl
 * 
 * Created on 29. Mai 2016, 08:33
 */

#include "cUser.h"

cUser::cUser() : name("Anonymous"), password(""), rights(true) {
}

cUser::cUser(string name, string password, cRights rights)
    : name(name), password(password), rights(rights) {
}

cUser::cUser(const cUser& src)
    : name(src.name), password(src.password), rights(src.rights) {
}

cUser::~cUser() {
}

cUser& cUser::operator =(const cUser& src) {
    if(this != &src) {
        this->name = src.name;
        this->password = src.password;
        this->rights = src.rights;
    }
    return *this;
}

string cUser::Name() {
    return this->name;
}

string cUser::Password() {
    return this->password;
}

cRights cUser::Rights() {
    return this->rights;
}