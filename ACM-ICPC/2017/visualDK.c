
#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>

#include <algorithm>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <vector>
using namespace std;



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
    vector<pair<pair<int, int>, int>> ul(N), br(N);
    for (int i = 0; i < N; i++) {
      in >> ul[i].first.first >> ul[i].first.second;
      ul[i].second = i+1;
    }
    for (int i = 0; i < N; i++) {
      in >> br[i].first.first >> br[i].first.second;
      br[i].second = i+1;
    }
    sort(ul.begin(), ul.end());
    sort(br.begin(), br.end());

    vector<int> ret(N+1);
    map<int, int> ulm;
    vector<pair<pair<int, int>, int>> rows;
    vector<pair<int, pair<int, int>>> cols;
    multiset<int> rowy;
    priority_queue<pair<int, int>> rowexpire;
    ulm[-2e9] = -1;
    for (int i = 0, j = 0; i < N || j < N; ) {
      if (j == N || i < N && ul[i].first.first <= br[j].first.first) {
        ulm[ul[i].first.second] = i;
        i++;
      } else {
        int x2 = br[j].first.first, y2 = br[j].first.second;
        auto it = --ulm.upper_bound(y2);
        int k = it->second, x = ul[k].first.first, y = ul[k].first.second;
        if (k == -1) goto fail;
        ulm.erase(it);
        ret[ul[k].second] = br[j].second;
        rows.push_back(make_pair(make_pair(x, x2), y));
        rows.push_back(make_pair(make_pair(x, x2), y2));
        cols.push_back(make_pair(x, make_pair(y, y2)));
        cols.push_back(make_pair(x2, make_pair(y, y2)));
        j++;
      }
    }

    sort(rows.begin(), rows.end());
    sort(cols.begin(), cols.end());
    for (int i = 0, j = 0; i < 2*N || j < 2*N; ) {
      if (j == 2*N || i < 2*N && rows[i].first.first <= cols[j].first) {
        rowy.insert(rows[i].second);
        rowexpire.push(make_pair(-rows[i].first.second, rows[i].second));
        i++;
      } else {
        while (!rowexpire.empty() && -rowexpire.top().first < cols[j].first) {
          rowy.erase(rowy.lower_bound(rowexpire.top().second));
          rowexpire.pop();
        }
        auto it = rowy.lower_bound(cols[j].second.first);
        auto it2 = rowy.upper_bound(cols[j].second.second);
        if ((++(++it)) != it2) goto fail;
        j++;
      }
    }

    for (int i = 1; i <= N; i++) cout << ret[i] << endl;
    if ( (retval=PAPI_read_counters(values, NUM_EVENTS)) != PAPI_OK)
            ERROR_RETURN(retval);
    out << values[0] << endl;

    continue;

fail:
    cout << "syntax error" << endl;


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


