/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   helpers.h
 * Author: karl
 *
 * Created on 14. Februar 2016, 08:25
 */

#ifndef HELPERS_H
#define HELPERS_H

#include <string>
#include <vector>
#include <ctime>

using namespace std;

vector<string> split(string str, char delimiter);
void trim(string& str);
bool startswith(const char *pre, const char *str);
void xmlEncode(string& data);

string uint32ToString(unsigned int value);
string timeToString(time_t t);
string intToString(int value);

#endif /* HELPERS_H */

