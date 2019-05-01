#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>

#include <iostream>
#include <vector>
using namespace std;

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

      int N, K;
  while (in >> N >> K) {
    int nv = 2*N+3;
    vector<vector<int> > cap(nv, vector<int>(nv));
    vector<vector<int> > cost(nv, vector<int>(nv));
    for (int i = 0; i <= N; i++) {
      cap[N+1+i][i] = 1;
      cost[N+1+i][i] = -10000000;
      cost[i][N+1+i] = 10000000;
      cap[i][nv-1] = N;
      for (int j = i+1; j <= N; j++) {
        cap[i][N+1+j] = 1;
        in >> cost[i][N+1+j];
        cost[N+1+j][i] = -cost[i][N+1+j];
      }
    }
    int ret = 10000000 * N;
    for (int f = 0; f < min(N, K); f++) {
      vector<int> best(nv, 1000000000), prev(nv, -1);
      best[0] = 0;
      vector<bool> changed(nv, true);
      for (int i = 0; i < nv; i++) {
        vector<bool> changed2(nv, false);
        for (int j = 0; j < nv; j++) if (changed[j])
        for (int k = 0; k < nv; k++) if (cap[j][k]) {
          if (best[k] > best[j] + cost[j][k]) {
            best[k] = best[j] + cost[j][k];
            prev[k] = j;
            changed2[k] = true;
          }
        }
        changed = changed2;
      }
      ret += best[nv-1];
      for (int x = nv-1; x != 0; x = prev[x]) {
        cap[prev[x]][x]--;
        cap[x][prev[x]]++;
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
