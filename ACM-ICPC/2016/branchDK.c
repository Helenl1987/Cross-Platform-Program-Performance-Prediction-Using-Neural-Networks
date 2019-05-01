#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>

#include <algorithm>
#include <iostream>
#include <queue>
#include <vector>
using namespace std;

#define NUM_EVENTS 1
#define THRESHOLD 10000
#define ERROR_RETURN(retval) { fprintf(stderr, "Error %d %s:line %d: \n", retval,__FILE__,__LINE__);  exit(retval); }

vector<int> dijkstra(const vector<vector<int>>& e,
                     const vector<vector<int>>& el,
                     int s) {
  vector<int> dist(e.size(), 1e9);
  priority_queue<pair<int, int>> q;
  q.push(make_pair(0, s));
  while (!q.empty()) {
    int d = -q.top().first, x = q.top().second;
    q.pop();
    if (d >= dist[x]) continue;
    dist[x] = d;
    for (int i = 0; i < e[x].size(); i++) {
      q.push(make_pair(-d-el[x][i], e[x][i]));
    }
  }
  return dist;
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


  int N, B, S, R;
  while (in >> N >> B >> S >> R) {
    vector<vector<int> > fe(N), fel(N), be(N), bel(N);
    for (int i = 0; i < R; i++) {
      int X, Y, L;
      in >> X >> Y >> L;
      X--; Y--;
      fe[X].push_back(Y);
      fel[X].push_back(L);
      be[Y].push_back(X);
      bel[Y].push_back(L);
    }

    vector<int> fdist = dijkstra(fe, fel, B);
    vector<int> bdist = dijkstra(be, bel, B);
    vector<int> dist(B);
    for (int i = 0; i < B; i++) dist[i] = fdist[i] + bdist[i];
    sort(dist.begin(), dist.end());
    vector<long long> cumdist(B+1);
    for (int i = 0; i < B; i++) cumdist[i+1] = cumdist[i] + dist[i];

    vector<long long> dyn(B+1);
    for (int i = 1; i <= B; i++) dyn[i] = (i-1)*cumdist[i];
    for (int s = 2; s <= S; s++) {
      vector<long long> dyn2 = dyn;
      for (int i = 1; i <= B; i++)
      for (int j = 1; j*s <= i; j++) {
        dyn2[i] = min(dyn2[i], dyn[i-j] + (j-1)*(cumdist[i]-cumdist[i-j]));
      }
      dyn.swap(dyn2);
    }

    cout << dyn[B] << endl;


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

