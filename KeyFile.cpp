/*
  Copyright 2001-2018  Denis Sureau
  Site: Scriptol.com - fr
  License Apache 2.0
  Loads an ini file and processes the content. 
*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <map>
#include <vector>
#include <string>

#include <iterator>
#include <string.h>

#include "KeyFile.hpp"



int EXISTS(const string &fname) {
  FILE *fp;
  if( (fp = fopen(fname.c_str(), "r") ) == NULL) return 0;
  fclose(fp);
  return 1;
}


string trim(const string &s) {
  int d = 0;
  int l = s.length();
  while((d < l) && (s[d] < 33)) d++;
  while((l > d) && (s[l] < 33)) l--;
  return s.substr(d, l - d + 1);
}


bool processEntry(const string &s) {
    if(s[0] == '#') return true;
    int i = s.find('=');
    if (i == -1) return false;
    string k = trim(s.substr(0, i));
    string v = trim(s.substr(i + 1));
    return true;    
}

void codePad(char *str) {
  static char *mask = "HIRKOEPCLMELEJKXKAZIOQPSDMCEOMWXCVBHRIZKJSURJCNY";

  for(int i = 0; i < 48; i++)   {
    char c = str[i];
    if(c < '0') goto change;
    if(c > 'z') goto change;
    if(c <= '9') continue;
    if(c < 'A') goto change;
    if(c < 'Z') continue;
    if(c >= 'a') continue;
 change:
    str[i] = mask[i];
  }
}


void changeLetter(char *s, int a, int b) {
  char c = s[a];
  s[a] = s[b];
  s[b] = c;
}


void codeIt(char *src, char *tgt) {
  int i, l;
  char c, d;

  changeLetter(src, 0, 5);
  changeLetter(src, 3, 8);
  changeLetter(src, 10, 22);
  changeLetter(src, 13, 37);
  changeLetter(src, 17, 44);
  changeLetter(src, 21, 26);
  changeLetter(src, 23, 43);
  changeLetter(src, 31, 32);

  l = 0;
  for(i = 0; i < 48; i++)   {
    if(i % 2)  {
       d = src[i];
       c = src[i - 1] + 1;
       src[i - 1] = d;
    }
    c = toupper(src[i]);
    if(!isalnum(c)) c = '0';
    if(l == 7) {
      tgt[i] = '-';
      l = 0;
    }
    else  {
      tgt[i] = c;
    }

    l++;
  }
  tgt[47] = '\0';
}

/* Loading and processing the .ini file */

bool ini(const string &relpath, const string &iname) {
  FILE *fp;
  char *tst;
  char *buf = new char[65536];
  string str;

  string relini = relpath + iname;
    
  fp = fopen(relini.c_str(), "r");
  if(fp == NULL)  {
    fp = fopen(iname.c_str(), "r");
    if(fp == NULL) goto testing;
  }      

  while(1)  {
    tst = fgets(buf, 65536, fp);
    if(tst == NULL) break;
    str = buf;
    processEntry(str);
  }

  fclose(fp);

testing:
  return true;
}
