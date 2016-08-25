/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   cPreset.h
 * Author: karl
 *
 * Created on 10. Februar 2016, 14:31
 */

#ifndef CPRESET_H
#define CPRESET_H

#include <string>

using namespace std;

class cPreset {
public:
    cPreset(string cmd, string mimetype, string extension);
    cPreset(const cPreset& src);
    virtual ~cPreset();

    cPreset& operator = (const cPreset& src);

    string FFmpegCmd(string input, int start = 0);
    string GetCmd();
    string MimeType();
    string Extension();
private:
    string cmd;
    string mimetype;
    string extension;



};

#endif /* CPRESET_H */

