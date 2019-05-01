#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>

#include <iostream>
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

  int T, R, S, N;
  while (in >> T >> R) {
    vector<int> tbp(T+1), ts(T+1), tip(T+1), rpc(R+1), tdone(T+1, -1);
    vector<vector<pair<char, int>>> ti(T+1);
    for (int i = 1; i <= T; i++) {
      in >> ts[i] >> tbp[i] >> N;
      ti[i].resize(N);
      for (int j = 0; j < N; j++) {
        in >> ti[i][j].first >> ti[i][j].second;
        if (ti[i][j].first == 'L') {
          rpc[ti[i][j].second] = max(rpc[ti[i][j].second], tbp[i]);
        }
      }
    }

    vector<int> held_by(R+1);
    for (int curtime = 0, ndone = 0; ndone < T;) {
      vector<int> curp = tbp;
      int bestp, besti, bestambig;
      bool change;
      do {
        change = false;
        bestp = 0;
        for (int i = 1; i <= T; i++)
        if (curtime >= ts[i] && tip[i] < ti[i].size()) {
          pair<char, int>& inst = ti[i][tip[i]];
          bool blocked = false;
          if (inst.first == 'L') {
            if (held_by[inst.second]) {
              blocked = true;
              if (curp[held_by[inst.second]] < curp[i]) {
                curp[held_by[inst.second]] = curp[i];
                change = true;
              }
            }
            for (int j = 1; j <= R; j++) {
              if (held_by[j] && held_by[j] != i && rpc[j] >= curp[i]) {
                blocked = true;
                if (curp[held_by[j]] < curp[i]) {
                  curp[held_by[j]] = curp[i];
                  change = true;
                }
              }
            }
          }
          if (!blocked && curp[i] > bestp) {
            bestp = curp[i];
            besti = i;
            bestambig = false;
          } else if (!blocked && curp[i] == bestp) {
            bestambig = true;
          }
        }
      } while (change);

      if (bestp == 0) {curtime++; continue;}
      if (bestambig) cout << "AMBIGUOUS!" << endl;

      pair<char, int>& inst = ti[besti][tip[besti]];
//cout << curtime << ' ' << bestp << ' ' << besti << ' ' << inst.first << inst.second << ' ' << held_by[1] << endl;
      if (inst.first == 'C' && inst.second > 0) {
        inst.second--;
        curtime++;
      } else {
        if (inst.first == 'L') {
          held_by[inst.second] = besti;
        } else if (inst.first == 'U') {
          held_by[inst.second] = 0;
        }
        if (++tip[besti] == ti[besti].size()) {
          tdone[besti] = curtime;
          ndone++;
        }
      }
    }

    for (int i = 1; i <= T; i++) {
      cout << tdone[i] << endl;
    }

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

