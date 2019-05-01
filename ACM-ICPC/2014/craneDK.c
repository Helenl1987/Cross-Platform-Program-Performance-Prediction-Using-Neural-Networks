#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>


#include <iostream>
#include <vector>
using namespace std;

#define EPS 1e-9

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
      
      int N;
      while (in >> N) {
        vector<int> vx(N), vy(N);
        int mnx = 1000000000, mxx = -1000000000;
        for (int i = 0; i < N; i++) {
          in >> vx[i] >> vy[i];
          if (vy[i] == 0) {mnx = min(mnx, vx[i]); mxx = max(mxx, vx[i]);}
        }
        vx.push_back(vx[0]); vy.push_back(vy[0]);

        double tcx = 0, tar = 0;
        for (int i = 0; i < N; i++) {
          double ar = (vx[i]*vy[i+1] - vy[i]*vx[i+1]) / 2.0;
          double cx = (vx[i] + vx[i+1]) / 3.0;
          double cy = (vy[i] + vy[i+1]) / 3.0;
          tcx += cx * ar;
          tar += ar;
        }
        if (tar < 0.0) {tar = -tar; tcx = -tcx;}
        double cx = tcx / tar;

        if (cx < mnx-EPS && vx[0] <= mnx || cx > mxx+EPS && vx[0] >= mxx) {
          cout << "unstable" << endl;
        } else {
          if (cx < mnx-EPS) {
            double a = tar * (mnx-cx) / (vx[0]-mnx);
            cout << (long long)(a+EPS) << " .. ";
          } else if (cx > mxx+EPS) {
            double a = tar * (cx-mxx) / (mxx-vx[0]);
            cout << (long long)(a+EPS) << " .. ";
          } else {
            cout << "0 .. ";
          }
          if (vx[0] >= mnx && vx[0] <= mxx) {
            cout << "inf" << endl;
          } else if (vx[0] > mxx) {
            double b = tar * (mxx-cx) / (vx[0]-mxx);
            cout << (long long)(b+1-EPS) << endl;
          } else {
            double b = tar * (cx-mnx) / (mnx-vx[0]);
            cout << (long long)(b+1-EPS) << endl;
          }
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


