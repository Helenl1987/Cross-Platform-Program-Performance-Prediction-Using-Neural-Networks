#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>

#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <vector>
using namespace std;

struct Tarp {
  int idx, X1, Y1, X2, Y2;
  bool operator<(const Tarp& t) const {
    if (X1 > t.X1) {
      return int64_t(Y1-t.Y1)*(t.X2-t.X1) < int64_t(X1-t.X1)*(t.Y2-t.Y1);
    } else {
      return int64_t(t.Y1-Y1)*(X2-X1) > int64_t(t.X1-X1)*(Y2-Y1);
    }
  }
};

struct Deltas {
  int base;
  map<int, int> d;
  map<int, int>::iterator mid;
  Deltas(int l, int r) : base(0) {
    d[-2e9] = d[l] = d[r] = d[2e9] = 1e8;
    mid = d.find(l);
  }

  int MidL() { return mid->first; }
  int MidR() { auto it = mid; return (++it)->first; }
  void Add(int x, int v, bool leftward) {
    if (v == 0) return;
    d[x] += v;
    if (leftward && x > MidL()) ++mid;
  }
  bool SubNearest(int x1, int x2, bool left) {
    auto it = left ? d.lower_bound(x1) : --d.upper_bound(x2);
    if (it->first < x1 || it->first > x2) return false;
    if (--it->second == 0) { if (it == mid) --mid; d.erase(it); }
    return true;
  }
  int Collect(int x1, int x2) {
    int tot = 0;
    for (auto it = d.lower_bound(x1); it->first <= x2; ) {
      tot += it->second;
      if (it == mid) --mid;
      d.erase(it++);
    }
    return tot;
  }
};

#define NUM_EVENTS 1
#define THRESHOLD 10000
#define ERROR_RETURN(retval) { fprintf(stderr, "Error %d %s:line %d: \n", retval,__FILE__,__LINE__);  exit(retval); }

int main(int argc, const char* argv[])
{
   ifstream in;
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


   /***************************************************************************
   *  This part initializes the library and compares the version number of the*
   * header file, to the version of the library, if these don't match then it *
   * is likely that PAPI won't work correctly.If there is an error, retval    *
   * keeps track of the version number.                                       *
   ***************************************************************************/

   if((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT )
   {
      fprintf(stderr, "Error: %d %s\n",retval, errstring);
      exit(1);
   }


   /**************************************************************************
    * PAPI_num_counters returns the number of hardware counters the platform *
    * has or a negative number if there is an error                          *
    **************************************************************************/
   if ((num_hwcntrs = PAPI_num_counters()) < PAPI_OK)
   {
      printf("There are no counters available. \n");
      exit(1);
   }

   //printf("There are %d counters in this system\n",num_hwcntrs);
		
   /**************************************************************************
    * PAPI_start_counters initializes the PAPI library (if necessary) and    *
    * starts counting the events named in the events array. This function    *
    * implicitly stops and initializes any counters running as a result of   *
    * a previous call to PAPI_start_counters.                                *
    **************************************************************************/
    for(int i = 0; i < 37; i++){
      in.open(argv[1], ios::in);
      Events[0] = counters[i];
      if ((retval = PAPI_start_counters(Events, NUM_EVENTS)) != PAPI_OK)
         ERROR_RETURN(retval);

      int L, R, N;
      while (in >> L >> R >> N) {
        vector<Tarp> T(N);
        for (int i = 0; i < N; i++) {
          in >> T[i].X1 >> T[i].Y1 >> T[i].X2 >> T[i].Y2;
          if (T[i].X1 > T[i].X2) { swap(T[i].X1, T[i].X2); swap(T[i].Y1, T[i].Y2); }
        }
        sort(T.begin(), T.end(), [] (const Tarp& a, const Tarp& b) { return a.X1 < b.X1; });
        for (int i = 0; i < N; i++) T[i].idx = i;

        vector<vector<int>> succ(N+1);
        set<Tarp> s;
        priority_queue<pair<int, int>> events;
        for (int i = 0; i < N; i++) {
          while (events.size() && -events.top().first < T[i].X1) {
            s.erase(T[events.top().second]);
            events.pop();
          }
          auto it = s.insert(T[i]).first;
          if (it == s.begin()) succ[N].push_back(i); else succ[(--it)->idx].push_back(i);
          events.push({-T[i].X2, i});
        }

        Deltas d(L, R);
        function<void(int)> doit = [&] (int n) {
          for (int i = succ[n].size()-1; i >= 0; i--) {
            const Tarp& t = T[succ[n][i]];
            if (t.Y1 < t.Y2) {
              if (t.X2 < d.MidR()) {
                if (d.SubNearest(t.X1, t.X2, true)) d.Add(t.X2, 1, true);
              } else if (t.X1 < d.MidL()) {
                d.SubNearest(t.X1, t.X2, true);
                d.base++;
                d.Add(t.X2, d.Collect(d.MidR(), t.X2)-1, false);
              } else {
                d.Add(t.X2, d.Collect(t.X1, t.X2), false);
              }
            } else {
              if (t.X1 > d.MidL()) {
                if (d.SubNearest(t.X1, t.X2, false)) d.Add(t.X1, 1, false);
              } else if (t.X2 > d.MidR()) {
                d.SubNearest(t.X1, t.X2, false);
                d.base++;
                d.Add(t.X1, d.Collect(t.X1, d.MidL())-1, true);
              } else {
                d.Add(t.X1, d.Collect(t.X1, t.X2), true);
              }
            }
            doit(t.idx);
          }
        };
        doit(N);

        auto it = d.mid;
        int ret = d.base;
        while (it->first >= R) { ret += it->second; --it; }
        ++it;
        while (it->first <= L) { ret += it->second; ++it; }
        //cout << ret << endl;
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

