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

vector<bool> comma, period;
vector<string> text;
map<string, vector<int>> wpos;

void donext(const string& w);
void doprev(const string& w) {
  static set<string> Done;
  if (Done.count(w)) return;
  Done.insert(w);
  for (auto i : wpos[w]) {
    if (i == 0 || period[i-1] || comma[i-1]) continue;
    comma[i-1] = true;
    donext(text[i-1]);
  }
}

void donext(const string& w) {
  static set<string> Done;
  if (Done.count(w)) return;
  Done.insert(w);
  for (auto i : wpos[w]) {
    if (period[i] || comma[i]) continue;
    comma[i] = true;
    if (i+1 < text.size()) doprev(text[i+1]);
  }
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
      string s;
  while (in >> s) {
    comma.push_back(s[s.size()-1] == ',');
    period.push_back(s[s.size()-1] == '.');
    if (comma.back() || period.back()) s = s.substr(0, s.size()-1);
    wpos[s].push_back(text.size());
    text.push_back(s);
  }
  for (int i = 0; i < text.size(); i++) if (comma[i]) {
    donext(text[i]);
    if (i+1 < text.size()) doprev(text[i+1]);
  }
  for (int i = 0; i < text.size(); i++) {
    if (i) cout << ' ';
    cout << text[i];
    if (comma[i]) cout << ',';
    if (period[i]) cout << '.';
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
