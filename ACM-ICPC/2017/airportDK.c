
#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <vector>
using namespace std;

#define NUM_EVENTS 1
#define THRESHOLD 10000
#define ERROR_RETURN(retval) { fprintf(stderr, "Error %d %s:line %d: \n", retval,__FILE__,__LINE__);  exit(retval); }


struct Point {
  double x, y;
  Point(double x = 0.0, double y = 0.0) : x(x), y(y) {}
  Point operator-(const Point &p) const { return Point(x-p.x, y-p.y); }
  Point operator*(double c) const { return Point(x*c, y*c); }
  Point operator/(double c) const { return Point(x/c, y/c); }
  double len() const { return hypot(x, y); }
};

double CrossP(const Point& a, const Point& b) {
  return a.x*b.y - a.y*b.x;
}
double DotP(const Point& a, const Point& b) {
  return a.x*b.x + a.y*b.y;
}
Point Intersection(const Point& a1, const Point& a2,
                  const Point& b1, const Point& b2) {
  double v1 = CrossP(b2-b1, a1-b1);
  double v2 = CrossP(b2-b1, a2-b1);
  return (a1*v2 - a2*v1) / (v2-v1);
}

vector<Point> P;


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
    P = vector<Point>(N);
    for (int i = 0; i < N; i++) in >> P[i].x >> P[i].y;

    // Eliminate colinear edges.
    for (int i = 0; i < N; i++) {
      if (CrossP(P[i]-P[(i+N-1)%N], P[(i+1)%N]-P[i]) == 0) {
        P.erase(P.begin() + i);
        i--; N--;
      }
    }

    // Make sure vertices are in CCL order.
    double ar = 0.0;
    for (int i = 0; i < N; i++) ar += CrossP(P[i], P[(i+1)%N]);
    if (ar < 0.0) reverse(P.begin(), P.end());

    double ret = 0.0;
    for (int i = 0; i < N; i++)
    for (int j = i+1; j < N; j++) {
      Point a = P[i], b = P[j];
      if (CrossP(b-a, P[(i+N-1)%N]-a) * CrossP(b-a, P[(i+1)%N]-a) > 0 &&
          CrossP(a-P[(i+N-1)%N], P[(i+1)%N]-a) > 0) {
        // Normally an incident vertex doesn't matter... unless it's vertex a.
        continue;
      }
      double alen = 1e50, blen = 1e50;
//cout << a.x << ',' << a.y << " - " << b.x << ',' << b.y << ":" << endl;
      for (int k = 0; k < N; k++) {
        Point p0 = P[(k+N-1)%N], p1 = P[k], p2 = P[(k+1)%N], p3 = P[(k+2)%N];
        if (CrossP(b-a, p1-a) == 0 && CrossP(b-a, p2-a) == 0) {
          // p1-p2 colinear with a-b.
          double dp1 = DotP(b-a, p1-a), dp2 = DotP(b-a, p2-a);
          if (dp2 <= 0 && dp2 <= dp1 && CrossP(b-a, p3-a) < 0) {
//cout << " alen p2: " << p2.x << ',' << p2.y << endl;
            alen = min(alen, DotP(a-b, p2-a));
          }
          if (dp1 <= 0 && dp1 <= dp2 && CrossP(b-a, p0-a) > 0) {
//cout << " alen p1: " << p1.x << ',' << p1.y << endl;
            alen = min(alen, DotP(a-b, p1-a));
          }
          if (dp2 >= 0 && dp2 >= dp1 && CrossP(b-a, p3-a) > 0) {
//cout << " blen p2: " << p2.x << ',' << p2.y << endl;
            blen = min(blen, DotP(b-a, p2-a));
          }
          if (dp1 >= 0 && dp1 >= dp2 && CrossP(b-a, p0-a) < 0) {
//cout << " blen p1: " << p1.x << ',' << p1.y << endl;
            blen = min(blen, DotP(b-a, p1-a));
          }
        } else if (CrossP(p2-p1, a-p1) >= 0 && CrossP(b-a, p2-a) < 0 &&
                   (CrossP(b-a, p1-a) > 0 ||
                    CrossP(b-a, p1-a) == 0 && CrossP(b-a, p0-a) > 0)) {
          Point ip = Intersection(p1, p2, a, b);
//cout << " alen ip: " << ip.x << ',' << ip.y << endl;
          alen = min(alen, DotP(a-b, ip-a));
        } else if (CrossP(p2-p1, a-p1) >= 0 && CrossP(b-a, p2-a) > 0 &&
                   (CrossP(b-a, p1-a) < 0 ||
                    CrossP(b-a, p1-a) == 0 && CrossP(b-a, p0-a) < 0)) {
          Point ip = Intersection(p1, p2, a, b);
//cout << " blen ip: " << ip.x << ',' << ip.y << endl;
          blen = min(blen, DotP(b-a, ip-a));
        }
      }
//cout << "  " << (alen+blen)/(b-a).len() << endl;
      ret = max(ret, (alen+blen)/(b-a).len());
    }

    printf("%.9lf\n", ret);


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

