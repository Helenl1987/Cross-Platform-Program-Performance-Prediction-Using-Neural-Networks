#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>

#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

#define RND1 0x195a82bb394827f9LL
#define RND2 0x4829b9a829471ac3LL
#define RND3 0x1528de2948920182LL
#define RND4 0x11a39b8f30abbc51LL
#define RND5 0xa59b4110247e8ff3LL
#define RND6 0x498d73f8100294ebLL
#define DEPTH_MUL 5

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
        vector<vector<int> > c(N+1), cidx(N+1);
        for (int i = 1; i <= N; i++) {
          int K, x;
          in >> K;
          for (int j = 0; j < K; j++) {
            in >> x;
            c[i].push_back(x);
          }
        }
        for (int i = 1; i <= N; i++)
        for (int j = 0; j < c[i].size(); j++)
        for (int k = 0; k < c[c[i][j]].size(); k++) {
          if (c[c[i][j]][k] == i) cidx[i].push_back(k);
        }

        // Seed with a depth-0 hash value based on the corridor count.
        vector<vector<long long> > hash(N+1);
        for (int i = 1; i <= N; i++) {
          long long h = RND1;
          for (int j = 0; j <= c[i].size(); j++) h = (h * RND2) ^ RND3;
          for (int j = 0; j < c[i].size(); j++) hash[i].push_back(h);
        }

        // Compute higher-depth hashes up to a multiple of N.
        for (int depth = 0; depth < DEPTH_MUL * N; depth++) {
          vector<vector<long long> > newhash(N+1);
          for (int i = 1; i <= N; i++) if (c[i].size()) {
            // Use a rolling hash-of-hashes.
            long long pow = RND4, curhh = 0;
            for (int j = c[i].size()-1; j >= 0; j--) {
              curhh += pow * (hash[c[i][j]][cidx[i][j]]%RND6);
              pow *= RND5;
            }
            int baseidx = 0, basehh = curhh;
            for (int j = 0; j < c[i].size(); j++) {
              curhh = curhh * RND5 + (RND4-pow) * (hash[c[i][j]][cidx[i][j]]%RND6);
              newhash[i].push_back(curhh);
            }
          }
          hash.swap(newhash);
        }

        // Sort the hashes for each room and look for matches.
        for (int i = 1; i <= N; i++) {
          sort(hash[i].begin(), hash[i].end());
          if (hash[i].size() == 0) hash[i].push_back(RND1);
        }
        vector<int> seen(N+1);
        bool sets = false;
        for (int i = 1; i <= N; i++) if (!seen[i]) {
          bool foundset = false;
          for (int j = i+1; j <= N; j++) if (!seen[j]) {
            int x = 0, y = 0;
            while (x < hash[i].size() && y < hash[j].size()) {
              if (hash[i][x] == hash[j][y]) {
                if (!foundset) cout << i;
                foundset = sets = true;
                seen[j] = true;
                cout << ' ' << j;
                break;
              }
              if (hash[i][x] < hash[j][y]) x++; else y++;
            }
          }
          if (foundset) cout << endl;
        }
        if (!sets) cout << "none" << endl;
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
