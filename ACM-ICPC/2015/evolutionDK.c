// Note: this is the solution written for DNA (as the problem was
// called then) for ICPC 2013, slightly modified for the new format. /Per
#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

int N;
string dna[4001];
vector<int> ret1, ret2;
short pred1[4001][4001], pred2[4001][4001];

bool lencmp(const string& a, const string& b) {
  return a.size() > b.size();
}

char memo[4001][4001];
bool subseq(int ai, int bi) {
  char& ret = memo[ai][bi];
  if (ret != -1) return ret;
  const string& a = dna[ai];
  const string& b = dna[bi];
  if (a.size() >= b.size()) return ret = false;
  int i, j;
  for (i = 0, j = 0; i < a.size() && j < b.size(); j++) {
    if (a[i] == b[j]) i++;
  }
  return ret = (i == a.size());
}

void doit(vector<int>& r1, vector<int>& r2, int i1, int i2) {
  if (i1 == 0 && i2 == 0) return;
  int p1 = pred1[i1][i2], p2 = pred2[i1][i2];
  if (p2 == i1) {
    doit(r2, r1, p1, p2);
  } else {
    doit(r1, r2, p1, p2);
  }
  r2.push_back(i2);
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

      while (in >> N) {
    for (int i = 0; i <= N; i++) in >> dna[i];
    sort(dna+1, dna+N+1, lencmp);

    vector<pair<int, int> > q;
    q.push_back(make_pair(0, 0));
    memset(memo, -1, sizeof(memo));
    memset(pred1, -1, sizeof(pred1));
    memset(pred2, -1, sizeof(pred2));
    while (q.size() && q.back().second < N) {
      int i1 = q.back().first, i2 = q.back().second;
      q.pop_back();
      if (pred1[i2][i2+1] == -1 && subseq(i2+1, i1)) {
        pred1[i2][i2+1] = i1;
        pred2[i2][i2+1] = i2;
        q.push_back(make_pair(i2, i2+1));
      }
      if (pred1[i1][i2+1] == -1 && subseq(i2+1, i2)) {
        pred1[i1][i2+1] = i1;
        pred2[i1][i2+1] = i2;
        q.push_back(make_pair(i1, i2+1));
      }
    }

    if (!q.size()) {
      cout << "Impossible" << endl;
      if ( (retval=PAPI_read_counters(values, NUM_EVENTS)) != PAPI_OK)
            ERROR_RETURN(retval);
      out << values[0] << endl;
      continue;
    }

    ret1.clear(); ret2.clear();
    doit(ret1, ret2, q.back().first, q.back().second);
    cout << ret1.size() << ' ' << ret2.size() << endl;
	reverse(ret1.begin(), ret1.end());
	reverse(ret2.begin(), ret2.end());
    for (int i = 0; i < ret1.size(); i++) cout << dna[ret1[i]] << endl;
    for (int i = 0; i < ret2.size(); i++) cout << dna[ret2[i]] << endl;
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
