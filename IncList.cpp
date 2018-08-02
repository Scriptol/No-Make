/*
    Copyright 2001-2018 Denis Sureau
    Site: scriptol.com - .fr
    License: Apache 2.0.

    Build the list of files to compile.
*/  

#pragma hdrstop

#include <stdlib.h>
#include <stdio.h>
#include <iterator>
#include <vector>
#include <string>
#include <iostream>

#include "IncList.hpp"

/* Case insensitive string compare */

int mystricmp(const char *pStr1, const char *pStr2) {
    char c1, c2;
    int  v;
    do {
        c1 = *pStr1++;
        c2 = *pStr2++;
        v = tolower(c1) - tolower(c2);
    } 
    while ((v == 0) && (c1 != '\0') && (c2 != '\0') );
    return v;
}

IncludedItem::IncludedItem(const string &s)  {
    filetime = 0;
    state = 0;
    dependency = 0;
    isMain = false;
    path = s;
    numList.clear();
  }

void IncludedItem::add(int no)  {
   numList.push_back(no);
   dependency++;
 }

void IncludedItem::remove(int idx) {
  numList[idx] = 0;
  dependency--;
}


void IncludedItem::update(int filenum)   {
    if(state) return;        // if compiled, ignore it
    int l = (int) numList.size();
    for(int i = 0; i < l; i++)  {
      if(filenum == numList[i]) { 
        remove(i); 
        return; 
      }
    }
    // no found, ignore
  }


// Include Vector

 IncludedVector::IncludedVector()  {
   prjList.clear();
   prjList.push_back(new IncludedItem(""));  // skip the 0 index
 }


int IncludedVector::find(const string & name) {
    int l = (int) prjList.size();

    for(int i = 1; i < l; i++)  {
        if (mystricmp(name.c_str(), prjList[i]->path.c_str()) == 0) return i;
        if(name.substr(0,2) == "./") {
            string relname = name.substr(2);
            if(mystricmp(relname.c_str(), prjList[i]->path.c_str()) == 0) return i;
        }
    }
   return -1;
 }

void IncludedVector::incDep(int no) {
    prjList[no]->dependency ++;
 }

int IncludedVector::getDep(int no) {
   return prjList[no]->dependency;
 }


int IncludedVector::add(const string &s) {
   int idx = find(s);
   if(idx != -1) return idx;
   prjList.push_back(new IncludedItem(s) );
   return (int) (prjList.size() - 1);
 }

// add an included file to the list of includes of a file (the last)

void IncludedVector::addDep(int idx, int filenum) {
  prjList[idx]->add(filenum);
}

int IncludedVector::best() {
   int l = (int) prjList.size();
   int m = 65535;
   int z;
   int no = -1;
   for(int i = 1; i < l; i++) {
      if(prjList[i]->state == 0) {
        z = prjList[i]->dependency;
        if(z == 0) return i;
        if (z < m) {
          m = z;
          no = i;
        }
      }
   }
   return no;  // should be -1 if all file compiled
 }

 // update dependency for a compiled item
void IncludedVector::update(int no) {
  int l = (int) prjList.size();
  for(int i = 0;i < l; i++)  {
    prjList[i]->update(no);
  }
}


/*
main()
{
  IncludedVector project = IncludedVector();

  string test = "test";
  project.add(test);
  if(project.find(test) != -1 ) cout << test << " found in project\n";
  else cout << test << " not found\n";
}
*/
