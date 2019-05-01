#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <queue>
#include <string>
#include <vector>
using namespace std;

int dx[4] = {1, 0, -1, 0}, dy[4] = {0, -1, 0, 1};
int mv[50][50][4];
int best[50][50];

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
      
      int Y, X;
  while (in >> Y >> X) {
    vector<string> g(Y);
    for (int i = 0; i < Y; i++) in >> g[i];
    string message;
    in >> message;
    message += '*';

    memset(mv, -1, sizeof(mv));
    for (int y = 0; y < Y; y++)
    for (int x = 0; x < X; x++)
    for (int d = 0; d < 4; d++) {
      int x2 = x, y2 = y;
      for (int n = 0; ; n++) {
        if (x2 < 0 || x2 >= X || y2 < 0 || y2 >= Y) break;
        if (g[y2][x2] != g[y][x]) {
          mv[y][x][d] = n;
          break;
        }
        x2 += dx[d]; y2 += dy[d];
      }
    }

    vector<pair<int, pair<short, short> > > cur;
    cur.push_back(make_pair(0, make_pair(0, 0)));
    for (int i = 0; i < message.size(); i++) {
      memset(best, 63, sizeof(best));
      vector<pair<short, short> > v;
      int dist = 0, curi = 0;
      sort(cur.begin(), cur.end());
      for (;;) {
        if (v.empty()) {
          if (curi == cur.size()) break;
          dist = cur[curi].first;
        }
        while (curi < cur.size() && cur[curi].first == dist) {
          v.push_back(cur[curi++].second);
        }
        vector<pair<short, short> > v2;
        for (int j = 0; j < v.size(); j++) {
          int x = v[j].first, y = v[j].second;
          if (best[y][x] <= dist) continue;
          best[y][x] = dist;
          for (int d = 0; d < 4; d++) if (mv[y][x][d] != -1) {
            int x2 = x + dx[d] * mv[y][x][d], y2 = y + dy[d] * mv[y][x][d];
            if (best[y2][x2] > dist+1) {
              v2.push_back(make_pair(x2, y2));
            }
          }
        }
        v.swap(v2);
        dist++;
      }

      cur.clear();
      for (int y = 0; y < Y; y++)
      for (int x = 0; x < X; x++) if (g[y][x] == message[i]) {
        cur.push_back(make_pair(best[y][x]+1, make_pair(x, y)));
      }
    }

    int ret = 1000000000;
    for (int i = 0; i < cur.size(); i++) ret = min(ret, cur[i].first);
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
