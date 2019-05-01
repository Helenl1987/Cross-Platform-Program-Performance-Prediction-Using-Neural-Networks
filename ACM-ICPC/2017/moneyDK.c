
#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>

#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

#define NUM_EVENTS 1
#define THRESHOLD 10000
#define ERROR_RETURN(retval) { fprintf(stderr, "Error %d %s:line %d: \n", retval,__FILE__,__LINE__);  exit(retval); }


vector<pair<int, int>> normalize(vector<pair<int, int>> v) {
  vector<pair<int, int>> ret;
  sort(v.begin(), v.end());
  for (int i = 0; i < v.size(); i++) {
    while (ret.size() && ret.back().first <= v[i].first &&
                         ret.back().second <= v[i].second) {
      ret.pop_back();
    }
    ret.push_back(v[i]);
  }
  return ret;
}

vector<pair<int, int>> rev(vector<pair<int, int>> v) {
  reverse(v.begin(), v.end());
  for (int i = 0; i < v.size(); i++) v[i].first = -v[i].first;
  for (int i = 0; i < v.size(); i++) v[i].second = -v[i].second;
  return v;
}

long long doit(const vector<pair<int, int>>& a, int ai1, int ai2,
               const vector<pair<int, int>>& b, int bi1, int bi2,
               bool swapped) {
  if (ai2-ai1 <= 50 || bi2-bi1 <= 50) {
    long long ret = 0;
    for (int i = ai1; i < ai2; i++)
    for (int j = bi1; j < bi2; j++) {
      if (swapped) {
        ret = max(ret, (long long)min(0, b[j].first-a[i].first) *
                                  (b[j].second-a[i].second));
      } else {
        ret = max(ret, (long long)max(0, b[j].first-a[i].first) *
                                  (b[j].second-a[i].second));
      }
    }
    return ret;
  }

  vector<pair<int, int>> b1, b2;
  int i = (ai2+ai1)/2;
  for (int j = bi1; j+1 < bi2; j += 2) {
    long long v1 = (long long)(b[j].first-a[i].first) *
                              (b[j].second-a[i].second);
    long long v2 = (long long)(b[j+1].first-a[i].first) *
                              (b[j+1].second-a[i].second);
    if (v1 < v2) {
      b1.push_back(b[j]); b1.push_back(b[j+1]); b2.push_back(b[j+1]);
    } else {
      b1.push_back(b[j]); b2.push_back(b[j]); b2.push_back(b[j+1]);
    }
  }
  if ((bi2-bi1)%2) { b1.push_back(b[bi2-1]); b2.push_back(b[bi2-1]); }

  return max(doit(b1, 0, b1.size(), a, ai1, i, !swapped),
             doit(b2, 0, b2.size(), a, i, ai2, !swapped));
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


  int M, N;
  while (in >> M >> N) {
    vector<pair<int, int>> A(M), B(N);
    for (int i = 0; i < M; i++) in >> A[i].first >> A[i].second;
    for (int i = 0; i < N; i++) in >> B[i].first >> B[i].second;

    A = rev(normalize(rev(A)));
    B = normalize(B);

    cout << doit(A, 0, A.size(), B, 0, B.size(), false) << endl;



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

