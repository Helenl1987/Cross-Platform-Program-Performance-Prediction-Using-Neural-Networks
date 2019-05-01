
#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>

#include <algorithm>
#include <iostream>
#include <set>
#include <vector>
using namespace std;

#define NUM_EVENTS 1
#define THRESHOLD 10000
#define ERROR_RETURN(retval) { fprintf(stderr, "Error %d %s:line %d: \n", retval,__FILE__,__LINE__);  exit(retval); }


typedef vector<int> VI;
typedef vector<VI> VVI;
// Returns -1 for unmatched items.
// Complexity: O(V*E)
VI BipartiteMatch(const VVI &mat, VI *back_match = NULL) {
  int max_item = -1;
  VI fmat(mat.size(), -1), seen(mat.size(), -1), prev(mat.size());
  for (int i = 0; i < mat.size(); i++) if (mat[i].size())
    max_item = max(max_item, *max_element(mat[i].begin(), mat[i].end()));
  VI bmat(max_item+1, -1);

  for (int i = 0; i < mat.size(); i++) {
    VI q(1, i);
    seen[i] = i; prev[i] = -1;
    int x, y;
    while (!q.empty()) {
      x = q.back(); q.pop_back();
      for (VI::const_iterator it = mat[x].begin(); it != mat[x].end(); ++it) {
        int bm = bmat[*it];
        if (bm == -1) {y = *it; goto found_match;}
        if (seen[bm] < i) {
          seen[bm] = i; prev[bm] = x;
          q.push_back(bm);
        }
      }
    }
    continue;
found_match:
    while (x != -1) {
      bmat[y] = x;
      swap(y, fmat[x]);
      x = prev[x];
    }
  }

  if (back_match) *back_match = bmat;
  return fmat;
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


  int X, Y;
  while (in >> Y >> X) {
    VVI g(Y, vector<int>(X));
    VI xh(X), yh(Y);
    long long ret = 0;
    for (int y = 0; y < Y; y++)
    for (int x = 0; x < X; x++) {
      in >> g[y][x];
      ret += max(0, g[y][x] - 1);
      xh[x] = max(xh[x], g[y][x]);
      yh[y] = max(yh[y], g[y][x]);
    }

    set<int> seen;
    for (int x = 0; x < X; x++) if (xh[x] > 1) {
      ret -= xh[x]-1;
      if (seen.count(xh[x])) continue;
      seen.insert(xh[x]);

      VVI mat;
      for (int x2 = 0; x2 < X; x2++) if (xh[x2] == xh[x]) {
        mat.push_back(vector<int>());
        for (int y2 = 0, yi = 0; y2 < Y; y2++) if (yh[y2] == xh[x]) {
          if (g[y2][x2]) mat.back().push_back(yi);
          yi++;
        }
      }
      VI v = BipartiteMatch(mat);
      for (int i = 0; i < v.size(); i++) if (v[i] != -1) ret += xh[x]-1;
    }
    for (int y = 0; y < Y; y++) ret -= max(0, yh[y]-1);

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
