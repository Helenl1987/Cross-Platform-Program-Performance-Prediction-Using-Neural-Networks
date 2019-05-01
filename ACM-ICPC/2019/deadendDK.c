#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>

#include <algorithm>
#include <functional>
#include <iostream>
#include <vector>
using namespace std;

typedef vector<int> VI;
typedef vector<VI> VVI;
typedef vector<pair<int, int> > VPII;

// Given an undirected graph, returns a list of all biconnected components.
// Note that articulation vertices will belong to multiple components.
// Complexity: O(E)
VVI BiconnectedComponents(const VVI& succ) {
  VVI ret;
  VI idx(succ.size(), -1), stack;
  function<int(int)> doit = [&] (int x) {
    idx[x] = stack.size();
    stack.push_back(x);
    int low = idx[x];
    if (succ[x].size() == 0) {
      ret.emplace_back(1, x);
    }
    for (int i = 0; i < succ[x].size(); i++) {
      if (idx[succ[x][i]] == -1) {
        int curs = stack.size();
        int v = doit(succ[x][i]);
        if (v >= idx[x]) {
          ret.emplace_back(stack.begin()+curs, stack.end());
          ret.back().push_back(x);
          stack.resize(curs);
        } else {
          low = min(low, v);
        }
      } else {
        low = min(low, idx[succ[x][i]]);
      }
    }
    return low;
  };
  for (int x = 0; x < succ.size(); x++) if (idx[x] == -1) doit(x);
  return ret;
}

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
      cout << i << " round of counting" << endl;
      Events[0] = counters[i];
      if ((retval = PAPI_start_counters(Events, NUM_EVENTS)) != PAPI_OK)
         ERROR_RETURN(retval);
      
      int N, M, V, W;
      while (in >> N >> M) {
        VVI g(N), gi(N), d(N);
        for (int i = 0; i < M; i++) {
          in >> V >> W;
          V--; W--;
          gi[V].push_back(g[W].size());
          gi[W].push_back(g[V].size());
          g[V].push_back(W);
          g[W].push_back(V);
          d[V].push_back(1);
          d[W].push_back(1);
        }

        VVI comps = BiconnectedComponents(g);

        VI seen(N);
        function<void(int,int)> clearD = [&] (int x, int i) {
          if (!d[x][i]) return;
          d[x][i] = 0;
          int y = g[x][i];
          if (++seen[y] > 3) return;
          for (int j = 0; j < g[y].size(); j++) if (g[y][j] != x) clearD(y, j);
        };

        for (auto const& comp : comps) if (comp.size() >= 3) {
          for (auto x : comp) for (int i = 0; i < g[x].size(); i++) clearD(x, i);
        }
        for (int x = 0; x < N; x++) for (int i = 0; i < g[x].size(); i++) {
          if (x < g[x][i]) swap(d[x][i], d[g[x][i]][gi[x][i]]);
        }

        seen = VI(N);
        for (int x = 0; x < N; x++) for (int i = 0; i < g[x].size(); i++) if (d[x][i]) {
          clearD(x, i);
          d[x][i] = 1;
        }

        VPII ret;
        for (int x = 0; x < N; x++) for (int i = 0; i < g[x].size(); i++) if (d[x][i]) {
          ret.emplace_back(x, g[x][i]);
        }
        sort(ret.begin(), ret.end());
        //cout << ret.size() << endl;
        for (auto const& p : ret) //cout << p.first+1 << ' ' << p.second+1 << endl;
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

