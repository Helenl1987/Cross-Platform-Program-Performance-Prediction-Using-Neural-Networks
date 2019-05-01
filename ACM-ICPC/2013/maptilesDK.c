
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include "papi.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <iostream>
#include <vector>
using namespace std;

#define MAXN 50
#define MAXM 50
#define EPSILON 1e-6

struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
  Point operator-(const Point& p) const {return Point(x - p.x, y - p.y);}
  Point operator+(const Point& p) const {return Point(x + p.x, y + p.y);}
  Point operator*(const double& c) const {return Point(x * c, y * c);}
  Point operator/(const double& c) const {return Point(x / c, y / c);}
  bool operator<(const Point& p) const {return x == p.x ? y < p.y : x < p.x;}
  bool operator==(const Point& p) const {return x == p.x && y == p.y;}
};

double dotp(const Point& a, const Point& b) {return a.x*b.x + a.y*b.y;}
double crossp(const Point& a, const Point& b) {return a.x*b.y - a.y*b.x;}

// Includes endpoints.  Returns (1000000000,1000000000) if no intersection.
Point intersection(const Point& a1, const Point& a2, const Point& b1, const Point& b2) {
  double cp1 = crossp(a2-a1, b1-a1);
  double cp2 = crossp(a2-a1, b2-a1);
  if (cp1 < 0 && cp2 < 0 || cp1 > 0 && cp2 > 0 || cp1 == 0 && cp2 == 0)
    return Point(1000000000, 1000000000);
  cp1 = crossp(b2-b1, a1-b1);
  cp2 = crossp(b2-b1, a2-b1);
  if (cp1 < 0 && cp2 < 0 || cp1 > 0 && cp2 > 0)
    return Point(1000000000, 1000000000);
  return (a2-a1) * (cp1 / (cp1-cp2)) + a1;
}

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
      
      int N, X, Y;
      while (in >> N >> X >> Y) {
        vector<Point> P(N+1);
        for (int i = 0; i < N; i++) {
          int x, y;
          in >> x >> y;
          P[i] = Point(x, y);
        }
        P[N] = P[0];

        vector<Point> poss;  // All "interesting" points for tile corners.
        // Treat all lattice points as "interesting".
        for (int x = 0; x < X; x++)
        for (int y = 0; y < Y; y++)
          poss.push_back(Point(x, y));
        // All intersections of edges with grid lines are "interesting".
        for (int i = 0; i < N; i++) {
          for (int x = (int)min(P[i].x, P[i+1].x)+1;
              x < (int)max(P[i].x, P[i+1].x);
              x++)
            poss.push_back(intersection(Point(x, -10000), Point(x, 10000),
                                        P[i], P[i+1]));
          for (int y = (int)min(P[i].y, P[i+1].y)+1;
              y < (int)max(P[i].y, P[i+1].y);
              y++)
            poss.push_back(intersection(Point(-10000, y), Point(10000, y),
                                        P[i], P[i+1]));
        }
        // All intersections of edges translated by (n*X, m*Y) with other edges
        // are "interesting".
        for (int n = -MAXM; n <= MAXM; n++)
        for (int m = -MAXM; m <= MAXM; m++) {
          Point translate(n*X, m*Y);
          for (int i = 0; i < N; i++) {
            Point a1 = P[i] + translate, a2 = P[i+1] + translate;
            for (int j = 0; j < N; j++) {
              // Shortcut expensive calculations with a bounding box.
              if (max(a1.x, a2.x) <= min(P[j].x, P[j+1].x)) continue;
              if (min(a1.x, a2.x) >= max(P[j].x, P[j+1].x)) continue;
              if (max(a1.y, a2.y) <= min(P[j].y, P[j+1].y)) continue;
              if (min(a1.y, a2.y) >= max(P[j].y, P[j+1].y)) continue;
              poss.push_back(intersection(a1, a2, P[j], P[j+1]));
            }
          }
        }

        // Prune duplicates (mod the tile dimensions).
        for (int i = 0; i < poss.size(); i++) {
          poss[i].x -= (int)poss[i].x / X * X;
          poss[i].y -= (int)poss[i].y / Y * Y;
        }
        sort(poss.begin(), poss.end());
        poss.erase(unique(poss.begin(), poss.end()), poss.end());

        // For each "interesting" point, count the number of map tiles needed.
        int ret = 1000000000;
        for (int pi = 0; pi < poss.size(); pi++) {
          vector<Point> Q(P.size());
          for (int j = 0; j < P.size(); j++) Q[j] = P[j] + Point(X,Y) - poss[pi];
          
          // Scan rows of tiles, starting at y=0 and moving up, until we're past
          // the polygon.
          int cur = 0, lastcur = 0;
          for (int y = 0; cur == 0 || cur != lastcur; y += Y) {
            lastcur = cur;

            // Keep track of which tiles are crossed by edges within the row itself.
            int tile[MAXM+10];
            memset(tile, 0, sizeof(tile));
            // Keep track of points where edges enter the row from the bottom/top.
            vector<double> bot, top;
            double boty = y+EPSILON, topy = y+Y-EPSILON;
            for (int i = 0; i < N; i++) {
              if (Q[i].y <= boty && Q[i+1].y <= boty) continue;
              if (Q[i].y >= topy && Q[i+1].y >= topy) continue;
              Point a = Q[i], b = Q[i+1];

              if (a.y > b.y) swap(a, b);
              if (a.y < boty) {
                a.x = (Q[i  ].x * (Q[i+1].y-boty) -
                      Q[i+1].x * (Q[i  ].y-boty)) / (Q[i+1].y-Q[i].y);
                a.y = boty;
              }
              if (b.y > topy) {
                b.x = (Q[i  ].x * (Q[i+1].y-topy) -
                      Q[i+1].x * (Q[i  ].y-topy)) / (Q[i+1].y-Q[i].y);
                b.y = topy;
              }
              if (a.y == boty) bot.push_back(a.x-EPSILON);
              if (b.y == topy) top.push_back(b.x-EPSILON);

              if (a.x > b.x) swap(a, b);
              a.x = a.x + EPSILON;
              b.x = b.x - EPSILON;
              int xs = (int)a.x / X;
              int xe = (int)b.x / X;
              for (int x = xs; x <= xe; x++) tile[x] = true;
            }
            sort(bot.begin(), bot.end());
            sort(top.begin(), top.end());

            for (int xi = 0, bi = 0, ti = 0; xi < MAXM+10; xi++) {
              while (bi < bot.size() && bot[bi] <= xi*X) bi++;
              while (ti < top.size() && top[ti] <= xi*X) ti++;

              if (tile[xi]) {cur++; continue;}
              assert(bi%2 == ti%2);
              if (bi%2) cur++;
            }
          }

    #if 0
          if (cur <= ret) {
            cerr << '(' << poss[pi].x << ", " << poss[pi].y << "): " << cur << endl;
          }
    #endif
          ret = min(ret, cur);
        }

        //cout << ret << endl;
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
