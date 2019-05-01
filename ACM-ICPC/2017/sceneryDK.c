
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



struct Partial {
  int t, curi, nphoto, dli;
  long long sumstarts;
  bool after_photo;

  // Between two states at the same time we always maximize # of photos taken,
  // and the times we started those photos.
  bool operator<(const Partial& p) const {
    if (t != p.t) return t > p.t;
    if (nphoto != p.nphoto) return nphoto < p.nphoto;
    return sumstarts < p.sumstarts;
  }
};


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



  int N, T;
  while (in >> N >> T) {
    vector<pair<int, int>> V(N);
    for (int i = 0; i < N; i++) in >> V[i].first >> V[i].second;
    sort(V.begin(), V.end());

    bool ret = false;
    priority_queue<Partial> q;
    vector<priority_queue<int>> deadlines(1);  // Maintain O(N) heaps.
    Partial start;
    start.after_photo = false;
    start.t = start.curi = start.nphoto = start.dli = start.sumstarts = 0;
    q.push(start);

    while (!q.empty()) {
      Partial p = q.top();
      while (!q.empty() && q.top().t == p.t) q.pop();
      if (p.nphoto == N) {
        ret = true;
        break;
      }

      if (p.after_photo) {
        deadlines[p.dli].pop();
      } else {
        deadlines.push_back(deadlines[p.dli]);
        p.dli = deadlines.size()-1;
      }
      priority_queue<int>& dl = deadlines[p.dli];
      for (; p.curi < V.size() && V[p.curi].first <= p.t; p.curi++) {
        dl.push(-V[p.curi].second);
      }
      if (dl.size() && -dl.top() < p.t + T) continue;

      if (p.curi < V.size() && (!dl.size() || V[p.curi].first < p.t+T)) {
        p.after_photo = false;
        int ot = p.t;
        p.t = V[p.curi].first;
        q.push(p);
        p.t = ot;
      }

      if (dl.size()) {
        p.after_photo = true;
        p.nphoto++;
        p.sumstarts += p.t;
        p.t += T;
        q.push(p);
      }
    }

    cout << (ret ? "yes" : "no") << endl;

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
