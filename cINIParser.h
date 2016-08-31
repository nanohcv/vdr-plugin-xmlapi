/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   cINIParser.h
 * Author: karl
 *
 * Created on 10. Februar 2016, 07:12
 */

#ifndef CINIPARSER_H
#define CINIPARSER_H

#include <string>
#include <map>
#include <vector>

using namespace std;

class cINIParser : public map< string, map<string, string> > {
public:
    cINIParser(string iniFile);
    cINIParser(const cINIParser& src);
    virtual ~cINIParser();
    vector<string> GetKeys();

private:
    void parse(string iniFile);
    vector<string> keys;
};

#endif /* CINIPARSER_H */

