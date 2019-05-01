#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>

#include <cstdio>
#include <iostream>
#include <queue>
using namespace std;

struct Group {
  double sz, p, ev;
  bool operator<(const Group& g) const {return p > g.p;}
};

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
  double P1, P2, P3, P4;
  while (in >> N) {
    in >> P1 >> P2 >> P3 >> P4;
    priority_queue<Group> q;
    for (int n1 = 0; n1 <= N; n1++)
    for (int n2 = 0; n1+n2 <= N; n2++)
    for (int n3 = 0; n1+n2+n3 <= N; n3++) {
      int n4 = N-n1-n2-n3;
      Group g;
      g.sz = 1.0;
      for (int i = 1; i <= N; i++) g.sz *= i;
      for (int i = 1; i <= n1; i++) g.sz /= i;
      for (int i = 1; i <= n2; i++) g.sz /= i;
      for (int i = 1; i <= n3; i++) g.sz /= i;
      for (int i = 1; i <= n4; i++) g.sz /= i;
      g.p = 1.0;
      for (int i = 0; i < n1; i++) g.p *= P1;
      for (int i = 0; i < n2; i++) g.p *= P2;
      for (int i = 0; i < n3; i++) g.p *= P3;
      for (int i = 0; i < n4; i++) g.p *= P4;
      g.ev = 0.0;
      if (g.p > 1e-90) q.push(g);
    }
    while (q.size() > 1) {
      Group g = q.top(); q.pop();
      if (g.sz < 1e15) {
        long long sz = (long long)(g.sz + 0.5);
        if (sz % 2) {
          sz--;
          Group g2 = q.top(); q.pop();
          g2.sz -= 1;
          if (g2.sz > 0.5) q.push(g2);
          Group g3;
          g3.sz = 1;
          g3.p = g.p + g2.p;
          g3.ev = g.ev + g2.ev + g3.p;
          q.push(g3);
        }
        g.sz = sz;
      }
      if (g.sz >= 2) {
        Group g2;
        g2.sz = g.sz / 2;
        g2.p = g.p * 2;
        g2.ev = g.ev * 2 + g2.p;
        q.push(g2);
      }
    }
    printf("%.6lf\n", q.top().ev);
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
