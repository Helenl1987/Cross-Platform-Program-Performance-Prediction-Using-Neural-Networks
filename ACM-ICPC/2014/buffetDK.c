#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>

#include <algorithm>
#include <cstdio>
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
      out << i << " round of counting" << endl;
      in.open(argv[1], ios::in);
      Events[0] = counters[i];
      if ((retval = PAPI_start_counters(Events, NUM_EVENTS)) != PAPI_OK)
         ERROR_RETURN(retval);
      
      int D, W;
      while (in >> D >> W) {
        vector<pair<int, pair<int, int> > > MD;
        vector<pair<int, double> > MC;
        for (int i = 0; i < D; i++) {
          char ch;
          int w, t, dt;
          in >> ch;
          if (ch == 'D') {
            in >> w >> t >> dt;
            MD.push_back(make_pair(w, make_pair(t, dt)));
          } else {
            in >> t >> dt;
            MC.push_back(make_pair(t, dt));
          }
        }

        vector<double> vc(W+1, -1e15);
        vc[0] = 0;
        double cw = 0.0, cv = 0.0;
        int cwi = 0;
        sort(MC.begin(), MC.end());
        while (cwi <= W && MC.size()) {
          while (MC.size() >= 2 && MC[MC.size()-2].first == MC[MC.size()-1].first) {
            MC[MC.size()-2].second = 1.0 / (1.0/MC[MC.size()-1].second +
                                            1.0/MC[MC.size()-2].second);
            MC.pop_back();
          }
          double curt = MC.back().first, curdt = MC.back().second;
          double nw = (curdt == 0.0) ? W+1 : cw + 1.0 / curdt;
          while (cwi <= W && cwi <= nw) {
            vc[cwi] = cv + (cwi-cw)*(curt + curt-(cwi-cw)*curdt)/2;
            cwi++;
          }
          cv += (nw-cw)*(curt + curt-(nw-cw)*curdt)/2;
          cw = nw;
          MC.back().first--;
        }

        vector<long long> vd(W+1, -1000000000000000LL);
        vd[0] = 0;
        random_shuffle(MD.begin(), MD.end());
        for (int i = 0; i < MD.size(); i++) {
          int w = MD[i].first, t = MD[i].second.first, dt = MD[i].second.second;
          for (int j = W-w; j >= 0; j--) {
            long long tt = t, ct = t;
            for (int k = j+w; k <= W; k += w) {
              if (vd[j] + tt <= vd[k]) break;
              vd[k] = vd[j] + tt;
              ct -= dt;
              tt += ct;
            }
          }
        }

        double ret = -1e15;
        for (int i = 0; i <= W; i++) ret = max(ret, vd[i] + vc[W-i]);
        if (ret < -5e14) {
          printf("impossible\n");
        } else {
          printf("%.9lf\n", ret);
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



