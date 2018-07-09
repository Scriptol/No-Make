/*
    Copyright 2001-2018 Denis Sureau
    Site: scriptol.com - .fr
    License: Apache 2.0.
*/  

#pragma once
using namespace std;

// The IncludeItem class is the image of a file of a project, it holds:
// - the name
// - the list of number of included file, dependencies
// - the number of dependencies
// - the date of the file to compare with php one
// - a flag to indicate if compiled

class IncludedItem
{
  public:

  string path;
  int filetime;
  int state;
  int dependency;  // number of included files
  bool isMain;

  vector<int, allocator<int> > numList;

  IncludedItem(const string &);    // constructor with a filename

  void add(int);       // adds a number
  void remove(int);
  void update(int);    // removes the number of a compiled file
};


/**
   The list of files in the project, represented by items
*/

class IncludedVector
{

 vector<IncludedItem *> prjList;  // list of all files

 public:

 IncludedVector();

 int find(const string &);      // returns number of a file in the list
 void incDep(int);              // increments dep for a file
 int getDep(int);               // get dep of a file

 int add(const string &);       // adds a file and return its index
 void addDep(int, int);         // adds a dep to a file
 void update(int);              // update dependency for a compiled item

 inline string getName(int no)   { return prjList[no]->path; }
 inline int isCompiled(int no)   { return prjList[no]->state == 1; }
 inline void setCompiled(int no) { prjList[no]->state = 1; }
 inline void setCompiled()       { prjList[prjList.size() -1]->state = 1; }
 inline unsigned int size()      { return prjList.size(); }

 int best();                    // returns best file to compile

 inline void setMain(int no)     { prjList[no]->isMain = true; }
 inline bool isMain(int no)      { return prjList[no]->isMain; }

};

int mystricmp(const char *, const char *);
