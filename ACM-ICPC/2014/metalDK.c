
#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>

#include <algorithm>
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
        vector<vector<int> > c(2*N, vector<int>(2*N));
        for (int i = 0; i < 2*N; i++) c[i][i] = 1000000001;
        vector<pair<int, pair<int, int> > > e;
        for (int i = 0; i < N; i++)
        for (int j = i+1; j < N; j++) {
          int d;
          in >> d;
          c[i][j+N] = max(c[i][j+N], d);
          c[j][i+N] = max(c[j][i+N], d);
          e.push_back(make_pair(d, make_pair(i, j)));
        }
        if (N <= 2) {cout << 0 << endl; continue;}

        for (int k = 0; k < 2*N; k++)
        for (int i = 0; i < 2*N; i++)
        for (int j = 0; j < 2*N; j++) {
          c[i][j] = max(c[i][j], min(c[i][k], c[k][j]));
        }

        sort(e.begin(), e.end());
        int ret = e.back().first;
        for (int i = e.size()-1; i >= 0; i--) {
          int mx = 0;
          for (int j = 0; j < N; j++) mx = max(mx, min(c[j][j+N], c[j+N][j]));
          ret = min(ret, mx + e[i].first);
          int a = e[i].second.first, b = e[i].second.second;
          if (c[a+N][b] < 1000000001)
          for (int j = 0; j < 2*N; j++)
          for (int k = 0; k < 2*N; k++) {
            c[j][k] = max(c[j][k], min(c[j][a+N], c[b][k]));
          }
          if (c[a][b+N] < 1000000001)
          for (int j = 0; j < 2*N; j++)
          for (int k = 0; k < 2*N; k++) {
            c[j][k] = max(c[j][k], min(c[j][a], c[b+N][k]));
          }
          if (c[b+N][a] < 1000000001)
          for (int j = 0; j < 2*N; j++)
          for (int k = 0; k < 2*N; k++) {
            c[j][k] = max(c[j][k], min(c[j][b+N], c[a][k]));
          }
          if (c[b][a+N] < 1000000001)
          for (int j = 0; j < 2*N; j++)
          for (int k = 0; k < 2*N; k++) {
            c[j][k] = max(c[j][k], min(c[j][b], c[a+N][k]));
          }
        }

        cout << ret << endl;
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


