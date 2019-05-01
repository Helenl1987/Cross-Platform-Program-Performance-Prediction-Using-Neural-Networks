#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>
#include <algorithm>
#include <cstdio>
#include <functional>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

struct RangeOp {
  vector<int> v;
  int s;

  RangeOp(int n) {
    for (s = 1; s < n; s <<= 1)
      ;
    v = vector<int>(s*2);
  }

  void set(int x, int val) {
    v[x += s] = val;
    while (x >>= 1) v[x] = v[2*x] + v[2*x+1];
  }

  int calc(int a, int b) {
    int ret = 0;
    for (a += s, b += s; a < b; a >>= 1, b >>= 1) {
      if (a&1) ret += v[a++];
      if (b&1) ret += v[--b];
    }
    return ret;
  }
};
#define NUM_EVENTS 1
#define THRESHOLD 10000
#define ERROR_RETURN(retval) { fprintf(stderr, "Error %d %s:line %d: \n", retval,__FILE__,__LINE__);  exit(retval); }

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
      
      ios::sync_with_stdio(false);
  int R, C;
  string line;
  while (getline(in, line)) {
    sscanf(line.c_str(), "%d %d", &R, &C);
    R = 2*R-1; C = 2*C-1;
    vector<string> v(R);
    for (int i = 0; i < R; i++) {
      getline(in, v[i]);
      v[i] += string(C-v[i].size(), ' ');
    }

    long long ret = 0;
    for (int swp = 0; swp < 2; swp++) {
      int sx = (swp == 0 || swp == 1 && R%4 == 1) ? 0 : 2;
      vector<vector<int>> ur(R, vector<int>(C)), ul(R, vector<int>(C));
      for (int x = sx; x < C+R; x += 4)
      for (int y = max(2, x-(C-3)); y < R && x-y >= 0; y += 2) {
        if (v[y-1][x-y+1] != ' ') ur[y][x-y] = ur[y-2][x-y+2]+1;
      }
      for (int x = sx-R/4*4; x < C; x += 4)
      for (int y = max(2, 4-x); y < R && x+y < C; y += 2) {
        if (v[y-1][x+y-1] != ' ') ul[y][x+y] = ul[y-2][x+y-2]+1;
      }
      vector<vector<int>> event(C);
      for (int y = 0; y < R; y += 2) {
        RangeOp rop(C/4+1);
        for (int x = sx^(y%4), lx = x; x < C; x += 4) {
          if (x && v[y][x-1] != ' ') {
            ret += rop.calc(max(lx, x-ul[y][x]*4)/4, x/4);
          } else {
            lx = x;
          }
          if (ur[y][x]) {
            if (x+4*ur[y][x] < C) event[x+4*ur[y][x]].push_back(x);
            rop.set(x/4, 1);
          }
          for (auto ex : event[x]) {
            rop.set(ex/4, 0);
          }
          event[x].clear();
        }
      }
      reverse(v.begin(), v.end());
    }
    cout << ret << endl;
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

