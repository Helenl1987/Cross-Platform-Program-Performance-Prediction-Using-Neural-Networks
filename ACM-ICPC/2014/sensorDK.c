#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>

#include <cstring>
#include <iostream>
#include <vector>
using namespace std;

int N, D;
int dist[101][101];
vector<int> v1, v2, v1seen, v2seen, v1forward, v2back, cur;

bool doit(int x) {
  if (v1seen[x]) return false;
  v1seen[x] = true;
  for (int y = 0; y < v2.size(); y++) if (dist[v1[x]][v2[y]] > D*D) {
    if (v2back[y] == -1 || doit(v2back[y])) {
      v1forward[x] = y;
      v2back[y] = x;
      return true;
    }
  }
  return false;
}

void mark(int y) {
  if (v2seen[y]) return;
  v2seen[y] = true;
  cur.push_back(v2[y]);
  int x = v2back[y];
  if (x == -1) return;
  v1seen[x] = true;
  for (int x2 = 0; x2 < v1.size(); x2++)
    if (dist[v1[x2]][v2[y]] > D*D) mark(v1forward[x2]);
}

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
      
      while (in >> N >> D) {
        vector<int> vx(N), vy(N);
        for (int i = 0; i < N; i++) in >> vx[i] >> vy[i];
        for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
          dist[i][j] = (vx[i]-vx[j])*(vx[i]-vx[j]) + (vy[i]-vy[j])*(vy[i]-vy[j]);

        vector<int> ret(1, 0);
        for (int i = 0; i < N; i++)
        for (int j = i+1; j < N; j++) if (dist[i][j] <= D*D) {
          v1.clear(); v2.clear();
          for (int k = 0; k < N; k++) {
            if (k != i && k != j && dist[i][k] <= dist[i][j]
                                && dist[j][k] <= dist[i][j]) {
              if ((vx[j]-vx[i])*(vy[k]-vy[i]) - (vy[j]-vy[i])*(vx[k]-vx[i]) <= 0) {
                v1.push_back(k);
              } else {
                v2.push_back(k);
              }
            }
          }

          v1forward = vector<int>(v1.size(), -1);
          v2back = vector<int>(v2.size(), -1);
          for (int k = 0; k < v1.size(); k++) {
            v1seen = vector<int>(v1.size(), 0);
            doit(k);
          }

          cur.clear();
          cur.push_back(i);
          cur.push_back(j);
          v1seen = vector<int>(v1.size(), 0);
          v2seen = vector<int>(v2.size(), 0);
          for (int x = 0; x < v1.size(); x++) if (!v1seen[x] && v1forward[x] != -1)
          for (int y = 0; y < v2.size(); y++)
          if (v2back[y] == -1 && dist[v1[x]][v2[y]] > D*D) {
            mark(v1forward[x]);
          }
          for (int x = 0; x < v1.size(); x++) if (!v1seen[x]) {
            cur.push_back(v1[x]);
            if (v1forward[x] != -1) v2seen[v1forward[x]] = true;
          }
          for (int y = 0; y < v2.size(); y++) if (!v2seen[y]) {
            cur.push_back(v2[y]);
          }

          if (cur.size() > ret.size()) ret = cur;
        }

        cout << ret.size() << endl;
        for (int i = 0; i < ret.size(); i++) {
          if (i) cout << ' ';
          cout << ret[i]+1;
        }
        cout << endl;
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



