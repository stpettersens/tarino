/*
  tarino: a tar implementation with Node.js.
  Copyright 2016 Sam Saint-Pettersen.

  Native add-on.

  Dual licensed under the GPL and MIT licenses;
  see GPL-LICENSE and MIT-LICENSE respectively.
*/

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdio>
#include <cstring>
using namespace std;

static int EOF_PADDING = 512;

class TarEntry {
private:
    string part;
    string file;
    int size;
    int modified;
    int etype;

public:
    TarEntry(string part, string file, int size, int modified, int etype) {
        this->part = part;
        this->file = file;
        this->size = size;
        this->modified = modified;
        this->etype = etype;
    };

    string get_part() {
        return this->part;
    }

    string get_file() {
        return this->file;
    }

    int get_size() {
        return this->size;
    }

    int get_modified() {
        return this->modified;
    }

    int get_type() {
        return this->etype;
    }
};

string write_checksum(string, string);
string merge_entries(string, vector<string>);
void finalize_tar(string, string);

vector<string> split(string str, char delimiter) {
    vector<string> internal;
    stringstream ss(str);
    string token;
    while(getline(ss, token, delimiter)) {
        internal.push_back(token);
    }
    return internal;
}

string dec_to_padded_octal(int num, int length) {
    ostringstream octal, padded;
    octal << std::oct << num;
    string padding = "";
    if(length > (int)octal.str().length()) {
        for(int i = 0; i < (length - (int)octal.str().length()); i++) {
            padding.append("0");
        } 
    }
    padded << padding << octal.str();
    return padded.str();
}

string pad_data(size_t length) {
    string padding = "";
    for(int i = 1; i < (int)length; i++) {
        ostringstream nc;
        nc << (char)0;
        padding.append(nc.str());
    }
    return padding;
}

string write_padded_data(string data) {
    ostringstream padded;
    int eof = 0;
    int m = 1;
    while(eof < (int)data.length()) {
        eof = EOF_PADDING * m;
        if((int)data.length() <= eof) break;
        m++;
    }
    padded << data << pad_data(eof - (data.length() - 1));
    return padded.str();
}

string calc_checksum(string header) {
    int checksum = 0;
    for(int i = 0; i < (int)header.length(); i++) {
        checksum += (int)header.at(i);
    }
    return dec_to_padded_octal(checksum - 64, 6); // - 64, 6
}

string p_write_tar_entry(string tarname, string filename, int _size, int _modified, int etype) { 
    ostringstream contents;
    char nc = (char)0;
    string size = dec_to_padded_octal(_size, 11);
    string modified = dec_to_padded_octal(_modified, 0);
    string fm = "0100777";
    if(etype == 5) {
        fm = "0040777";
    }
    else {
        ifstream input(filename.c_str());
        contents << input.rdbuf();
        input.close();
    }
    /*
        * TAR FORMAT SPECIFICATION
        * (a) File name (0-)
        * (b) File mode (100; 8)
        * (c) Owner's numeric user ID (108; 8)
        * (d) Group's numeric user ID (116; 8)
        * (e) File size in bytes (octal) (124; 12)
        * (f) Last modification time in numeric Unix time format (octal) (136; 12)
        * (g) Checksum for header record (148; 8)
        * (h) Link indicator (file type) (156; 1)
        * (i) UStar indicator (257; 6)
    */
    ostringstream temp;
    ostringstream header;
    ostringstream data;
    temp << "_" << tarname << "_";
    ofstream tar;
    tar.open(temp.str().c_str(), ofstream::out | ofstream::binary);
    header << filename << pad_data(101 - filename.length()) // (a)
    << fm << nc << "0000000" << nc << "0000000" << nc << size << nc << modified << nc // (b, c, d, e, f)
    << "000000" << nc << " " << etype << pad_data(101) << "ustar" << nc << "00" << pad_data(248); // (g, h, i)
    tar << header.str();
    if(etype == 0) {
        data << write_padded_data(contents.str());
        tar << data.str();
    }
    tar.close();
    return header.str();
}

void p_write_tar_entries(string tarname, vector<TarEntry> entries) {
    vector<string> e;
    for(int i = 0; i < (int)entries.size(); i++) {
        string header = p_write_tar_entry(entries[i].get_part(), entries[i].get_file(),
        entries[i].get_size(), entries[i].get_modified(), entries[i].get_type());
        e.push_back(write_checksum(entries[i].get_part(), header));
    }
    finalize_tar(merge_entries(tarname, e), tarname);
}

string write_checksum(string tarname, string header) {
    ostringstream t1, t2, contents;
    t1 << "_" << tarname << "_";
    t2 << "__" << tarname << "__";
    ifstream input(t1.str().c_str());
    contents << input.rdbuf();
    input.close();
    ofstream tar;
    tar.open(t2.str().c_str(), ofstream::out | ofstream::binary);
    tar << contents.str();
    tar.seekp(148);
    tar << calc_checksum(header);
    tar.close();
    return t2.str();
}

string merge_entries(string tarname, vector<string> entries) {
    ostringstream contents, temp;
    for(int i = 0; i < (int)entries.size(); i++) {
        ifstream input(entries[i].c_str());
        contents << input.rdbuf();
        remove(entries[i].c_str());
    }
    temp << "___" << tarname << "___";
    ofstream tar;
    tar.open(temp.str().c_str(), ofstream::out | ofstream::binary);
    tar << contents.str();
    tar.close();
    return temp.str();
}

void finalize_tar(string temp, string tarname) {
    ostringstream contents;
    ifstream input(temp.c_str());
    contents << input.rdbuf();
    input.close();
    ofstream tar;
    tar.open(tarname.c_str(), ofstream::out | ofstream::binary);
    tar << contents.str() << pad_data((EOF_PADDING * 2) + 1);
    tar.close();
}

int write_tar_entry(string tarname, string filename, int size, int modified, int etype) {
    ostringstream temp1, temp2;
    temp1 << "_" << tarname << "_";
    temp2 << "__" << tarname << "__";
    string header = p_write_tar_entry(tarname, filename, size, modified, etype);
    finalize_tar(write_checksum(tarname, header), tarname);
    remove(temp1.str().c_str());
    remove(temp2.str().c_str());
    return 0;
}

int write_tar_entries(string tarname, string manifest) {
    ostringstream temp;
    temp << "___" << tarname << "___";
    vector<TarEntry> entries;
    string l;
    ifstream f(manifest.c_str());
    while(getline(f, l)) {
        vector<string> entry = split(l, ':');
        entries.push_back(
        TarEntry(entry[0], entry[1], stoi(entry[2]), stoi(entry[3]), stoi(entry[4])));
    }
    f.close();
    p_write_tar_entries(tarname, entries);
    remove(manifest.c_str());
    remove(temp.str().c_str());
    for(int i = 0; i < (int)entries.size(); i++) {
        ostringstream part1, part2;
        part1 << "_" << entries[i].get_part() << "_";
        remove(part1.str().c_str());
        part2 << "__" << entries[i].get_part() << "__";
        remove(part2.str().c_str());
    }
    return 0;
}
