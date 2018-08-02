/*
    Copyright 2001-2018 Denis Sureau
    Site: scriptol.com - .fr
    License: Apache 2.0.
*/    

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstring>
#include <iterator>
#include <vector>

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#ifdef UNIX
#include <unistd.h>
#include <libio.h>
#include <utime.h>
#else
#include <process.h>
#include <io.h>
#include <sys/utime.h>
#endif

#include "IncList.hpp"
#include "NoMake.hpp"
#include "KeyFile.hpp"

#define VERBOSE if(OPTION_VERBOSE) cout

IncludedVector project;

int OPTION_QUIET = 0;
int OPTION_DEBUG = 0;
int OPTION_VERBOSE = 0;
int COMPILE_ONLY = 0;     // compile only
int OPTION_ALL = 0;
int ALLCOMPILED = 0;
int errorCounter = 0;

bool isMainFile = true;

string CPPCOMMAND = "@cl /O2 /c /nologo /EHsc ";
string LINKER = "@link /nologo ";
string LIB_PATH;
string LIBS_TO_ADD = "";
string DEFAULT_SRC = ".cpp";
string DEFAULT_OBJ = ".obj";
string DEFAULT_H = ".hpp";
string initial;
string sourceName;
string nodeName;

int title_flag = 0;
void title() {
   if(title_flag) return;
   title_flag = 1;
   printf("\nNo Make V. 1.1 - Scriptol.com\n");
}

void usage() {
	title();
	puts("Copyright 2018 by D. G. Sureau");
	puts("Usage: nomake mainfile");
	puts("Options:");
	puts("  -a   compile all sources and build executable.");
	puts("  -c   compile only, changed file only.");
	puts("  -v   verbose.");
	puts("  -q   quiet.");
    puts("Default: Compile only if needed, and build executable.");
	exit(0);
}

void fatalError(string mess) {
    cout << "Error, " << mess << endl;
    exit(0);
}

void sourceExists() {
    if(!EXISTS(sourceName)) {
        fatalError(sourceName + " not found...");
    }    
}

// Add default extension if missing or define extension type

void setExtension() {
    int i = sourceName.rfind(".");
    if(i != -1 )  {
        nodeName = sourceName.substr(0, i);
        DEFAULT_SRC = sourceName.substr(i);
        if(DEFAULT_SRC == ".c") DEFAULT_H = ".h";
    }
    else {
        sourceName += DEFAULT_SRC;
    }

    sourceExists();
}

string changeExtension(string sourceName, string ext) {
    int i = sourceName.rfind(".");
    string node = "";
    if(i != -1 )  {
        node = sourceName.substr(0, i);
    }
    else {
        node = sourceName;
    }
    return node + ext;
}


/* Time functions */

time_t getFileTime(const string &fname) {
   FILE *fh;
   struct stat stat1;

   if ((fh = fopen(fname.c_str(), "r")) == NULL) return 0;
   fstat(fileno(fh), &stat1);
   fclose(fh);

   return stat1.st_mtime;
}

int compareTime(const string sourceName, const string objectName) {
   time_t t1;
   time_t t2;
   time_t tobj;   

    if(!EXISTS(objectName)) return 1;

   t1 = getFileTime(sourceName);
   if(t1 == 0) return 0;   

   string source = changeExtension(sourceName, DEFAULT_H);
   t2 = getFileTime(source);
   if(t2 == 0) return 0; 

   tobj = getFileTime(objectName);
   if(tobj == 0) return 0;

   return (unsigned long) t1 > (unsigned long) tobj || (unsigned long) t2 > (unsigned long) tobj;
}


bool setEntry() {
  if(entryName.compare("compiler") == 0)    { CPPCOMMAND = reass(); return true;}
  if(entryName.compare("linker") == 0)      { LINKER = reass(); return true; }
  if(entryName.compare("extension") == 0)   { DEFAULT_OBJ = reass(); return true; }
  if(entryName.compare("libs") == 0)        { LIBS_TO_ADD = reass(); return true; }
  return true;
}

/*
    Main function of the builder
*/

int main(int argc, char* argv[]) {
    string sourceDir;
   
    if (argc < 2) usage();

    project = IncludedVector();

    int counter = 0;
    ALLCOMPILED = 0;
    int l;
    const char *opt;
    char c;

    try {
        // Processing command line

        for(int o = 1; o < argc; o++) {
            if (argv[o][0] == '-') {
                opt = argv[o] + 1;
                l = strlen(opt);
                for(int x = 0; x < l; x++)  {
			        c = opt[x];
                    if(c == 'a') OPTION_ALL = 1;
                    else if(c == 'c') COMPILE_ONLY = 1;
                    else if(c == 'v') OPTION_VERBOSE = 1;
                    else if(c == 'q') OPTION_QUIET = 1;
                    else  {
                        cout << "Invalid option " << c << endl;
                        usage();
                    }
                }
            }
            else {
                if(sourceName == "")  {
                    sourceName = argv[o];
                }   
                else {
                    LIBS_TO_ADD += " " + string(argv[o]);
                }
            }
        }    

        if(sourceName == "") usage();
        title();

        int fpos = sourceName.find_last_of("/\\");
        if(fpos == string::npos)
            sourceDir = "./";
        else
            sourceDir = sourceName.substr(0, fpos + 1);

        initial = sourceName;
        setExtension();        

        ini(sourceDir, "nomake.ini");

        VERBOSE << "Starting." << endl;

        // Building the project
        
        extractFile(sourceDir, 1);   

        // Compiling the sources

        if(OPTION_VERBOSE) cout << "Building." << endl;
        if(OPTION_ALL || !ALLCOMPILED)  {
            if(!OPTION_QUIET) title();
            counter = buildAll();       
        }
    
        if(counter > 1 && !OPTION_QUIET) cout << counter << " files" << " processed.\n";
    } // try
    catch(exception& e)  {
        cout << "Exception in main: " << e.what() << endl;
        return 1;
    }
    return errorCounter;
}

