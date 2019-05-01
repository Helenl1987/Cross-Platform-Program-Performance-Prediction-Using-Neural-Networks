#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>

#include <iostream>
#include <string>
#include <vector>
using namespace std;

vector<int> h1, h2;
vector<string> history;
int lidx;
string s;
bool doit(int mv) {
  if (mv == 0) {
    int i;
    for (i = 0; i < s.size(); i++) if (s[i] != '.') break;
    for (; i < s.size(); i++) if (s[i] != 'A') break;
    for (; i < s.size(); i++) if (s[i] != 'B') break;
    for (; i < s.size(); i++) if (s[i] != '.') break;
    if (i == s.size()) {
      for (int j = 0; j < history.size(); j++) {
        cout << history[j] << "  " << h1[j] << " to " << h2[j] << endl;
      }
      cout << s << endl << endl;
      return true;
    }
    return false;
  }

  bool ret = false;
  history.push_back(s);
  for (int i = 0; i+1 < s.size(); i++) if (s[i] != '.' && s[i+1] != '.') {
    char c1 = s[i], c2 = s[i+1];
    s[i] = '.'; s[i+1] = '.';
    for (int j = 0; j+1 < s.size(); j++)
      if (j != i && s[j] == '.' && s[j+1] == '.') {
        s[j] = c1; s[j+1] = c2;
        h1.push_back(i+lidx); h2.push_back(j+lidx);
        ret |= doit(mv-1);
        h1.pop_back(); h2.pop_back();
        s[j] = '.'; s[j+1] = '.';
      }
    s[i] = c1; s[i+1] = c2;
  }
  history.pop_back();
  return ret;
}

void doit(int N, int add) {
  if (N == 4) {
    cout <<  6+add << " to " << -1+add << endl;
    cout <<  3+add << " to " <<  6+add << endl;
    cout <<  0+add << " to " <<  3+add << endl;
    cout <<  7+add << " to " <<  0+add << endl;
  } else if (N == 5) {
    cout <<  8+add << " to " << -1+add << endl;
    cout <<  3+add << " to " <<  8+add << endl;
    cout <<  6+add << " to " <<  3+add << endl;
    cout <<  0+add << " to " <<  6+add << endl;
    cout <<  9+add << " to " <<  0+add << endl;
  } else if (N == 6) {
    cout << 10+add << " to " << -1+add << endl;
    cout <<  7+add << " to " << 10+add << endl;
    cout <<  2+add << " to " <<  7+add << endl;
    cout <<  6+add << " to " <<  2+add << endl;
    cout <<  0+add << " to " <<  6+add << endl;
    cout << 11+add << " to " <<  0+add << endl;
  } else if (N == 7) {
    cout <<  8+add << " to " << -1+add << endl;
    cout <<  5+add << " to " <<  8+add << endl;
    cout << 12+add << " to " <<  5+add << endl;
    cout <<  3+add << " to " << 12+add << endl;
    cout <<  9+add << " to " <<  3+add << endl;
    cout <<  0+add << " to " <<  9+add << endl;
    cout << 13+add << " to " <<  0+add << endl;
  } else {
    cout << 2*N-2+add << " to " << -1+add << endl;
    cout << 3+add << " to " << 2*N-2+add << endl;
    doit(N-4, 4+add);
    cout << 0+add << " to " << 2*N-5+add << endl;
    cout << 2*N-1+add << " to " << 0+add << endl;
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
      out << i << " round of counting" << endl;
      in.open(argv[1], ios::in);
      Events[0] = counters[i];
      if ((retval = PAPI_start_counters(Events, NUM_EVENTS)) != PAPI_OK)
         ERROR_RETURN(retval);
      
      int N;
      while (in >> N) {
        if (N == 3) {
          cout << "2 to -1" << endl;
          cout << "5 to 2" << endl;
          cout << "3 to -3" << endl;
        } else {
          doit(N, 0);
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


