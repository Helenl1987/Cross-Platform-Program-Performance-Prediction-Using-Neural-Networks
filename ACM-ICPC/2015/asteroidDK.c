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

Point intersect(const Point& l1a, const Point& l1b,
                const Point& l2a, const Point& l2b) {
  double cp1 = crossp(l1b-l1a, l2a-l1a);
  double cp2 = crossp(l1b-l1a, l2b-l1a);
  return (l2a * cp2 - l2b * cp1) / (cp2 - cp1);
}

vector<Point> ClipPoly(const vector<Point>& p, const Point& a, const Point& b) {
  vector<Point> ret;
  int i;
  for (i = 0; i < p.size(); i++) {
    if (crossp(b-a, p[i]-a) < 0) break;
  }
  if (i == p.size()) return ret;
  for (i = 0; i < p.size(); i++) {
    const Point& p1 = p[i];
    const Point& p2 = p[(i+1)%p.size()];
    if (crossp(b-a, p1-a) >= 0 && crossp(b-a, p2-a) < 0) {
      ret.push_back(intersect(p1, p2, a, b));
      break;
    }
  }
  if (i == p.size()) return p;
  for (int j = 1; ; j++) {
    const Point& p1 = p[(i+j)%p.size()];
    const Point& p2 = p[(i+j+1)%p.size()];
    ret.push_back(p1);
    if (crossp(b-a, p2-a) >= 0) {
      ret.push_back(intersect(p1, p2, a, b));
      break;
    }
  }
  return ret;
}

double PolyArea(const vector<Point>& p) {
  double ret = 0.0;
  for (int i = 0; i < p.size(); i++) ret -= crossp(p[i], p[(i+1)%p.size()]);
  return ret/2;
}

double calc(vector<Point> p1, const vector<Point>& p2, Point v, double t) {
  for (int j = 0; j < p1.size(); j++) p1[j] = p1[j] + v*t;
  for (int j = 0; j < p2.size(); j++) {
    p1 = ClipPoly(p1, p2[j], p2[(j+1)%p2.size()]);
  }
  return PolyArea(p1);
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

      int N;
  while (in >> N) {
    Point v1, v2;
    vector<Point> p1(N);
    for (int i = 0; i < N; i++) in >> p1[i].x >> p1[i].y;
    in >> v1.x >> v1.y;
    in >> N;
    vector<Point> p2(N);
    for (int i = 0; i < N; i++) in >> p2[i].x >> p2[i].y;
    in >> v2.x >> v2.y;
    v1 = v1-v2;

    vector<double> vt;
    for (int swp = 0; swp < 2; swp++) {
      for (int i = 0; i < p1.size(); i++) {
        const Point& p1a = p1[i];
        const Point& p1b = p1[(i+1)%p1.size()];
        double v = crossp(p1b-p1a, v1);
        if (v == 0) continue;
        for (int j = 0; j < p2.size(); j++) {
          Point ip = intersect(p1a, p1b, p2[j], p2[j] + v1);
          if (dotp(p1b-p1a, ip-p1a) < 0) continue;
          if (dotp(p1a-p1b, ip-p1b) < 0) continue;
          double cp = crossp(p1b-p1a, p2[j]-p1a);
          double t = cp/v;
          if (t <= 0) continue;
          vt.push_back(t);
        }
      }
      swap(p1, p2);
      v1 = v1 * -1;
    }
    sort(vt.begin(), vt.end());

    double rett = 1e10, retarea = -1;
    for (int i = 0; i+1 < vt.size(); i++) {
      double lot, hit;
      for (lot = vt[i], hit = vt[i+1]; hit-lot > EPS; ) {
        double t1 = (2*lot + hit)/3;
        double t2 = (2*hit + lot)/3;
        if (calc(p1, p2, v1, t1) > calc(p1, p2, v1, t2)) {
          hit = t2;
        } else {
          lot = t1;
        }
      }
      double ar = calc(p1, p2, v1, vt[i]);
      if (ar > retarea+EPS) {retarea = ar; rett = vt[i];}
      ar = calc(p1, p2, v1, (lot+hit)/2);
      if (ar > retarea+EPS) {retarea = ar; rett = (lot+hit)/2;}
    }

    if (vt.size() == 0) {
      printf("never\n");
    } else if (vt.size() == 1) {
      printf("%.6lf\n", vt[0]);
    } else {
      printf("%.6lf\n", rett);
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

