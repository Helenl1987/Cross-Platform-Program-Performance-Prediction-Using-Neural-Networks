#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <tuple>
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
      
      int N, X, Y;
  while (in >> N >> X >> Y) {
    vector<tuple<int, int, int, long long>> v;
    long long shash = 0;
    for (int i = 0; i < N; i++) {
      int x1, y1, x2, y2;
      in >> x1 >> y1 >> x2 >> y2;
      int d1 = (x1 == 0) ? y1 : (y1 == Y) ? Y+x1 : (x1 == X) ? Y+X+Y-y1 : Y+X+Y+X-x1;
      int d2 = (x2 == 0) ? y2 : (y2 == Y) ? Y+x2 : (x2 == X) ? Y+X+Y-y2 : Y+X+Y+X-x2;
      long long hash = ((long long)rand()<<40)+((long long)rand()<<20)+rand();
      if (d2 < d1) shash += hash; else shash -= hash;
      v.emplace_back(d1, x1, y1, 2*hash);
      v.emplace_back(d2, x2, y2, -2*hash);
    }
    sort(v.begin(), v.end());

    map<long long, int> hashes;
    for (int i = 0; i < v.size(); i++) {
      shash += get<3>(v[i]);
      hashes[shash] = i;
    }
    for (int i = 0; i < v.size(); i++) {
      shash += get<3>(v[i]);
      if (hashes.count(-shash)) {
        int x1 = get<1>(v[i]), y1 = get<2>(v[i]);
        int x2 = get<1>(v[hashes[-shash]]), y2 = get<2>(v[hashes[-shash]]);
        printf("1\n");
        printf("%.1lf ", x1 + ((y1 == Y) ? 0.5 : (y1 == 0) ? -0.5 : 0));
        printf("%.1lf ", y1 + ((x1 == 0) ? 0.5 : (x1 == X) ? -0.5 : 0));
        printf("%.1lf ", x2 + ((y2 == Y) ? 0.5 : (y2 == 0) ? -0.5 : 0));
        printf("%.1lf\n", y2 + ((x2 == 0) ? 0.5 : (x2 == X) ? -0.5 : 0));
        goto done;
      }
    }
    printf("2\n");
    printf("%.1lf %.1lf %.1lf %.1lf\n", 0.5, 0.0, X-0.5, (double)Y);
    printf("%.1lf %.1lf %.1lf %.1lf\n", X-0.5, 0.0, 0.5, (double)Y);
done:{};
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


