#include <stdio.h>
#include <stdlib.h>
#include "papi.h"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

char startPlayer;
vector<char> moveType;
vector<vector<int>> moves;

inline char opp(char player) { return player == 'W' ? 'B' : 'W'; }
inline int sqX(int sq) { return (sq-1)%4*2 + 1-((sq-1)/4)%2; }
inline int sqY(int sq) { return (sq-1)/4; }

pair<vector<string>, vector<string>> doit(vector<string> start) {
  vector<string> board = start;
  char player = startPlayer;
  for (int i = 0; i < moves.size(); i++, player = opp(player))
  for (int j = 0; j+1 < moves[i].size(); j++) {
    int sx = sqX(moves[i][j  ]), sy = sqY(moves[i][j  ]);
    int ex = sqX(moves[i][j+1]), ey = sqY(moves[i][j+1]);
    bool promoted = ((player == 'W' && ey == 0) || (player == 'B' && ey == 7)) && islower(board[sy][sx]);

    if (moveType[i] == '-') {
      // Make sure no jumps are possible.
      for (int y = 0; y < 8; y++)
      for (int x = 0; x < 8; x++) if (toupper(board[y][x]) == player)
      for (int dx = -1; dx <= 1; dx += 2)
      for (int dy = -1; dy <= 1; dy += 2) {
        if (board[y][x] == 'w' && dy == 1) continue;
        if (board[y][x] == 'b' && dy == -1) continue;
        int x2 = x+dx+dx, y2 = y+dy+dy;
        if (x2 < 0 || x2 >= 8 || y2 < 0 || y2 >= 8) continue;
        if (toupper(board[y+dy][x+dx]) != opp(player)) continue;
        if (board[y2][x2] == '.') return {{}, {}};  // Failure.
        if (board[y2][x2] == '?') {
          // Jump must be blocked.  Recurse!
          start[y2][x2] = (y2 == 0) ? 'W' : 'w';
          auto ret = doit(start);
          if (ret.first.size()) return ret;
          start[y2][x2] = (y2 == 7) ? 'B' : 'b';
          return doit(start);
        }
      }
    }

    board[ey][ex] = board[sy][sx];
    if (promoted) board[ey][ex] = toupper(board[ey][ex]);
    board[sy][sx] = '.';
    if (moveType[i] == 'x') {
      int mx = (sx+ex)/2, my = (sy+ey)/2;
      board[my][mx] = '.';

      if (j+2 == moves[i].size() && !promoted) {
        // Make sure no further jumps are possible.
        int x = ex, y = ey;
        for (int dx = -1; dx <= 1; dx += 2)
        for (int dy = -1; dy <= 1; dy += 2) {
          if (board[y][x] == 'w' && dy == 1) continue;
          if (board[y][x] == 'b' && dy == -1) continue;
          int x2 = x+dx+dx, y2 = y+dy+dy;
          if (x2 < 0 || x2 >= 8 || y2 < 0 || y2 >= 8) continue;
          if (toupper(board[y+dy][x+dx]) != opp(player)) continue;
          if (board[y2][x2] == '.') return {{}, {}};  // Failure.
          if (board[y2][x2] == '?') {
            // Jump must be blocked.  Recurse!
            start[y2][x2] = (y2 == 0) ? 'W' : 'w';
            auto ret = doit(start);
            if (ret.first.size()) return ret;
            start[y2][x2] = (y2 == 7) ? 'B' : 'b';
            return doit(start);
          }
        }
      }
    }
  }

  return {start, board};
}

#define NUM_EVENTS 1
#define THRESHOLD 10000
#define ERROR_RETURN(retval) { fprintf(stderr, "Error %d %s:line %d: \n", retval,__FILE__,__LINE__);  exit(retval); }

int main(int argc, const char* argv[])
{
   ifstream in;
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
      in.open(argv[1], ios::in);
      cout << i << " round of counting" << endl;
      Events[0] = counters[i];
      if ((retval = PAPI_start_counters(Events, NUM_EVENTS)) != PAPI_OK)
         ERROR_RETURN(retval);

      int N, first = true;
      while (in >> startPlayer >> N) {
        moveType = vector<char>(N);
        moves = vector<vector<int>>(N);
        for (int i = 0; i < N; i++) {
          string s;
          in >> s;
          int j = 0;
          for (;;) {
            int sq = s[j]-'0';
            if (j+1 < s.size() && isdigit(s[j+1])) sq = 10*sq + s[++j]-'0';
            moves[i].push_back(sq);
            if (++j == s.size()) break;
            moveType[i] = s[j++];
          }
        }

        // First place all explicit pieces.
        vector<string> start(8, "????????"), board = start;
        char player = startPlayer;
        vector<vector<int>> startX(8), startY(8);
        for (int y = 0; y < 8; y++)
        for (int x = 0; x < 8; x++) {
          startX[y].push_back(x);
          startY[y].push_back(y);
        }
        for (int i = 0; i < moves.size(); i++, player = opp(player))
        for (int j = 0; j+1 < moves[i].size(); j++) {
          int sx = sqX(moves[i][j  ]), sy = sqY(moves[i][j  ]);
          int ex = sqX(moves[i][j+1]), ey = sqY(moves[i][j+1]);
          bool promoted = ((player == 'W' && ey == 0) || (player == 'B' && ey == 7));
          if (board[sy][sx] == '?') {
            board[sy][sx] = start[sy][sx] = tolower(player);  // Prefer man until proven otherwise.
          }
          if (board[ey][ex] == '?') {
            board[ey][ex] = start[ey][ex] = '.';
          }
          assert(toupper(board[sy][sx]) == player);
          assert(board[ey][ex] == '.');
          if (((player == 'W') ^ (ey < sy)) && islower(board[sy][sx])) {
            board[sy][sx] = start[startY[sy][sx]][startX[sy][sx]] = toupper(board[sy][sx]);  // Forced king.
          }
          board[ey][ex] = board[sy][sx];
          if (promoted && j+2 == moves[i].size()) board[ey][ex] = toupper(board[ey][ex]);
          board[sy][sx] = '.';
          startX[ey][ex] = startX[sy][sx];
          startY[ey][ex] = startY[sy][sx];
          if (moveType[i] == 'x') {
            int mx = (sx+ex)/2, my = (sy+ey)/2;
            if (board[my][mx] == '?') {
              board[my][mx] = start[my][mx] = tolower(opp(player));
            }
            assert(toupper(board[my][mx]) == opp(player));
            board[my][mx] = '.';
          }
        }

        // Now recurse on colours of implicit pieces.
        auto ret = doit(start);
        assert(ret.first.size());

        //if (!first) cout << endl;
        first = false;
        for (int y = 0; y < 8; y++)
        for (int x = y%2; x < 8; x += 2) {
          ret.first[y][x] = ret.second[y][x] = '-';
        }
        for (int y = 0; y < 8; y++)
        for (int x = 1-y%2; x < 8; x += 2) {
          if (ret.first[y][x] == '?') ret.first[y][x] = '.';
          if (ret.second[y][x] == '?') ret.second[y][x] = '.';
        }
        for (int y = 0; y < 8; y++) cout << ret.first[y] << ' ' << ret.second[y] << endl;
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



