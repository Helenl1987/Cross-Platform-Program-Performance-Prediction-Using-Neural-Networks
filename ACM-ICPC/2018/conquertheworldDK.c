#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>
#include <cassert>
#include <cmath>
#include <iostream>
#include <set>
#include <vector>
using namespace std;

// Pair of sets that implement a heap in two (shiftable) parts, with
// the ability to add a constant to either part in constant time.
struct DoubleHeap {
  long long set1Base, set2Base;
  multiset<long long> set1, set2;

  // Make sure set 1 contains exactly "part" elements, padding with 0 if needed.
  void shiftPartition(int part) {
    while (set1.size() < part) {
      if (set2.size()) {
        set1.insert(*set2.rbegin() + set2Base - set1Base);
        set2.erase(--set2.end());
      } else {
        set1.insert(-set1Base);
      }
    }
    while (set1.size() > part) {
      set2.insert(*set1.begin() + set1Base - set2Base);
      set1.erase(set1.begin());
    }
  }
  int size() {
    return set1.size() + set2.size();
  }
  void push(long long v) {
    if (!set2.size() || v - set2Base > *set2.rbegin()) {
      set1.insert(v - set1Base);
    } else {
      set2.insert(v - set2Base);
    }
  }
  void pop() {
    if (set1.size()) {
      set1.erase(--set1.end());
    } else {
      set2.erase(--set2.end());
    }
  }
  long long top() {
    if (set1.size()) {
      return *set1.rbegin() + set1Base;
    }
    return *set2.rbegin() + set2Base;
  }
  void pruneNeg() {
    while (set2.size() && *set2.begin() + set2Base < 0) {
      set2.erase(set2.begin());
    }
    while (set1.size() && *set1.begin() + set1Base < 0) {
      set1.erase(set1.begin());
    }
  }
};

vector<vector<int>> e, ec;
vector<int> X, Y;

struct State {
  int minInc;
  long long baseCost;
  DoubleHeap* savings;

  State() : minInc(0), baseCost(0), savings(new DoubleHeap()) {}
  ~State() { delete savings; }
};

State doit(int nd, int prev) {
  State st;
  for (int i = 0; i < e[nd].size(); i++) if (e[nd][i] != prev) {
    auto st2 = doit(e[nd][i], nd);
    st.minInc += st2.minInc;
    st.baseCost += st2.baseCost + (long long)ec[nd][i] * abs(st2.minInc);
    st2.savings->set1Base += ec[nd][i];
    st2.savings->set2Base -= ec[nd][i];
    st2.savings->pruneNeg();
    if (st2.savings->size() > st.savings->size()) swap(st.savings, st2.savings);
    while (st2.savings->size()) {
      long long v = st2.savings->top();
      if (v <= 0) break;
      st2.savings->pop();
      st.savings->push(v);
    }
  }
  st.minInc += Y[nd] - X[nd];
  st.savings->shiftPartition(max(0, -st.minInc));
  //cout << nd << ' ' << st.minInc << ' ' << st.baseCost << ' ';
  //DoubleHeap q = *st.savings;
  //while (q.size()) { if (q.top()) cout << ' ' << q.top(); q.pop(); }
  //cout << endl;
  return st;
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
      
      int N;
  while (in >> N) {
    e = ec = vector<vector<int>>(N+1);
    for (int i = 0; i < N-1; i++) {
      int A, B, C;
      in >> A >> B >> C;
      e[A].push_back(B);
      e[B].push_back(A);
      ec[A].push_back(C);
      ec[B].push_back(C);
    }
    X = Y = vector<int>(N+1);
    for (int i = 1; i <= N; i++) in >> X[i] >> Y[i];

    auto st = doit(1, -1);
    long long ret = st.baseCost;
    for (int i = 0; i < -st.minInc; i++) {
      ret -= st.savings->top();
      st.savings->pop();
    }
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

