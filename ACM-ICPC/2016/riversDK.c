
#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>

#include <algorithm>
#include <iostream>
#include <queue>
#include <string>
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


  int N, M;
  while (in >> N >> M) {
    vector<string> rname(N);
    vector<vector<int>> ce(M+1);
    vector<long long> cmin(M+1, 1e18), rret(N);
    vector<int> rc(N), rd(N), cn(M+1), cc(M+1), cd(M+1);

    priority_queue<pair<long long, int>> event;
    for (int i = 0; i < N; i++) {
      in >> rname[i] >> rc[i] >> rd[i];
      ce[rc[i]].push_back(~i);
      cn[rc[i]]++;
      event.push(make_pair(-rd[i], rc[i]));
    }
    for (int i = 1; i <= M; i++) {
      in >> cc[i] >> cd[i];
      ce[cc[i]].push_back(i);
      cn[cc[i]]++;
    }

    vector<pair<long long, int>> rq, q;
    q.push_back(make_pair(0, 0));
    while (!q.empty()) {
      int x = q.back().second;
      long long d = q.back().first;
      q.pop_back();
      for (int i = 0; i < ce[x].size(); i++) {
        if (ce[x][i] < 0) {
          rq.push_back(make_pair(d+rd[~ce[x][i]], ~ce[x][i]));
        } else {
          q.push_back(make_pair(d+cd[ce[x][i]], ce[x][i]));
        }
      }
    }
    sort(rq.begin(), rq.end());

    int smaller = 0;
    for (int i = 0; i < N; i++) {
      while (event.size() && -event.top().first <= rq[i].first) {
        long long d = -event.top().first;
        int x = event.top().second;
        event.pop();
        cmin[x] = min(cmin[x], d);
        cn[x]--;
        if (cn[x] > 0 || x == 0) {
          smaller++;
        } else {
          event.push(make_pair(-cmin[x]-cd[x], cc[x]));
        }
      }
      rret[rq[i].second] = N-smaller+1;
    }

    for (int i = 0; i < N; i++) cout << rname[i] << ' ' << rret[i] << endl;

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

