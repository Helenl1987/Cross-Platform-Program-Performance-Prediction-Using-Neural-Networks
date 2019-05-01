#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include "papi.h"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <vector>
using namespace std;

struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
  Point operator-(const Point& p) const {return Point(x - p.x, y - p.y);}
  Point operator+(const Point& p) const {return Point(x + p.x, y + p.y);}
  Point operator*(const double& c) const {return Point(x * c, y * c);}
  Point operator/(const double& c) const {return Point(x / c, y / c);}
  bool operator<(const Point& p) const {return x == p.x ? y < p.y : x < p.x;}
  bool operator==(const Point& p) const {return x == p.x && y == p.y;}
  double len() {return hypot(x, y);}
};

double dotp(const Point& a, const Point& b) {return a.x*b.x + a.y*b.y;}
double crossp(const Point& a, const Point& b) {return a.x*b.y - a.y*b.x;}

void LineCircleIntersection(const Point& a1, const Point& a2, double R,
    Point& i1, Point& i2) {
  i1 = i2 = Point(1e18, 1e18);
  double d = fabs(crossp(a1, a2-a1) / (a2-a1).len());
  if (d >= R) return;
  double x1 = dotp(a1, a2-a1) / (a2-a1).len();
  double x2 = dotp(a2, a2-a1) / (a2-a1).len();
  double y1 = -sqrt(R*R-d*d);
  double y2 = sqrt(R*R-d*d);
  if (y1 >= x1 && y1 <= x2 || y1 >= x2 && y1 <= x1) {
    i1 = a1 + (a2-a1)*(y1-x1)/(x2-x1);
  }
  if (y2 >= x1 && y2 <= x2 || y2 >= x2 && y2 <= x1) {
    i2 = a1 + (a2-a1)*(y2-x1)/(x2-x1);
    if (i1.x == 1e18) swap(i1, i2);
  }
}

double add_seg(const Point& a1, const Point& a2) {
  //cout << "line   " << a1.x << ',' << a1.y << " to "
  //                  << a2.x << ',' << a2.y << endl;
  return crossp(a1, a2) / 2;
}

double add_sector(const Point& a1, const Point& a2, double R) {
  //cout << "sector " << a1.x << ',' << a1.y << " to "
  //                  << a2.x << ',' << a2.y << endl;
  return R*R * atan2(crossp(a1, a2), dotp(a1, a2)) / 2;
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
      
      int N, R;
      while (in >> N >> R) {
        vector<Point> P(N);
        for (int i = 0; i < N; i++) in >> P[i].x >> P[i].y;
        double ret = 0.0;

        int sp;
        for (sp = 0; sp < N; sp++) if (P[sp].len() < R) break;

        double ix, iy;
        Point lp(1e18, 1e18);
        for (int i = 0; i < N; i++) {
          Point p1 = P[(sp+i)%N], p2 = P[(sp+i+1)%N];
          Point i1, i2;
          LineCircleIntersection(p1, p2, R, i1, i2);
          if (p1.len() < R) {
            if (p2.len() < R) {
              assert(i1.x == 1e18);
              assert(i2.x == 1e18);
              ret += add_seg(p1, p2);
            } else {
              assert(i1.x != 1e18);
              assert(i2.x == 1e18);
              ret += add_seg(p1, i1);
              lp = i1;
            }
          } else {
            if (p2.len() < R) {
              assert(lp.x != 1e18);
              assert(i1.x != 1e18);
              assert(i2.x == 1e18);
              ret += add_sector(lp, i1, R);
              ret += add_seg(i1, p2);
              lp = Point(1e18, 1e18);
            } else {
              if (i1.x != 1e18) {
                assert(i2.x != 1e18);
                ret += add_seg(i1, i2);
                ret += add_sector(i2, i1, R);
              }
            }
          }
        }
        
        //printf("%.3lf\n", ret);
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

