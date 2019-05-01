#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>

#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

#define NUM_EVENTS 1
#define THRESHOLD 10000
#define ERROR_RETURN(retval) { fprintf(stderr, "Error %d %s:line %d: \n", retval,__FILE__,__LINE__);  exit(retval); }


bool slopecmp(const pair<int, pair<int, int> >& a,
              const pair<int, pair<int, int> >& b) {
  long long av = (long long)a.second.first * b.second.second;
  long long bv = (long long)b.second.first * a.second.second;
  if (av != bv) return av < bv;
  return a.first > b.first;
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

  int N;
  while (in >> N) {
    vector<int> x1(N), x2(N), y(N);
    for (int i = 0; i < N; i++) {
      in >> x1[i] >> x2[i] >> y[i];
      if (x1[i] > x2[i]) swap(x1[i], x2[i]);
    }
    int ret = 0;
    for (int i = 0; i < N; i++) {
      vector<pair<int, pair<int, int> > > v;
      int cur = 0;
      for (int j = 0; j < N; j++) {
        if (y[j] == y[i]) {
          if (x1[i] >= x1[j] && x1[i] <= x2[j]) {
            cur += x2[j]-x1[j];
          }
        } else if (y[j] < y[i]) {
          v.push_back(make_pair(x2[j]-x1[j], make_pair(x1[i]-x2[j], y[i]-y[j])));
          v.push_back(make_pair(x1[j]-x2[j], make_pair(x1[i]-x1[j], y[i]-y[j])));
        } else {
          v.push_back(make_pair(x2[j]-x1[j], make_pair(x1[j]-x1[i], y[j]-y[i])));
          v.push_back(make_pair(x1[j]-x2[j], make_pair(x2[j]-x1[i], y[j]-y[i])));
        }
      }
      sort(v.begin(), v.end(), slopecmp);
      ret = max(ret, cur);
      for (int j = 0; j < v.size(); j++) {
        cur += v[j].first;
        ret = max(ret, cur);
      }
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

