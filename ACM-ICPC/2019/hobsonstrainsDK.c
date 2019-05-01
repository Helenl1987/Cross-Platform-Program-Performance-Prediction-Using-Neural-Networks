
#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>

#include <functional>
#include <iostream>
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
      Events[0] = counters[i];
      if ((retval = PAPI_start_counters(Events, NUM_EVENTS)) != PAPI_OK)
         ERROR_RETURN(retval);

      int N, K;
      while (in >> N >> K) {
        vector<int> succ(N), ret(N, -1);
        vector<vector<int>> pred(N);
        for (int i = 0; i < N; i++) {
          in >> succ[i];
          succ[i]--;
          pred[succ[i]].push_back(i);
        }
        for (int x = 0; x < N; x++) if (ret[x] == -1) {
          int c, c2;
          for (c = x, c2 = succ[x]; c != c2 && c != succ[c2]; c = succ[c], c2 = succ[succ[c2]])
            ;
          vector<int> cyc(1, c);
          for (int y = succ[c]; y != c; y = succ[y]) cyc.push_back(y);
          vector<int> diff(cyc.size());
          int base = min(K+1, int(cyc.size()));
          for (int i = 0; i < cyc.size(); i++) {
            for (auto y : pred[cyc[i]]) if (y != cyc[(i+cyc.size()-1)%cyc.size()]) {
              int ns = 0;
              vector<int> stack(1);
              function<void(int,int,int)> doit = [&] (int nd, int prev, int d) {
                int curns = ns;
                if (stack.size() == d) stack.push_back(0);
                ns++; stack[d]++;
                for (auto nd2 : pred[nd]) if (nd2 != prev) doit(nd2, nd, d+1);
                ret[nd] = ns-curns;
                if (stack.size() == d+K+1) { ns -= stack.back(); stack.pop_back(); }
              };
              doit(y, cyc[i], 1);
              for (int j = 1; j < stack.size(); j++) {
                if (K-j+1 >= cyc.size()) {
                  base += stack[j];
                } else {
                  diff[i] += stack[j];
                  diff[(i+K-j+1)%cyc.size()] -= stack[j];
                  if (i+K-j+1 >= cyc.size()) base += stack[j];
                }
              }
            }
          }
          for (int i = 0; i < cyc.size(); i++) {
            base += diff[i];
            ret[cyc[i]] = base;
          }
        }
        for (auto n : ret) //cout << n << endl;
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
