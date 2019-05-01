#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include "papi.h"

#include <algorithm>
#include <cstring>
#include <iostream>
using namespace std;

int g[301][301], mncol[301][301], szmx[301][301];

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
      int A, B, X, Y;
      while (in >> A >> B >> Y >> X) {
        for (int y = 0; y < Y; y++)
        for (int x = 0; x < X; x++)
          in >> g[y][x];

        memset(szmx, 0, sizeof(szmx));
        for (int y1 = 0; y1 < Y; y1++) {
          for (int x = 0; x < X; x++)
            mncol[y1][x] = g[y1][x];
          for (int y2 = y1+1; y2 < Y; y2++)
          for (int x = 0; x < X; x++)
            mncol[y2][x] = min(mncol[y2-1][x], g[y2][x]);

          for (int x1 = 0; x1 < X; x1++)
          for (int y2 = y1; y2 < Y; y2++) {
            int cur = 1000000001;
            int* mnc = &mncol[y2][x1];
            int* sm = &szmx[y2-y1+1][1];
            for (int x2 = x1; x2 < X; x2++) {
              cur = min(cur, *mnc);
              *sm = max(*sm, cur);
              mnc++; sm++;
            }
          }
        }

        long long ret = 0;
        for (int a = 1; a <= A; a++)
        for (int b = 1; b <= B; b++) {
          //long long depth = max(szmx[a][b], szmx[b][a]);
          long long depth = szmx[a][b];
          if (depth == 0) continue;
          ret = max(ret, a*b * (depth + (a*b*depth-1) / (X*Y-a*b)));
        }
        //couut << ret << endl;
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

