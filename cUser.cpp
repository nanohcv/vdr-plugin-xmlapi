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

cUser::cUser() {
    name = "Anonymous";
    password = "";
    isAdmin = false;
}

cUser::cUser(string name, string password, bool isadmin)
    : name(name), password(password), isAdmin(isadmin) {
}

cUser::cUser(const cUser& src)
    : name(src.name), password(src.password), isAdmin(src.isAdmin) {
}

cUser::~cUser() {
}

cUser& cUser::operator =(const cUser& src) {
    if(this != &src) {
        this->name = src.name;
        this->password = src.password;
        this->isAdmin = src.isAdmin;
    }
    return *this;
}

string cUser::Name() {
    return this->name;
}

string cUser::Password() {
    return this->password;
}

bool cUser::IsAdmin() {
    return this->isAdmin;
}