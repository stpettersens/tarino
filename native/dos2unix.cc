/*
dos2unix implementation.
Copyright 2016 Sam Saint-Pettersen.

Native add-on.

Released under the MIT License;
see MIT-LICENSE file.
*/
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
using namespace std;

bool is_ascii(string filename) {
    bool ascii = false;
    int c = -1;
    ifstream f(filename.c_str());
    while((c = f.get()) != EOF && c <= 127);
    if(c == EOF) {
        ascii = true;
    }
    f.close();
    return ascii;
}

bool is_dos_eol(string filename) {
    bool dos_eol = false;
    string l;
    ifstream f;
    f.open(filename.c_str(), ios::binary);
    while(getline(f, l)) {
        for(char& c : l) {
            if (c == '\r') {
                dos_eol = true;
                break;
            }
        }
    }
    f.close();
    return dos_eol;
}

string to_unix_line_endings(string contents) {
    string ucontents;
    for(char& c : contents) {
        if(c != '\r') ucontents.push_back(c);
    }
    return ucontents;
}

string dos2unix_str(string filename) {
    ifstream i;
    stringstream contents;
    i.open(filename, ios::binary);
    contents << i.rdbuf();
    i.close();
    if(is_ascii(filename) && is_dos_eol(filename)) {
        return to_unix_line_endings(contents.str());
    }
    return contents.str();
}

int dos2unix(string filename) {
    if(is_ascii(filename) && is_dos_eol(filename)) {
        ifstream i;
        stringstream contents;
        i.open(filename, ios::binary);
        contents << i.rdbuf();
        i.close();
        string out = to_unix_line_endings(contents.str());
        ofstream o;
        o.open(filename, ios::out | ios::binary);
        o << out;
        o.close();
    }
    else {
        // Trigger "File already has UNIX line endings or is binary" in calling code.
        return 1;
    }
    return 0;
}
