/*
  tarino: a tar implementation with Node.js.
  Copyright 2016 Sam Saint-Pettersen.

  Native add-on.

  Dual licensed under the GPL and MIT licenses;
  see GPL-LICENSE and MIT-LICENSE respectively.
*/

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include "dos2unix.h"
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
        return part;
    }

    string get_file() {
        return file;
    }

    int get_size() {
        return size;
    }

    int get_modified() {
        return modified;
    }

    int get_type() {
        return etype;
    }
};

string write_checksum(string, int, string);
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

string to_date_time(int timestamp) {
    time_t rawtime = timestamp;
    struct tm* timeinfo;
    char datetime[25];
    timeinfo = localtime(&rawtime);
    strftime(datetime, 25, "%a %b %d %Y %X", timeinfo);
    return string(datetime);
}

string pad_str(string data, int length) {
  string padding = "";
  for(int i = 0; i < length - (int)data.length(); i++) {
    padding.append(" ");
  }
  return padding.append(data);
}

bool file_exists(const char* filename) {
    ifstream f(filename);
    return f.good();
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

int get_padding(string data) {
    int padding = 0;
    int m = 1;
    while(padding < (int)data.length()) {
        padding = EOF_PADDING * m;
        if((int)data.length() <= padding) break;
        m++;
    }
    return padding + EOF_PADDING;
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

void patch_tar(string temp, int etype, string checksum) {
    if(etype == 5) {
        ostringstream fs;
        ostringstream patched;
        ifstream input(temp.c_str());
        patched << input.rdbuf();
        input.close();
        ofstream tar;
        tar.open(temp.c_str());
        tar << patched.str();
        tar.seekp(130);
        tar << "0";
        #ifdef __unix__
        tar.seekp(148);
        tar << dec_to_padded_octal(stoi(checksum, 0, 8) - 1, 6);
        #endif
        tar.close();
    }
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
        contents << dos2unix_str(filename);
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
        e.push_back(write_checksum(entries[i].get_part(), entries[i].get_type(), header));
    }
    finalize_tar(merge_entries(tarname, e), tarname);
}

string write_checksum(string tarname, int etype, string header) {
    string checksum = calc_checksum(header);
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
    tar << checksum;
    tar.close();
    patch_tar(t2.str(), etype, checksum);
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
    tar << contents.str() << pad_data((EOF_PADDING * 3) + 1);
    tar.close();
}

int write_tar_entry(string tarname, string filename, int size, int modified, int etype) {
    ostringstream temp1, temp2;
    temp1 << "_" << tarname << "_";
    temp2 << "__" << tarname << "__";
    string header = p_write_tar_entry(tarname, filename, size, modified, etype);
    finalize_tar(write_checksum(tarname, etype, header), tarname);
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

void extract_entry(string tarname, int i, int overwrite, int verbose, int extract) {
    ifstream tar;
    tar.open(tarname.c_str(), ios::binary);
    char* filename = new char[100];
    char* mode = new char[8];
    char* owner = new char[8];
    char* group = new char[8];
    char* size = new char[12];
    char* modified = new char[12];
    char* checksum = new char[8];
    char* type = new char[1];
    tar.seekg(i); tar.read(filename, 99); tar.seekg(i + 100);
    tar.read(mode, 8);tar.seekg(i + 108); tar.read(owner, 8);
    tar.seekg(i + 116);tar.read(group, 8); tar.seekg(i + 124);
    tar.read(size, 12); tar.seekg(i + 136); tar.read(modified, 12);
    tar.seekg(i + 148);tar.read(checksum, 8); tar.seekg(i + 156);
    tar.read(type, 2);
    char* contents = new char[atoi(size)];
    tar.seekg(i + 512); tar.read(contents, atoi(size));
    tar.close();

    if(verbose == 1 && extract == 1) {
        cout << filename << endl;
    }
    else {
        cout << atoi(mode) << "  " << pad_str(to_string(atoi(size)), 10) << "  ";
        cout << to_date_time(stol(string(modified), nullptr, 8)) << "  " << filename << endl;
    }

    if(string(type) == "5" && extract == 1) {
        string fn = string(filename);
        string command = "mkdir ";
        #ifdef __unix__
        command.append("-p ");
        #else
        std::replace(fn.begin(), fn.end(), '/', '\\');
        #endif
        command.append(fn);
        int ec = system(command.c_str());
        if(ec == 1) {
          cout << "tarino-native: Exit code from system call was 1" << endl;
        }
    }

    if(string(type) == "0" && extract == 1) {
        if(overwrite || !file_exists(filename)) {
            ofstream out;
            out.open(filename, ofstream::out | ofstream::binary);
            out << contents;
            out.close();
        }
    }
}

vector<int> get_entry_offsets(string tarname, int size) {
    vector<int> offsets;
    char* magic = new char[5];
    ifstream tar;
    tar.open(tarname.c_str(), ios::binary);
    for(int i = 257; i <= size; i++) {
        tar.seekg(i); tar.read(magic, 6);
        if(string(magic) == "ustar" || string(magic) == "ustar ") {
            offsets.push_back((i + 249) - 506);
        }
    }
    tar.close();
    return offsets;
}

int extract_tar_entries(string tarname, int size, int overwrite, int verbose) {
    vector<int> offsets = get_entry_offsets(tarname, size);
    if(verbose == 1) {
        cout << "tarino-native: Extracting " << offsets.size() << " entries from archive.";
        cout << endl << endl;
    }

    for(int i = 0; i < (int)offsets.size(); i++) {
        extract_entry(tarname, offsets[i], overwrite, verbose, 1);
    }
    return 0;
}

int list_tar_entries(string tarname, int size, int verbose) {
    vector<int> offsets = get_entry_offsets(tarname, size);
    if(verbose == 1) {
        cout << "tarino-native: Listing " << offsets.size() << " entries from archive.";
        cout << endl << endl;
    }

    for(int i = 0; i < (int)offsets.size(); i++) {
        extract_entry(tarname, offsets[i], 0, 1, 0);
    }
    return 0;
}
