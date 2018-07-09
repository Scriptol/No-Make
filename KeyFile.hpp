#pragma once
using namespace std;


int EXISTS(const string &fname);
string trim(const string &s);
char *reass();
bool processEntry(const string &s);
void codePad(char *str);
int makeInt(char *str);
char *makeStr(int i);
void codeIt(char *src, char *tgt);
bool ini(const string &, const string &);