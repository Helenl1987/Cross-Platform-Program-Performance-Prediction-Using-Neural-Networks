#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <map>
#include <tuple>
#include <vector>
using namespace std;

long double Comb(int a, int b) {
  #define COMB__MAXA 2000
  #define COMB__MAXB 2000
  if (b > a || b < 0) return 0;
  if (!a) return 1;
  static long double combmemo[COMB__MAXA+1][COMB__MAXB+1];
  long double& ret = combmemo[a][b];
  if (!ret) ret = Comb(a-1, b-1)+Comb(a-1,b);
  return ret;
}

// Probability of first person getting g gems if d are distributed to n people.
inline long double finalProb(int n, int d, int g) {
  if (n == 1) return d == g;
  return Comb(n+d-g-2, d-g) / Comb(n+d-1, d);
}

map<int, pair<double, double>> m[1001][1001];

void add(int n, int d, int mx, double p, double e) {
  if (p < 1e-10) return;
  if (mx * n < d) return;
  if (mx > d) mx = d;
  if (mx < 0) mx = 0;
  pair<double, double>& ret = m[n][mx][d];
  ret.first += p; ret.second += e;
}

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
      
      int N, D, R;
  while(in >> N >> D >> R){
    add(N, D, D, 1.0, 0.0);
    for (int n = N; n >= 0; n--)
    for (int mx = D; mx >= 0; mx--)
    for (auto it : m[n][mx]) {
      int d = it.first;
      long double p = it.second.first, e = it.second.second;
      if (mx > 1 || mx == 1 && d == 0) add(n, d, mx-1, p, e);
      long double p2 = p;
      for (int rep = 1, n2 = n, d2 = d, eg = 0; n2 > 0 && d2 >= mx; rep++, n2--, d2 -= mx) {
        p2 *= finalProb(n2, d2, mx) * n2 / rep;
        if (N-n2 < R) eg += mx;
        if (mx > 0 || n2-1 == 0) add(n2-1, d2-mx, mx-1, p2, p2*(e/p+eg));
      }
    }
    long double tot = 0.0, ret = 0.0;
    for (auto it : m[0][0]) { tot += it.second.first; ret += it.second.second; }
    printf("%.7lf\n", (double)(ret/tot)+R);
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
