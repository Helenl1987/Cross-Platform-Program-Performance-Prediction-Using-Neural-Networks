#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include "papi.h"
#include <cstring>
#include <iostream>
#include <set>
using namespace std;

#define MAXN 1000

int N;
int doll[MAXN+1], dollv[2014], retcost[MAXN+1];
int cost[MAXN+1][MAXN+1], mx[MAXN+1][MAXN+1];

#define NUM_EVENTS 1
#define THRESHOLD 10000
#define ERROR_RETURN(retval) { fprintf(stderr, "Error %d %s:line %d: \n", retval,__FILE__,__LINE__);  exit(retval); }

int main(int argc, const char* argv[])
{
   /*Declaring and initializing the event set with the presets*/
   int counters[] = {PAPI_TOT_INS, PAPI_TOT_CYC,\
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
   int Events[1] = {0};
   ifstream in;
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
      Events[0] = counters[i];
      in.open(argv[1], ios::in);
      cout << i << " round of counting" << endl;
      if ( (retval = PAPI_start_counters(Events, NUM_EVENTS)) != PAPI_OK)
         ERROR_RETURN(retval);
      
      while (in >> N) {
        for (int i = 0; i < N; i++) in >> doll[i];
        memset(cost, 63, sizeof(cost));
        memset(mx, 0, sizeof(mx));
        for (int i = N-1; i >= 0; i--) {
          cost[i][i+1] = 0;
          memset(dollv, 0, sizeof(dollv));
          set<int> sorted;
          for (int j = i; j < N; j++) {
            if (dollv[doll[j]]) break;
            dollv[doll[j]] = true;
            mx[i][j+1] = max(mx[i][j], doll[j]);
            sorted.insert(doll[j]);

            set<int>::iterator smallest = sorted.end();
            --smallest;
            int seqsize1 = sorted.size(), seqsize2 = 0;
            for (int k = j+1; k < N; k++) {
              if (dollv[doll[k]]) break;
              while (seqsize1 && doll[k] < *smallest) {
                --seqsize1;
                if (smallest != sorted.begin()) --smallest;
              }
              if (!seqsize1 && doll[k] < *smallest) {
                ++seqsize2;
              }
              cost[i][k+1] = min(cost[i][k+1], cost[i][j+1] + cost[j+1][k+1] +
                  (k+1-i) - max(seqsize1, seqsize2));
            }
          }
        }

        memset(retcost, 63, sizeof(retcost));
        retcost[0] = 0;
        for (int i = 0; i < N; i++) if (retcost[i] < 1000000000) {
          for (int j = i; j < N; j++) if (mx[i][j+1] == j+1-i) {
            retcost[j+1] = min(retcost[j+1], retcost[i] + cost[i][j+1]);
          }
        }
        if (retcost[N] < 1000000000) {
          //cout << retcost[N] << endl;
        } else {
          //cout << "impossible" << endl;
        }

        if ( (retval=PAPI_read_counters(values, NUM_EVENTS)) != PAPI_OK)
            ERROR_RETURN(retval);
        cout << values[0] << endl;
      }
      
      /******************* PAPI_stop_counters **********************************/
      if ((retval=PAPI_stop_counters(values, NUM_EVENTS)) != PAPI_OK)
         ERROR_RETURN(retval);	
      in.close();
   }

   exit(0);	
}
