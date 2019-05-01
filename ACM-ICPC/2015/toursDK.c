#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>

#include <iostream>
#include <vector>
using namespace std;

int Gcd(int a, int b) {return b ? Gcd(b, a%b) : a;}

vector<vector<int> > c;

int goal, cura, curb, eq;
vector<int> depth;
pair<int, bool> doit(int x, int prev, int d) {
  pair<int, bool> ret(d, x == goal);
  depth[x] = d;
  for (int i = 0; i < c[x].size(); i++) {
    int y = c[x][i];
    if (y == prev) continue;
    if (cura == x && curb == y || curb == x && cura == y) continue;
    if (depth[y] != -1) {
      ret.first = min(ret.first, depth[y]);
    } else {
      pair<int, bool> v = doit(y, x, d+1);
      ret.first = min(ret.first, v.first);
      if (v.second) ret.second = true;
      if (v.second && v.first == d+1) eq++;
//if (v.second && v.first == d+1) cout << ' ' << x << ',' << y;
    }
  }
  return ret;
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
      
      int n, m;
  while (in >> n >> m) {
    c = vector<vector<int> >(n+1);
    for (int i = 0; i < m; i++) {
      int a, b;
      in >> a >> b;
      c[a].push_back(b);
      c[b].push_back(a);
    }

    int ret = 0;
    for (int a = 1; a <= n; a++)
    for (int i = 0; i < c[a].size(); i++) if (c[a][i] > a) {
      depth = vector<int>(n+1, -1);
      cura = a; goal = curb = c[a][i]; eq = 1;
//cout << cura << ',' << curb << " -";
      if (doit(a, -1, 0).second) ret = Gcd(ret, eq);
//cout << endl;
    }

    cout << "1";
    for (int i = 2; i <= ret; i++) if (ret%i == 0) cout << ' ' << i;
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

