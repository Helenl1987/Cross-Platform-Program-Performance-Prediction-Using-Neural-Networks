
#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

#define NUM_EVENTS 1
#define THRESHOLD 10000
#define ERROR_RETURN(retval) { fprintf(stderr, "Error %d %s:line %d: \n", retval,__FILE__,__LINE__);  exit(retval); }


vector<string> FAIL(1, "fail");

inline char chxor(char ch1, char ch2) {
  return (ch1 == '#') ^ (ch2 == '#') ? '#' : '.';
}

string invert_line(string s) {
  string ret;
  for (int i = 0; i+2 < s.size(); i++) {
    ret += s[i];
    s[i+1] = chxor(s[i], s[i+1]);
    s[i+2] = chxor(s[i], s[i+2]);
  }
  if (s[s.size()-2] != '.' || s[s.size()-1] != '.') return "";
  return ret;
}

vector<string> invert(vector<string> g) {
  int sx, sy;
  for (sy = 0; sy < g.size(); sy++) {
    if (invert_line(g[sy]).size() == 0) break;
  }
  if (sy < g.size()) {
    for (sx = 0; sx < g[0].size(); sx++) {
      string s;
      for (int y = 0; y < g.size(); y++) s += g[y][sx];
      if (invert_line(s).size() == 0) break;
    }
    if (sx == g[0].size()) return FAIL;
    g[sy][sx] = chxor('#', g[sy][sx]);
  }

  vector<string> ret(g.size()-2);
  for (int y = 0; y+2 < g.size(); y++) {
    ret[y] = invert_line(g[y]);
    if (ret[y].size() == 0) return FAIL;
    for (int y2 = y+1; y2 <= y+2; y2++)
    for (int x = 0; x < ret[y].size(); x++) {
      g[y2][x  ] = chxor(g[y2][x  ], ret[y][x]);
      g[y2][x+1] = chxor(g[y2][x+1], ret[y][x]);
      g[y2][x+2] = chxor(g[y2][x+2], ret[y][x]);
    }
  }
  if (g[g.size()-1].find('#') != -1 || g[g.size()-2].find('#') != -1) {
    return FAIL;
  }

  int minx = 1000000, maxx = -1, miny = 1000000, maxy = -1;
  for (int y = 0; y < ret.size(); y++)
  for (int x = 0; x < ret[0].size(); x++) if (ret[y][x] == '#') {
    minx = min(minx, x);
    maxx = max(maxx, x);
    miny = min(miny, y);
    maxy = max(maxy, y);
  }
  vector<string> ret2(maxy-miny+1);
  for (int y = miny; y <= maxy; y++) {
    ret2[y-miny] = ret[y].substr(minx, maxx-minx+1);
  }
  return ret2;
}



int main(int argc, const char* argv[])
{
   ifstream in;
   ofstream out;
   out.open(argv[2], ios::out);
   /*Declaring and initializing the event set with the presets*/
   int counters[37] = {PAPI_TOT_INS, PAPI_TOT_CYC,\
                     PAPI_L2_ICH, PAPI_L2_ICA, PAPI_L2_ICM,\
                     PAPI_L2_TCH, PAPI_L2_TCA, PAPI_L2_TCM,\
                     PAPI_L2_DCH, PAPI_L2_DCA, PAPI_L2_DCM,\
                     PAPI_L1_ICR,\
                     PAPI_L1_ICA, PAPI_L1_ICM,\
                     PAPI_L1_TCH, PAPI_L1_TCA, PAPI_L1_TCM,\
                     PAPI_L1_DCH, PAPI_L1_DCA, PAPI_L1_DCM,\
                     PAPI_TLB_DM, PAPI_TLB_IM, PAPI_TLB_TL,\
                     PAPI_STL_ICY, PAPI_HW_INT,\
                     PAPI_BR_TKN, PAPI_BR_MSP, PAPI_BR_INS,\
                     PAPI_VEC_INS, PAPI_RES_STL,\
                     PAPI_FML_INS, PAPI_FAD_INS, PAPI_FDV_INS, PAPI_FSQ_INS, \
                     PAPI_FP_OPS, PAPI_SP_OPS, PAPI_DP_OPS};
   int Events[1] = {};
   /*The length of the events array should be no longer than the 
     value returned by PAPI_num_counters.*/ 
 
   /*declaring place holder for no of hardware counters */
   int num_hwcntrs = 0;
   int retval;
   char errstring[PAPI_MAX_STR_LEN];
   /*This is going to store our list of results*/
   long long values[NUM_EVENTS];

   if((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT )
   {
      fprintf(stderr, "Error: %d %s\n",retval, errstring);
      exit(1);
   }

   if ((num_hwcntrs = PAPI_num_counters()) < PAPI_OK)
   {
      printf("There are no counters available. \n");
      exit(1);
   }
    for(int i = 0; i < 37; i++){
      out << i << " round of counting" << endl;
      in.open(argv[1], ios::in);
      Events[0] = counters[i];
      if ((retval = PAPI_start_counters(Events, NUM_EVENTS)) != PAPI_OK)
         ERROR_RETURN(retval);



  int X, Y;
  while (in >> X >> Y) {
    vector<string> g(Y);
    for (int y = 0; y < Y; y++) in >> g[y];

    while (g.size() >= 3 && g[0].size() >= 3) {
      vector<string> g2 = invert(g);
      if (g2 == FAIL) break;
      g = g2;
    }

    for (int y = 0; y < g.size(); y++) cout << g[y] << endl;


       if ( (retval=PAPI_read_counters(values, NUM_EVENTS)) != PAPI_OK)
            ERROR_RETURN(retval);
      out << values[0] << endl;
  }

     
      /******************* PAPI_stop_counters **********************************/
      if ((retval=PAPI_stop_counters(values, NUM_EVENTS)) != PAPI_OK)
         ERROR_RETURN(retval); 
      in.close();
   }
   out.close();
   exit(0); 
}

