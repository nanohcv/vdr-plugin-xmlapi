/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cResponseHeader.cpp
 * Author: karl
 * 
 * Created on 29. August 2016, 10:37
 */

#include "cResponseHeader.h"

cResponseHeader::cResponseHeader(string key, string value) : key(key), value(value) {
}

cResponseHeader::cResponseHeader(const cResponseHeader& src) : key(src.key), value(src.value) {
}

cResponseHeader::~cResponseHeader() {
}

cResponseHeader& cResponseHeader::operator =(const cResponseHeader& src) {
    if(this != &src) {
        this->key = src.key;
        this->value = src.value;
    }
    return *this;
}

string cResponseHeader::Key() {
    return this->key;
}

string cResponseHeader::Value() {
    return this->value;
}

