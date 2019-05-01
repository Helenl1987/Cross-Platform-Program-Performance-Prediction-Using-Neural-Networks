#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>

#include <cassert>
#include <cstdio>
#include <cstring>
#include <iostream>
using namespace std;

int W, H, Pu, Pd, Pl, Pr;
char g[10000][21];
bool seen[10000][20];
double M[10000][20][61], V[10000][20];

// A DFS isn't strictly necessary but it simplifies the GE.
void dfs(int x, int y) {
  if (x < 0 || x >= W || y < 0 || y >= H || g[y][x] == 'X') return;
  if (seen[y][x]) return;
  seen[y][x] = true;
  if (g[y][x] == 'T') return;
  dfs(x-1, y);
  dfs(x+1, y);
  dfs(x, y-1);
  dfs(x, y+1);
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

      while (in >> W >> H) {
        in >> Pu >> Pd >> Pl >> Pr;
        for (int y = 0; y < H; y++) in >> g[y];
        memset(seen, 0, sizeof(seen));
        memset(M, 0, sizeof(M));
        memset(V, 0, sizeof(V));

        int ns = 0;
        for (int x = 0; x < W; x++) if (g[0][x] == '.') {ns++; dfs(x, 0);}
        for (int x = 0; x < W; x++) if (g[0][x] == '.') M[0][x][60] = 1.0 / ns;

        for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++) {
          M[y][x][20+x] = 1.0;
          if (g[y][x] != '.' || !seen[y][x]) continue;
          double P = 0;
          if (x > 0   && g[y][x-1] != 'X') P += Pl;
          if (x < W-1 && g[y][x+1] != 'X') P += Pr;
          if (y > 0   && g[y-1][x] != 'X') P += Pu;
          if (y < H-1 && g[y+1][x] != 'X') P += Pd;
          if (P == 0) continue;
          if (x > 0   && g[y][x-1] != 'X') M[y][x-1][20+x] -= Pl / P;
          if (x < W-1 && g[y][x+1] != 'X') M[y][x+1][20+x] -= Pr / P;
          if (y > 0   && g[y-1][x] != 'X') M[y-1][x][40+x] -= Pu / P;
          if (y < H-1 && g[y+1][x] != 'X') M[y+1][x][   x] -= Pd / P;
        }

        // Run a specialized, efficient GE to upper-triangularize M.
        for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++) if (seen[y][x]) {
          // Every reachable space should still have a positive coefficient.
          // No need for row swaps.
          double c = M[y][x][20+x];
          for (int i = 20+x; i < 61; i++) M[y][x][i] /= c;
          for (int x2 = x+1; x2 < W; x2++) {  // Rest of this row
            c = M[y][x2][20+x];
            for (int i = 20+x; i < 61; i++) M[y][x2][i] -= c * M[y][x][i];
          }
          if (y < H-1) for (int x2 = 0; x2 < W; x2++) {  // Next row
            c = M[y+1][x2][x];
            for (int i = 20+x; i < 60; i++) M[y+1][x2][i-20] -= c * M[y][x][i];
            M[y+1][x2][60] -= c * M[y][x][60];
          }
        }

        // Now compute the solution V, bottom-up.
        double tot = 0.0;
        for (int y = H-1; y >= 0; y--)
        for (int x = W-1; x >= 0; x--) if (seen[y][x]) {
          double& v = V[y][x];
          v = M[y][x][60];
          for (int i = 20+x+1; i < 40; i++) v -= M[y][x][i] * V[y][i-20];
          if (y < H-1) for (int i = 40; i < 60; i++) v -= M[y][x][i] * V[y+1][i-40];
          tot += v;
        }
        assert(tot < 1e9);

        for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++) if (g[y][x] == 'T') {
          printf("%.9lf\n", V[y][x]);
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


