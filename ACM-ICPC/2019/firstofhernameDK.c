
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

int main(int argc, const char* argv[])
{
   ifstream in;
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
      in.open(argv[1], ios::in);
      cout << i << " round of counting" << endl;
      Events[0] = counters[i];
      if ((retval = PAPI_start_counters(Events, NUM_EVENTS)) != PAPI_OK)
         ERROR_RETURN(retval);
      int N, K;
      while (in >> N >> K) {
        vector<int> suffix(N+1, -1), succ(N+1);
        string S(N+1, -1);
        for (int i = 1; i <= N; i++) {
          in >> S[i] >> succ[i];
          suffix[i] = S[i];
        }

        vector<int> succ2 = succ;
        for (int sz = 1; sz <= N; sz <<= 1) {
          vector<pair<pair<int, int>, int>> v(suffix.size());
          for (int i = N; i >= 0; i--) {
            v[i] = {{suffix[i], suffix[succ2[i]]}, i};
            succ2[i] = succ2[succ2[i]];
          }
          sort(v.begin(), v.end());
          for (int i = 0, cur = 0; i < v.size(); i++) {
            if (i > 0 && v[i-1].first != v[i].first) cur++;
            suffix[v[i].second] = cur;
          }
        }
        vector<pair<int, int>> order;
        for (int i = 0; i < suffix.size(); i++) order.emplace_back(suffix[i], i);
        sort(order.begin(), order.end());

        for (int i = 0; i < K; i++) {
          string s;
          in >> s;
          auto cmp = [&] (int p) {
            for (int i = 0; i < s.size(); i++, p = succ[p]) {
              if (s[i] != S[p]) return s[i] < S[p] ? -1 : 1;
            }
            return 0;
          };
          int lo, hi;
          for (lo = 0, hi = N+1; lo < hi; ) {
            int mid = (lo+hi)/2;
            if (cmp(order[mid].second) > 0) lo = mid+1; else hi = mid;
          }
          int start = lo;
          for (lo = 0, hi = N+1; lo < hi; ) {
            int mid = (lo+hi)/2;
            if (cmp(order[mid].second) >= 0) lo = mid+1; else hi = mid;
          }
          //cout << lo-start << endl;
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


