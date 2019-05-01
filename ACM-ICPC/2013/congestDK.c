#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include "papi.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>
using namespace std;

int N, M, C;
vector<vector<int> > con, con2, dist, eidx;
vector<int> mindist, seen, ecap, ecap2;

bool flow(int x) {
  if (x == 1) return true;
  if (seen[x]) return false;
  seen[x] = true;
  for (int i = 0; i < con[x].size(); i++)
    if (ecap[eidx[x][i]]) {
      if (flow(con[x][i])) {
        ecap[eidx[x][i]] = 0;
        ecap[eidx[x][i]^1] = 1;
        return true;
      }
    }
  return false;
}

#define NUM_EVENTS 1
#define THRESHOLD 10000
#define ERROR_RETURN(retval) { fprintf(stderr, "Error %d %s:line %d: \n", retval,__FILE__,__LINE__);  exit(retval); }

int main(int argc, const char* argv[])
{
   /*Declaring and initializing the event set with the presets*/
   int counters[] = {PAPI_TOT_INS, PAPI_TOT_CYC,\
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
   int Events[1] = {0};
   ifstream in;
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
      Events[0] = counters[i];
      in.open(argv[1], ios::in);
      cout << i << " round of counting" << endl;
      if ( (retval = PAPI_start_counters(Events, NUM_EVENTS)) != PAPI_OK)
         ERROR_RETURN(retval);

      int prob = 1;
      while (in >> N >> M >> C) {
        con = dist = vector<vector<int> >(N+1);
        for (int i = 0; i < M; i++) {
          int x, y, t;
          in >> x >> y >> t;
          con[x].push_back(y);
          dist[x].push_back(t);
          con[y].push_back(x);
          dist[y].push_back(t);
        }

        // Dijkstra
        mindist = vector<int>(N+1, 100000000);
        mindist[1] = 0;
        seen = vector<int>(N+1);
        for (int i = 0; i < N; i++) {
          int xd = 100000000, x = 0;
          for (int y = 1; y <= N; y++)
            if (!seen[y] && mindist[y] < xd) {
              xd = mindist[y];
              x = y;
            }
          assert(x > 0);
          seen[x] = true;
          for (int j = 0; j < con[x].size(); j++)
            mindist[con[x][j]] = min(mindist[con[x][j]], mindist[x] + dist[x][j]);
        }

        // Sort commuter distances.
        vector<pair<int, int> > comdist;
        for (int i = 0; i < C; i++) {
          int x;
          in >> x;
          comdist.push_back(make_pair(mindist[x], x));
        }
        sort(comdist.begin(), comdist.end());

        // Keep only useful edges.
        con2 = eidx = vector<vector<int> >(N+1);
        ecap.clear();
        for (int x = 1; x <= N; x++)
        for (int j = 0; j < con[x].size(); j++)
          if (mindist[x] == mindist[con[x][j]] + dist[x][j]) {
            con2[x].push_back(con[x][j]);
            eidx[x].push_back(ecap.size());
            ecap.push_back(1);
            con2[con[x][j]].push_back(x);
            eidx[con[x][j]].push_back(ecap.size());
            ecap.push_back(0);
          }
        con = con2;
        ecap2 = ecap;

        // Find flow for each commuter.
        int ret = 0;
        for (int i = 0; i < comdist.size(); i++) {
          if (i && comdist[i-1].first != comdist[i].first) {
            ecap = ecap2;  // Reset flow graph
          }
          seen = vector<int>(N+1);
          ret += flow(comdist[i].second);
        }

        //cout << "Case " << prob++ << ": " << ret << endl;
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


