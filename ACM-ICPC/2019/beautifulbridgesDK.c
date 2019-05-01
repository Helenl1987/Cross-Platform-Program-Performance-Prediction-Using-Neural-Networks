#include <stdio.h>
#include <stdlib.h>
#include "papi.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

#define NUM_EVENTS 1
#define THRESHOLD 10000
#define ERROR_RETURN(retval) { fprintf(stderr, "Error %d %s:line %d: \n", retval,__FILE__,__LINE__);  exit(retval); }

int main(int argc, const char* argv[])
{
   /*Declaring and initializing the event set with the presets*/
   ifstream in;
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


   /***************************************************************************
   *  This part initializes the library and compares the version number of the*
   * header file, to the version of the library, if these don't match then it *
   * is likely that PAPI won't work correctly.If there is an error, retval    *
   * keeps track of the version number.                                       *
   ***************************************************************************/

   if((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT )
   {
      fprintf(stderr, "Error: %d %s\n",retval, errstring);
      exit(1);
   }


   /**************************************************************************
    * PAPI_num_counters returns the number of hardware counters the platform *
    * has or a negative number if there is an error                          *
    **************************************************************************/
   if ((num_hwcntrs = PAPI_num_counters()) < PAPI_OK)
   {
      printf("There are no counters available. \n");
      exit(1);
   }

   //printf("There are %d counters in this system\n",num_hwcntrs);
		
   /**************************************************************************
    * PAPI_start_counters initializes the PAPI library (if necessary) and    *
    * starts counting the events named in the events array. This function    *
    * implicitly stops and initializes any counters running as a result of   *
    * a previous call to PAPI_start_counters.                                *
    **************************************************************************/
    for(int i = 0; i < 37; i++){
      Events[0] = counters[i];
      cout << i << " round of counting" << endl;
      in.open(argv[1], ios::in);
      if ((retval = PAPI_start_counters(Events, NUM_EVENTS)) != PAPI_OK)
         ERROR_RETURN(retval);

      int64_t N, H, HC, DC;
      while (in >> N >> H >> HC >> DC) {
        vector<int64_t> X(N), Y(N);
        for (int i = 0; i < N; i++) in >> X[i] >> Y[i];
        vector<int64_t> cost(N, 1e18);
        cost[0] = HC*(H-Y[0]);
        for (int i = 0; i < N; i++) {
          int64_t mind = 0, maxd = 2*(H-Y[i]);
          for (int j = i+1; j < N; j++) {
            int64_t d = X[j]-X[i], h = H-Y[j];
            if (d > maxd) break;
            if (2*h <= d) mind = max(mind, 2*d+2*h-int64_t(sqrt(8*d*h)));
            maxd = min(maxd, 2*d+2*h+int64_t(sqrt(8*d*h)));
            if (mind > maxd) break;
            if (d >= mind && d <= maxd) {
              cost[j] = min(cost[j], cost[i] + HC*h + DC*d*d);
            }
          }
        }
        //if (cost[N-1] == 1e18) cout << "impossible" << endl; else cout << cost[N-1] << endl;
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

