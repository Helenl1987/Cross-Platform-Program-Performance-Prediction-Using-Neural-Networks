#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>

#include <cmath>
#include <iostream>
#include <string>
using namespace std;

#define NUM_EVENTS 1
#define THRESHOLD 10000
#define ERROR_RETURN(retval) { fprintf(stderr, "Error %d %s:line %d: \n", retval,__FILE__,__LINE__);  exit(retval); }


long long findb(string s, long long Y) {
  long long lo = 10, hi = 1.1e18;
  while (lo < hi) {
    long long b = (hi+lo+1)/2, cur = 0;
    if (pow(b, s.size()-1)*(s[0]-'0') > Y+1e9) {hi = b-1; continue;}
    for (int i = 0; i < s.size(); i++) cur = (cur*b) + s[i]-'0';
    if (cur > Y) hi = b-1; else lo = b;
  }
  return lo;
}


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

  long long Y, b;
  string L;
  while (in >> Y >> L) {
    for (int lp = L.size()-1;;) {
      long long b1 = findb(L.substr(0, lp+1) + string(L.size()-lp-1, '9'), Y);
      long long b2 = findb(L.substr(0, lp+1) + string(L.size()-lp-1, '0'), Y);
//cout << L << ' ' << b1 << ' ' << b2 << endl;
      for (b = b2; b >= b1; b--) {
        long long pb = 1, y;
        for (int i = 0; i < L.size()-1; i++) pb *= b;
        for (y = Y; pb; pb /= b) {
          long long dig = y/pb;
          if (dig > 9) break;
          y -= dig*pb;
        }
        if (y == 0) goto done;
      }
      while (L[lp]++ == '9') {
        if (lp == 0) {
          L = '0' + L;
        } else {
          lp--;
        }
      }
    }

done:
    cout << b << endl;



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

