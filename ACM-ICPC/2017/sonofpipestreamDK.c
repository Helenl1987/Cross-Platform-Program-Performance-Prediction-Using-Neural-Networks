
#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <vector>
using namespace std;


#define NUM_EVENTS 1
#define THRESHOLD 10000
#define ERROR_RETURN(retval) { fprintf(stderr, "Error %d %s:line %d: \n", retval,__FILE__,__LINE__);  exit(retval); }



#define MAXN 200

int N, P, cap[MAXN+1][MAXN+1], cap2[MAXN+1][MAXN+1], cap3[MAXN+1][MAXN+1];
double V, A, B, wflow[MAXN+1][MAXN+1], fflow[MAXN+1][MAXN+1];

int seen[MAXN+1];
int flow_dfs(int c[MAXN+1][MAXN+1], int x, int e, int flow) {
  if (x == e) return flow;
  if (seen[x]) return 0;
  seen[x] = true;
  for (int y = 1; y <= N; y++) if (c[x][y]) {
    int f = flow_dfs(c, y, e, min(flow, c[x][y]));
    if (f) {
      c[x][y] -= f;
      c[y][x] += f;
      return f;
    }
  }
  return 0;
}

int flow(int c[MAXN+1][MAXN+1], int x, int e) {
  memset(seen, 0, sizeof(seen));
  return flow_dfs(c, x, e, 1000000000);
}

double flubber_dfs(int x, int e, double flubber) {
  seen[x] = true;
  if (x == e) return flubber;
  for (int y = 1; y <= N && flubber > 1e-7; y++) {
    if (!seen[y] && wflow[x][y] > 1e-7) {
      double f = flubber_dfs(y, e, min(wflow[x][y], flubber));
      if (f > 0) {
        wflow[x][y] -= f; wflow[y][x] += f;
        fflow[x][y] += f; fflow[y][x] -= f;
        return f;
      }
    }
  }
  return 0;
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


  while (in >> N >> P >> V >> A) {
    B = 1.0-A;
    memset(cap, 0, sizeof(cap));
    vector<int> X(P), Y(P), C(P);
    for (int i = 0; i < P; i++) {
      in >> X[i] >> Y[i] >> C[i];
      cap[X[i]][Y[i]] = cap[Y[i]][X[i]] = C[i];
    }

    memcpy(cap2, cap, sizeof(cap));
    memcpy(cap3, cap, sizeof(cap));
    int mnF = 0, mnW = 0, mxF = 0, mxW = 0, f;
    while ((f = flow(cap, 1, 3))) mxF += f;
    while ((f = flow(cap, 2, 3))) mnW += f;
    while ((f = flow(cap2, 2, 3))) mxW += f;
    while ((f = flow(cap2, 1, 3))) mnF += f;

    f = mxF + mnW;
    // Maximum of (x/V)^A * (f-x)^B lies at A/V * (f-x) - B * x/V = 0, or
    // F = Af / (A+B)
    double F = max((double)mnF, min((double)mxF, A*f / (double)(A+B)));
    double v1, v2;
    if (mnF == mxF) {
      v1 = v2 = 0.5;
    } else {
      v1 = (F-mnF)/(mxF-mnF);
      v2 = (mxF-F)/(mxF-mnF);
    }
    for (int x = 1; x <= N; x++)
    for (int y = 1; y <= N; y++) {
      wflow[x][y] = cap3[x][y] - (v1*cap[x][y] + v2*cap2[x][y]);
    }
    memset(fflow, 0, sizeof(fflow));
    for (double rem = F; rem > 1e-7; ) {
      memset(seen, 0, sizeof(seen));
      rem -= flubber_dfs(1, 3, rem);
    }

    for (int i = 0; i < P; i++) {
      printf("%.9lf ",  fflow[X[i]][Y[i]] / V);
      printf("%.9lf\n", wflow[X[i]][Y[i]]);
    }
    printf("%.9lf\n", pow(F/V, A) * pow(f-F, B));


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

