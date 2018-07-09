/*
    Copyright 2001-2018 Denis Sureau
    Site: scriptol.com - .fr
    License: Apache 2.0.
*/  

#pragma once
using namespace std;


#define _DEBUG

#define COMPILE 0
#define WEB 1
#define SCRIPT 0
#define PHP4 2
#define PHP5 3
#define APP 4
#define BUILD 5
#define RUN 16

extern char * COMP_CEXT;
extern string LIB_PATH;

extern int OPTION_DEBUG;
extern int USE_FILENAME;

extern string initial;
extern string sourceName;
extern string jsName;
extern string nodeName;
extern string nodeOnly;
extern string NEWSTRING;

extern int staticMethod;
extern string intervalLow;
extern string intervalUp;
extern int intervalType;
extern int isIncluded;
extern int compiler;
extern string naturalSuffix;

extern int line;
extern int returnIndex;
extern int returnExpected;
extern bool xsol;
extern bool isMainFile;

extern int getLine();
string intToString(int);
void generateUp();      // for interval
string getGenerated();
void generateDown();
void makeObject(const string &);
void addCpp(const string &);

void parseFile();
int extractFile(const string, int);
int buildAll();

    
