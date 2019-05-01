#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>

#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

int N, K;
vector<pair<int, int> > c;
int skip[1000001][20];

int dist(int s, int e) {
  if (s >= e) return 0;
  int i = lower_bound(c.begin(), c.end(),
      make_pair(s, 1000000000)) - c.begin() - 1;
  if (i < 0) return 1000000000;
  int ret = 1;
  for (int skp = 19; skp >= 0; skp--) {
    if (c[skip[i][skp]].second+1 < e) {
      ret += (1<<skp);
      i = skip[i][skp];
    }
  }
  if (c[i].second+1 < e) {
    ret++;
    i = skip[i][0];
  }
  if (c[i].second+1 < e) return 1000000000;
  return ret;
}

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
      
      while (in >> N >> K) {
        c.clear();
        vector<pair<int, int> > v;
        for (int i = 0; i < K; i++) {
          int a, b;
          in >> a >> b;
          if (a <= b) {
            c.push_back(make_pair(a, b));
          } else {
            v.push_back(make_pair(a, b));
            c.push_back(make_pair(1, b));
          }
        }

        c.push_back(make_pair(N+1, N+1));
        sort(c.begin(), c.end());
        {
          vector<pair<int, int> > c2;
          for (int i = 0, curmx = 0; i < c.size(); i++) {
            if (c[i].second <= curmx) continue;
            if (i+1 < c.size() && c[i+1].first == c[i].first) continue;
            c2.push_back(c[i]);
            curmx = c[i].second;
          }
          c = c2;
        }

        for (int i = 0; i < c.size(); i++) {
          skip[i][0] = lower_bound(c.begin(), c.end(),
              make_pair(c[i].second+1, 1000000000)) - c.begin() - 1;
        }
        for (int skp = 1; skp < 20; skp++)
        for (int i = 0; i < c.size(); i++) {
          skip[i][skp] = skip[skip[i][skp-1]][skp-1];
        }

    /*for (int i = 0; i < c.size(); i++) {
    cout << i << ' ' << c[i].first << '-' << c[i].second << ":";
    for (int j = 0; j < 20; j++) cout << ' ' << skip[i][j];
    cout << endl;
    }*/

        int ret = dist(1, N+1);
        for (int i = 0; i < v.size(); i++) {
          ret = min(ret, 1 + dist(v[i].second+1, v[i].first));
        }
        if (ret == 1000000000) {
          cout << "impossible" << endl;
        } else {
          cout << ret << endl;
        }
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

