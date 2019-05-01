#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>

#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
using namespace std;

string dir = "enws";
int dx[4] = {1, 0, -1, 0};
int dy[4] = {0, -1, 0, 1};

int R, C, D, E;
vector<string> G, P(27);
vector<vector<int>> next1(27), next2(27);

void donext(int p) {
  next1[p] = next2[p] = vector<int>(P[p].size(), -1);
  for (int i = 0; i < P[p].size(); i++) if (next1[p][i] == -1) {
    if (P[p][i] == 'u') {
      int j, par;
      for (j = i+3, par = 1; par; j++) par += (P[p][j]=='(')-(P[p][j]==')');
      next2[p][i] = i+3;
      next1[p][i] = j;
      next1[p][j-1] = i;
    } else if (P[p][i] == 'i') {
      int j, k, par;
      for (j = i+3, par = 1; par; j++) par += (P[p][j]=='(')-(P[p][j]==')');
      for (k = j+1, par = 1; par; k++) par += (P[p][k]=='(')-(P[p][k]==')');
      next1[p][i] = i+3;
      next2[p][i] = j;
      next1[p][j-1] = k;
    } else {
      next1[p][i] = i+1;
    }
  }
}

struct State {
  char p, x, y, d;
  int hash() const { return (int(p)<<24)+(int(x)<<16)+(int(y)<<8)+d; }
  bool operator<(const State& s) const { return hash() < s.hash(); }
  bool operator==(const State& s) const { return hash() == s.hash(); }
};
State Inf{-1,-1,-1,-1};

map<State, State> memo;
State doit(const State& os) {
  if (os.p != 26 && memo.count(os)) return memo[os];
  memo[os] = Inf;
  State s = os;
  vector<set<State>> seen(P[s.p].size());
  for (int i = 0; i < P[s.p].size(); ) {
    if (P[s.p][i] == 'm') {
      int x2 = s.x + dx[s.d], y2 = s.y + dy[s.d];
      if (G[y2][x2] != '#') { s.x = x2; s.y = y2; }
      i = next1[s.p][i];
    } else if (P[s.p][i] == 'l') {
      s.d = ((s.d+1)&3);
      i = next1[s.p][i];
    } else if (P[s.p][i] == 'u' || P[s.p][i] == 'i') {
      if (P[s.p][i] == 'u' && !seen[i].insert(s).second) return Inf;
      if (P[s.p][i+1] == 'b') {
        i = ((G[s.y+dy[s.d]][s.x+dx[s.d]] == '#') ? next1 : next2)[s.p][i];
      } else {
        i = ((P[s.p][i+1] == dir[s.d]) ? next1 : next2)[s.p][i];
      }
    } else if (P[s.p][i] >= 'A' && P[s.p][i] <= 'Z') {
      s.p = P[s.p][i]-'A';
      s = doit(s);
      if (s == Inf) return s;
      s.p = os.p;
      i = next1[s.p][i];
    } else {
      i = next1[s.p][i];
    }
  }
  if (s.p != 26) memo[os] = s;
  return s;
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
      
      while (in >> R >> C >> D >> E) {
        G = vector<string>(R+2, string(C+2, '#'));
        memo.clear();
        for (int y = 1; y <= R; y++) for (int x = 1; x <= C; x++) in >> G[y][x];
        for (int i = 0; i < D; i++) {
          string s;
          in >> s;
          P[s[0]-'A'] = s.substr(2);
          donext(s[0]-'A');
        }
        for (int i = 0; i < E; i++) {
          char ch;
          int x, y;
          in >> y >> x >> ch >> P[26];
          donext(26);
          State s = doit({26, char(x), char(y), char(dir.find(ch))});
          if (s == Inf) {
            //cout << "inf" << endl;
          } else {
            //cout << int(s.y) << ' ' << int(s.x) << ' ' << dir[s.d] << endl;
          }
        }
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


