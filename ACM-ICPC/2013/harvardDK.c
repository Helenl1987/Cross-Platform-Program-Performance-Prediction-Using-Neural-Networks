#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include "papi.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>
using namespace std;

int B, S;
vector<char> ttype;
vector<int> tval, tend;
int freev[14];
long long con[15][15], totv, cur, best;

void doit(int a, int b, long long mul, int& firstv, int& lastv) {
  if (a == b) return;
  firstv = lastv = -1;
  for (int i = a; i < b; i++) {
    if (ttype[i] == 'V') {
      totv += mul;
      if (!freev[tval[i]]) {
        if (firstv == -1) firstv = tval[i];
        if (lastv != -1) con[lastv][tval[i]] += mul;
        lastv = tval[i];
      }
    } else if (ttype[i] == 'R') {
      int fv, lv;
      doit(i+1, tend[i], mul*tval[i], fv, lv);
      if (fv != -1) {
        con[lv][fv] += mul*(tval[i]-1);
        if (firstv == -1) fv = tval[i];
        if (lastv != -1) con[lastv][fv] += mul;
        lastv = lv;
      }
      i = tend[i];
    }
  }
}

#include <cmath>
#include <cstdio>
#include <iostream>
using namespace std;

#define NUM_EVENTS 1
#define THRESHOLD 10000
#define ERROR_RETURN(retval) { fprintf(stderr, "Error %d %s:line %d: \n", retval,__FILE__,__LINE__);  exit(retval); }

int main(int argc, const char* argv[])
{
   /*Declaring and initializing the event set with the presets*/
   int counters[] = {PAPI_TOT_INS, PAPI_TOT_CYC,\
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
   int Events[1] = {0};
   ifstream in;
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
      Events[0] = counters[i];
      in.open(argv[1], ios::in);
      cout << i << " round of counting" << endl;
      if ( (retval = PAPI_start_counters(Events, NUM_EVENTS)) != PAPI_OK)
         ERROR_RETURN(retval);
      
      for(;;) {
        string line;
        if (!getline(in, line)) break;
        istringstream line1(line);
        if (!(line1 >> B >> S)) break;
        if (!getline(in, line)) break;
        istringstream line2(line);
        vector<int> rstack;
        char ch;
        ttype.clear(); tval.clear(); tend.clear();
        while (line2 >> ch) {
          ttype.push_back(ch);
          tval.push_back(0);
          tend.push_back(0);
          if (ch == 'E') {
            tend[rstack.back()] = tend.size()-1;
            rstack.pop_back();
          } else {
            line2 >> tval.back();
            if (ch == 'R') rstack.push_back(tend.size()-1);
            if (ch == 'V') tval.back()--;
          }
        }

        best = 1e18;
        for (int i = 0; i < 14; i++) freev[i] = (i >= 14-S);
        do {
          memset(con, 0, sizeof(con));
          totv = 0;
          int firstv, lastv;
          doit(0, ttype.size(), 1, firstv, lastv);

          cur = totv + (firstv != -1);
          for (int i = 0; i < 14; i++)
          for (int j = 0; j < 14; j++) if (i != j)
            cur += con[i][j];

          if (S == 1) {best = cur; continue;}
          vector<int> cliques;
          for (int i = 0; i < 14-S || (i%S); i++) cliques.push_back(i/S+1);
          do {
            bool ordered = true;
            for (int j = 0, cc = 0; j < cliques.size(); j++)
              if (cliques[j] > cc) {
                if (cliques[j] > cc+1) {ordered = false; break;}
                cc++;
              }
            if (!ordered) continue;

            long long saved = 0;
            for (int ii = 0, i = 0; ii < 14; ii++) if (!freev[ii]) {
              for (int ji = ii+1, j = i+1; ji < 14; ji++) if (!freev[ji]) {
                if (cliques[j] == cliques[i])
                  saved += con[ii][ji] + con[ji][ii];
                j++;
              }
              i++;
            }
            best = min(best, cur-saved);
          } while (next_permutation(cliques.begin(), cliques.end()));
        } while (next_permutation(freev, freev+14));

        //cout << best << endl;
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

