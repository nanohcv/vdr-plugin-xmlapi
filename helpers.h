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
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

#define READ 0
#define WRITE 1

using namespace std;

vector<string> split(string str, char delimiter);
void trim(string& str);
void xmlEncode(string& data);

string uint32ToString(unsigned int value);
string timeToString(time_t t);
string intToString(int value);

pid_t popen2(const char *command, int *infp, int *outfp);


#endif /* HELPERS_H */

