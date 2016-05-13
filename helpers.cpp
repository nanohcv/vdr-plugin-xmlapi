/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <cstring>
#include <algorithm>
#include <sstream>
#include "helpers.h"

vector<string> split(string str, char delimiter) {
    vector<string> internal;
    stringstream ss(str);
    string tok;

    while(getline(ss, tok, delimiter)) {
        internal.push_back(tok);
    }

    return internal;
}

void trim(string& str) {
    string::size_type pos = str.find_last_not_of(' ');
    if(pos != string::npos) {
        str.erase(pos + 1);
        pos = str.find_first_not_of(' ');
        if(pos != string::npos) str.erase(0, pos);
    }
    else str.erase(str.begin(), str.end());
}

void xmlEncode(string& data) {
    std::string buffer;
    buffer.reserve(data.size());
    for(size_t pos = 0; pos != data.size(); ++pos) {
        switch(data[pos]) {
            case '&':  buffer.append("&amp;");       break;
            case '\"': buffer.append("&quot;");      break;
            case '\'': buffer.append("&apos;");      break;
            case '<':  buffer.append("&lt;");        break;
            case '>':  buffer.append("&gt;");        break;
            default:   buffer.append(&data[pos], 1); break;
        }
    }
    data.swap(buffer);
}

string urlEncode(string str) {
    string new_str = "";
    unsigned char c;
    int ic;
    const char* chars = str.c_str();
    char bufHex[10];
    int len = strlen(chars);

    for(int i=0;i<len;i++){
        c = chars[i];
        ic = c;
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') new_str += c;
        else {
            sprintf(bufHex,"%X",c);
            if(ic < 16) 
                new_str += "%0"; 
            else
                new_str += "%";
            new_str += bufHex;
        }
    }
    return new_str;
}

string urlDecode(string str) {
    string ret;
    unsigned char ch;
    int i, ii, len = str.length();

    for (i=0; i < len; i++){
        if(str[i] != '%'){
            if(str[i] == '+')
                ret += ' ';
            else
                ret += str[i];
        }else{
            sscanf(str.substr(i + 1, 2).c_str(), "%x", &ii);
            ch = static_cast<unsigned char>(ii);
            ret += ch;
            i = i + 2;
        }
    }
    return ret;
}

string uint32ToString(unsigned int  value) {
    ostringstream temp;
    temp<<value;
    return temp.str();
}

string timeToString(time_t t) {
    ostringstream temp;
    temp<<t;
    return temp.str();
}

string intToString(int value) {
    ostringstream temp;
    temp<<value;
    return temp.str();
}

bool searchInString(string str, string search) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    std::transform(search.begin(), search.end(), search.begin(), ::tolower);
    if(str.find(search) != string::npos)
        return true;
    return false;
}