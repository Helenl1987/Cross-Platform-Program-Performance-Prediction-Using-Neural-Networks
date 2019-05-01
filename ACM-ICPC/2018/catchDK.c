#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <vector>
using namespace std;

struct Route {
  int B, E;
  long long S, T;
  double P, value;

  bool operator<(const Route& r) const {
    if (B != r.B) return B < r.B;
    return S < r.S;
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
      
      int M, N;
  long long K;

  while (in >> M >> N >> K) {
    vector<Route> v(M+1);
    for (int i = 0; i < M; i++) {
      in >> v[i].B >> v[i].E >> v[i].S >> v[i].T >> v[i].P;
      v[i].value = 0.0;
    }
    v[M].B = 1; v[M].S = K+1; v[M].value = 1.0;
    sort(v.begin(), v.end());

    vector<pair<long long, int>> ord(M+1);
    for (int i = 0; i <= M; i++) ord[i] = make_pair(-v[i].S, -i);
    sort(ord.begin(), ord.end());

    double ret = 0.0;
    for (int i = 0; i <= M; i++) if (-ord[i].first <= K) {
      int idx = -ord[i].second;
      Route& r = v[idx];
      r.value = 0.0;
      Route r2;

      r2.B = r.B;
      r2.S = r.S;
      auto it = upper_bound(v.begin(), v.end(), r2);
      if (it != v.end() && it->B == r.B) {
        r.value += (1.0-r.P) * it->value;  // Miss route.
      }

      r2.B = r.E;
      r2.S = r.T;
      it = upper_bound(v.begin(), v.end(), r2);
      if (it != v.end() && it->B == r.E) {
        r.value += r.P * it->value;  // Catch route.
      }

      if (idx < M && v[idx+1].B == v[idx].B) {
        r.value = max(r.value, v[idx+1].value);  // Don't try route at all.
      }

      if (r.B == 0) ret = max(ret, r.value);
    }

    printf("%0.8lf\n", ret);
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
