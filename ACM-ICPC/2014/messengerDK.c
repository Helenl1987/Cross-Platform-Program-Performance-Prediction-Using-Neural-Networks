#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <vector>
using namespace std;

#define EPS 1e-9

struct Point {
  double x, y;
  Point(double x = 0.0, double y = 0.0) : x(x), y(y) {}
  Point operator-(const Point& p) const {return Point(x-p.x, y-p.y);}
  Point operator+(const Point& p) const {return Point(x+p.x, y+p.y);}
  Point operator*(double c) const {return Point(x*c, y*c);}
  Point operator/(double c) const {return Point(x/c, y/c);}
  double len() {return hypot(x, y);}
};

double dotp(const Point& a, const Point& b) {
  return a.x*b.x + a.y*b.y;
}
double crossp(const Point& a, const Point& b) {
  return a.x*b.y - a.y*b.x;
}

double ClosestApproach(const Point& a, const Point& b, const Point& p) {
  if (dotp(b-a, p-a) <= 0) return (p-a).len();
  if (dotp(a-b, p-b) <= 0) return (p-b).len();
  return abs(crossp(b-a, p-a)) / (b-a).len();
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

      int N1, N2;
      while (in >> N1) {
        vector<Point> P1(N1);
        for (int i = 0; i < N1; i++) in >> P1[i].x >> P1[i].y;
        vector<double> T1(N1);
        for (int i = 1; i < N1; i++) T1[i] = T1[i-1] + (P1[i]-P1[i-1]).len();
        in >> N2;
        vector<Point> P2(N2);
        for (int i = 0; i < N2; i++) in >> P2[i].x >> P2[i].y;
        vector<double> T2(N2);
        for (int i = 1; i < N2; i++) T2[i] = T2[i-1] + (P2[i]-P2[i-1]).len();

        if ((P2.back()-P1[0]).len() > T2.back()+EPS) {
          printf("impossible\n");
          if ( (retval=PAPI_read_counters(values, NUM_EVENTS)) != PAPI_OK)
            ERROR_RETURN(retval);
          out << values[0] << endl;

          continue;
        }

        double dlo = 0.0, dhi = T2.back();
        while (dhi-dlo > EPS) {
          double d = (dhi+dlo)/2;
          int i1, i2;
          bool success = false;
          for (i1 = 0, i2 = 0; i1+1 < N1 && i2+1 < N2; ) {
            if (T2[i2+1] < T1[i1] + d) {
              i2++;
              continue;
            }
            double t1 = max(T1[i1]+d, T2[i2]);
            double t2 = min(T1[i1+1]+d, T2[i2+1]);
            assert(t1 < t2+EPS);
            Point v1 = P1[i1+1]-P1[i1];
            Point v2 = P2[i2+1]-P2[i2];
            v1 = v1 / v1.len(); v2 = v2 / v2.len();
            Point p1 = P1[i1] + v1 * (t1-d-T1[i1]);
            Point p2 = P2[i2] + v2 * (t1-T2[i2]);
            if (ClosestApproach(p1, p1+(v1-v2)*(t2-t1), p2) <= d + EPS) {
              success = true;
              break;
            }
            if (T1[i1+1]+d < T2[i2+1]) i1++; else i2++;
          }
          if (success) dhi = d; else dlo = d;
        }

        printf("%.6lf\n", (dhi+dlo)/2);
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

