#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>

#include <iostream>
#include <string>
#include <vector>
using namespace std;

#define NUM_EVENTS 1
#define THRESHOLD 10000
#define ERROR_RETURN(retval) { fprintf(stderr, "Error %d %s:line %d: \n", retval,__FILE__,__LINE__);  exit(retval); }


typedef vector<string> VS;

char digit[7][10][5] = {
{".XX.","....",".XX.",".XX.","....",".XX.",".XX.",".XX.",".XX.",".XX."},
{"X..X","...X","...X","...X","X..X","X...","X...","...X","X..X","X..X"},
{"X..X","...X","...X","...X","X..X","X...","X...","...X","X..X","X..X"},
{"....","....",".XX.",".XX.",".XX.",".XX.",".XX.","....",".XX.",".XX."},
{"X..X","...X","X...","...X","...X","...X","X..X","...X","X..X","...X"},
{"X..X","...X","X...","...X","...X","...X","X..X","...X","X..X","...X"},
{".XX.","....",".XX.",".XX.","....",".XX.",".XX.","....",".XX.",".XX."}
};

char display[7][22] = {
".??...??.....??...??.",
"?..?.?..?...?..?.?..?",
"?..?.?..?.?.?..?.?..?",
".??...??.....??...??.",
"?..?.?..?.?.?..?.?..?",
"?..?.?..?...?..?.?..?",
".??...??.....??...??."
};

void WriteDigit(int x, int dig, VS* display) {
  for (int y = 0; y < 7; y++)
  for (int i = 0; i < 4; i++)
    (*display)[y][x+i] = digit[y][dig][i];
}

VS GetDisplay(int time) {
  VS ret(7, string(21, '.'));
  ret[2][10] = ret[4][10] = 'X';
  if (time >= 10*60) WriteDigit(0, time/10/60, &ret);
  WriteDigit(5, (time/60)%10, &ret);
  WriteDigit(12, ((time%60)/10)%10, &ret);
  WriteDigit(17, time%10, &ret);
  return ret;
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
  int N;
  while (in >> N && N) {
    vector<VS> d(N, VS(7));
    for (int i = 0; i < N; i++)
    for (int y = 0; y < 7; y++)
      in >> d[i][y];

    VS ret(7);
    for (int y = 0; y < 7; y++) ret[y] = display[y];

    for (int i = 1; i < N; i++)
    for (int y = 0; y < 7; y++)
    for (int x = 0; x < 21; x++)
      if (d[i-1][y][x] != d[i][y][x])
        ret[y][x] = 'W';
    for (int y = 0; y < 7; y++)
    for (int x = 0; x < 21; x++)
      if (ret[y][x] == '?')
        ret[y][x] = (d[0][y][x] == 'X') ? '1' : '0';

    bool valid = false;
    for (int stime = 0; stime < 24*60; stime++) {
      vector<VS> displays(N);
      for (int i = 0; i < N; i++) {
        displays[i] = GetDisplay((stime + i) % (24*60));
        for (int y = 0; y < 7; y++)
        for (int x = 0; x < 21; x++)
          if (ret[y][x] == 'W' && displays[i][y][x] != d[i][y][x]) {
            goto fail;
          }
      }

      valid = true;
      for (int y = 0; y < 7; y++)
      for (int x = 0; x < 21; x++)
        if (ret[y][x] == '0' || ret[y][x] == '1') {
          int i;
          for (i = 0; i < N; i++) {
            if (ret[y][x] == '0' && displays[i][y][x] == 'X') break;
            if (ret[y][x] == '1' && displays[i][y][x] == '.') break;
          }
          if (i == N) ret[y][x] = '?';
        }

fail:;
    }

    if (!valid) {
      cout << "impossible" << endl;
    } else {
      for (int y = 0; y < 7; y++) cout << ret[y] << endl;
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

