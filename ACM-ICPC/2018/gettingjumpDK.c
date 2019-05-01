#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>
#include <cmath>
#include <cstring>
#include <iostream>
#include <tuple>
#include <vector>
using namespace std;

#define EPS 1e-9
#define G 9.80665

bool intersect(double vd, double vh, double dx, double dy, double x, double y1, double y2, double bh) {
  if (dx < 0.0) { dx = -dx; x = -x; }
  if (x < 0.0 || x > dx) return false;
  if (dx*y1 > x*dy) return false;  // exact
  if (dx*y2 < x*dy) return false;  // exact
  // d(t) = vd * t, h(t) = vh * t - 1/2 * G * t^2
  double d = hypot(x, x*dy/dx);
  double t = d / vd;
  double h = vh * t - G/2.0 * t*t;
  //if (h < bh+EPS) cout << "  fail " << d << " " << t << " " << h << endl;
  return h < bh+EPS;
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
      
      double w, v;
  int dx, dy, lx, ly;
  while (in >> dx >> dy >> w >> v >> lx >> ly) {
    vector<vector<int>> g(dy+1, vector<int>(dx+1));
    vector<vector<int>> dist(dy+1, vector<int>(dx+1, 1000000000));
    for (int y = 1; y <= dy; y++)
    for (int x = 1; x <= dx; x++) {
      in >> g[y][x];
    }

    int curd = 0;
    vector<pair<int, int>> q{{lx, ly}};
    dist[ly][lx] = 0;
    while (!q.empty()) {
      curd++;
      vector<pair<int, int>> q2;
      for (auto c : q) {
        int x, y;
        tie(x, y) = c;
        for (int x2 = 1; x2 <= dx; x2++)
        for (int y2 = 1; y2 <= dy; y2++) if (dist[y2][x2] > curd) {
          double d = w * hypot(x2-x, y2-y);
          double h = g[y2][x2]-g[y][x];
          double a = h*h + d*d;
          double b = -2*h*h*v*v - G*d*d*h - d*d*v*v;
          double c = h*h*v*v*v*v + G*d*d*h*v*v + G*G*d*d*d*d/4;
          double disc = b*b-4*a*c;
          if (disc < -EPS) continue;
          double vhsqr = (-b+sqrt(disc))/2/a;  // Two vh will work; choose higher one.
          if (vhsqr < EPS || vhsqr > v*v-EPS) continue;
          double vd = sqrt(v*v-vhsqr), vh = sqrt(vhsqr);
          //cout << '(' << x << ',' << y << ") -> (" << x2 << ',' << y2 << ") d=" << d << " h=" << h;
          //cout << " a=" << a << " b=" << b << " c=" << c << " vd=" << vd << " vh=" << vh;
          //cout << " h2=" << vh*(d/vd) - G/2*(d/vd)*(d/vd) << endl;

          bool fail = false;
          for (int x3 = min(x, x2); !fail && x3 <= max(x, x2); x3++)
          for (int y3 = min(y, y2); !fail && y3 <= max(y, y2); y3++) {
            fail |= intersect(vd/w, vh, x2-x, y2-y, x3+0.5-x, y3-0.5-y, y3+0.5-y, g[y3][x3]-g[y][x]);
            fail |= intersect(vd/w, vh, x2-x, y2-y, x3-0.5-x, y3-0.5-y, y3+0.5-y, g[y3][x3]-g[y][x]);
            fail |= intersect(vd/w, vh, y2-y, x2-x, y3+0.5-y, x3-0.5-x, x3+0.5-x, g[y3][x3]-g[y][x]);
            fail |= intersect(vd/w, vh, y2-y, x2-x, y3-0.5-y, x3-0.5-x, x3+0.5-x, g[y3][x3]-g[y][x]);
          }
          if (fail) continue;

          q2.push_back({x2, y2});
          dist[y2][x2] = curd;
        }
      }
      q.swap(q2);
    }

    for (int y = 1; y <= dy; y++) {
      for (int x = 1; x <= dx; x++) {
        if (x > 1) cout << ' ';
        if (dist[y][x] > dx*dy) cout << 'X'; else cout << dist[y][x];
      }
      cout << endl;
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