/*
    Parsing the source code for included files
*/

string getIncluded(string str, const string sourceDir) {
    int l = 0;
    int size = str.size();
    if(size < 3) return "";
    // skip spaces, quotes
    str = str.substr(10);       // remove #include
    while((l < size) && (str[l] == ' ' || str[l] == '\"' || str[l] == '\''))  l++;

    int f = l;
    // go until quotes or space or end of line
    while((f < size) &&
        str[f] != '\n' &&
        str[f] != '\r' &&
        str[f] != '\"' &&
        str[f] != '\'' &&
        str[f] != ' '
    ) f++;

    str = str.substr(l, f - l);

    // test for extension
    int lastdot = str.rfind(".");
    if(lastdot == -1) {
        cout << "No extension, skipped" << endl;
        return "";
    }
    string ext = str.substr(lastdot, (size - lastdot) - 1);

    if(ext != ".hpp" && ext != ".h") {
        if(OPTION_VERBOSE) {
            cout << ext << " not C++ or C, skipped" << endl;
        }
        return "";
    }

    // replace header ext by source
    string newext = ".cpp";
    if(ext == ".h") newext = ".c";

    str = str.substr(0, lastdot) + newext;

    if((str[0] != '/') && (str[1] != ':')) {
        str = sourceDir + str;
    }

    if(!EXISTS(str)) {
        cout <<  str << " not found, ignored." << endl;
        return "";
    }
    return str;
}

// Extract all included files and add them to the tree.
// Each file is processed once.

char reader_buffer[65535];

int extractFile(const string sourceDir, int mainfile) {
    int current = 0;
    FILE *fp;
    char *tst;
    string str;
    int ld, lf;
    int INSIDE;
    int ins;
    int isCompiled;
    int isize;

    try {
        if(OPTION_DEBUG) cout << "Analyzing " << sourceName << endl;
        current = project.add(sourceName.c_str());
        if(mainfile) project.setMain(current);

        tst = "";
        INSIDE = 0;
        ins = 0;

        fp = fopen(sourceName.c_str(), "r");

        while(1) {
            tst = fgets(reader_buffer, 65535, fp);
            if(tst == NULL) break;
            str = reader_buffer;
            ld = 0;
            lf = str.size();
            while((lf > 0)  && (str[lf-1] < 33)) lf--;
            while((ld < lf) && (str[ld] < 33))   ld++;
            str.erase(lf);
            str = str.substr(ld);
            if(lf < 2) continue;    // skip too short line
            ins = 0;
            if(!INSIDE) {
                if(str[0] == '/') {
                    if(str[1] == '/') continue;
                    if(str[1] == '*') { INSIDE = 2; ins = 2; }
                }
                if(str[0] == '<') {
                    if(str[1] == '-') { INSIDE = 3; ins = 2; }
                }
                if(str.substr(0, 8) != "#include") 
                    continue;
            }
            if(INSIDE) {
                if(INSIDE == 2) {
                    ins = str.find("*/", ins);
                if(ins != -1) INSIDE = 0;
                }
                if(INSIDE == 0) {
                    ins = str.find("/*", ins + 2);
                    if(ins != -1) INSIDE = 1;
                }
                continue;
            }

            isCompiled = 0;
            isize = 7;

            ins = str.find("#include", ins);
            if(ins == -1) continue;

            ins = str.find("<");
            if(ins != -1) continue;           // include with < > are ignored
            if(OPTION_DEBUG) {
                cout << "Found header: " << str << endl;    
            } 
            string included = getIncluded(str, sourceDir);
            if(included.length() == 0) continue;
            ins = project.find(included);
            if (ins == -1) {
                sourceName = included;
                ins = extractFile(sourceDir, 0);
                project.addDep(current, ins);
                if(OPTION_VERBOSE) cout << "Found " << included << endl;
            }
            if(isCompiled) project.setCompiled();

        } // loop to read a line

        fclose(fp);
    }
    catch (exception& e) {
        cout << "Exception in source: " << e.what() << endl;
    }
    return current;
}

string plural(int x) {
    if(x < 2) return "";
    return "s";
}

// Build all files, starting with all referenced ones

int buildAll() {
    int l = (int) project.size();
    int counter = 0;
    LINKER += " /OUT:" + changeExtension(initial, ".exe");

    if(OPTION_VERBOSE) cout << "Building project: " << l << " file"<< plural(l) << endl;

    while(1) {
        int no = project.best();
        if (no == -1) break;
        sourceName = project.getName(no);
        isMainFile = project.isMain(no);
        project.setCompiled(no);
        project.update(no);
        string objectName = changeExtension(sourceName, ".obj");  

        // Compile only if object file older than source or header

        if(OPTION_ALL || compareTime(sourceName, objectName)) {
            string compiling = CPPCOMMAND + " " +sourceName + " /Fo" + objectName;
            if(OPTION_VERBOSE) {
                puts(compiling.c_str());
            }               
            system(compiling.c_str());
        }        
        
        if(!COMPILE_ONLY) {
            LINKER += " " + objectName;
        }
        counter++;
        if(counter > l) {
            fatalError("multiple main functions");
        }    
    }
    
    if(!COMPILE_ONLY) {
        LINKER += " " + LIBS_TO_ADD;  
        if(OPTION_VERBOSE) {
            puts(LINKER.c_str());
        }      
        system(LINKER.c_str());
    }
    return counter;
}

