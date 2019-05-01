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


#define EPS 1e-8
#define PI 3.14159265358979323846

struct Point {
  double x, y;
  Point(double x = 0.0, double y = 0.0) : x(x), y(y) {}
  Point operator+(const Point& p) const {return Point(x+p.x, y+p.y);}
  Point operator-(const Point& p) const {return Point(x-p.x, y-p.y);}
  Point operator*(double c) const {return Point(x*c, y*c);}
  Point operator/(double c) const {return Point(x/c, y/c);}
  Point angle() const {return atan2(y, x);}
  double len() const {return hypot(y, x);}
  bool operator==(const Point& p) const {
    return fabs(x-p.x) < EPS && fabs(y-p.y) < EPS;
  }
  Point rotate(double th) const {
    return Point(x*cos(th) - y*sin(th), x*sin(th) + y*cos(th));
  }
  friend ostream& operator<<(ostream& out, const Point& p) {
    out << '(' << p.x << ',' << p.y << ')';
    return out;
  }
};

double dotp(const Point& a, const Point& b) {
  return a.x*b.x + a.y*b.y;
}
double crossp(const Point& a, const Point& b) {
  return a.x*b.y - a.y*b.x;
}
double angle(const Point& a, const Point& b) {
  return atan2(crossp(a, b), dotp(a, b));
}

// Interior of line only.
bool LinePointIntersection(const Point& a1, const Point& a2, const Point& b) {
  double cp = crossp(a2-a1, b-a1);
  if (cp < -EPS || cp > EPS) return false;
  double dp = dotp(a2-a1, b-a1);
  if (dp < EPS) return false;
  dp = dotp(a1-a2, b-a2);
  if (dp < EPS) return false;
  return true;
}

// Interior of lines only.  Doesn't count overlap.
bool LineLineIntersection(const Point& a1, const Point& a2,
                          const Point& b1, const Point& b2) {
  double cp1 = crossp(a2-a1, b1-a1);
  double cp2 = crossp(a2-a1, b2-a1);
  if (cp1 < EPS && cp2 < EPS) return false;
  if (cp1 > -EPS && cp2 > -EPS) return false;
  cp1 = crossp(b2-b1, a1-b1);
  cp2 = crossp(b2-b1, a2-b1);
  if (cp1 < EPS && cp2 < EPS) return false;
  if (cp1 > -EPS && cp2 > -EPS) return false;
  return true;
}

bool TryPolyIntersection(const vector<Point>& p1, const vector<Point>& p2,
    int i, int j) {
  const Point& a0 = p1[(i+p1.size()-1)%p1.size()];
  const Point& a1 = p1[i];
  const Point& a2 = p1[(i+1)%p1.size()];
  const Point& b0 = p2[(j+p2.size()-1)%p2.size()];
  const Point& b1 = p2[j];
  const Point& b2 = p2[(j+1)%p2.size()];

  if (LineLineIntersection(b1, b2, a1, a2)) return true;

  if (LinePointIntersection(b1, b2, a1)) {
    // Test if either incident line cuts into polygon.
    if (crossp(b2-b1, a0-b1) > EPS) return true;
    if (crossp(b2-b1, a2-b1) > EPS) return true;
  }
  if (LinePointIntersection(a1, a2, b1)) {
    // Test if either incident line cuts into polygon.
    if (crossp(a2-a1, b0-a1) > EPS) return true;
    if (crossp(a2-a1, b2-a1) > EPS) return true;
  }

  if (a1 == b1) {
    // Test if incident lines cut each other.
    double th = angle(b2-b1, b0-b1);
    if (th < 0.0) th += 2*PI;
    double th2 = angle(b2-b1, a0-b1);
    if (th2 < 0.0) th2 += 2*PI;
    if (th2 > EPS && th2 < th-EPS) return true;
    th2 = angle(b2-b1, a2-b1);
    if (th2 < 0.0) th2 += 2*PI;
    if (th2 > EPS && th2 < th-EPS) return true;
  }

  return false;
}

bool PolyIntersection(const vector<Point>& p1, const vector<Point>& p2) {
  static int ihint = -1, jhint = -1;
  if (ihint != -1 && TryPolyIntersection(p1, p2, ihint, jhint)) return true;

  // Polygons must be defined in ccl order.
  // NOTE: We don't need a strict containment check for this problem.
  for (int i = 0; i < p1.size(); i++)
  for (int j = 0; j < p2.size(); j++) {
    if (TryPolyIntersection(p1, p2, i, j)) {
      ihint = i; jhint = j;
      return true;
    }
  }
  return false;
}

double PolyCommonBoundary(const vector<Point>& p1, const vector<Point>& p2) {
  double ret = 0.0;
  for (int i = 0; i < p1.size(); i++) {
    const Point& a1 = p1[i];
    const Point& a2 = p1[(i+1)%p1.size()];
    double ln = (a2-a1).len();
    for (int j = 0; j < p2.size(); j++) {
      const Point& b1 = p2[j];
      const Point& b2 = p2[(j+1)%p2.size()];
      double cp1 = crossp(a2-a1, b1-a1);
      if (cp1 < -EPS || cp1 > EPS) continue;
      double cp2 = crossp(a2-a1, b2-a1);
      if (cp2 < -EPS || cp2 > EPS) continue;
      double dp1 = dotp(a2-a1, b1-a1) / ln;
      double dp2 = dotp(a2-a1, b2-a1) / ln;
      dp1 = max(0.0, min(dp1, ln));
      dp2 = max(0.0, min(dp2, ln));
      ret += fabs(dp2-dp1);
    }
  }
  return ret;
}

double PolySlide(const vector<Point>& p1, const vector<Point>& p2,
                 const Point& vec, double maxdist) {
  double ret = 0.0;

  // Try each point-line collision along vector "vec".
  vector<double> cand;
  for (int i1 = 0; i1 < p1.size(); i1++) {
    const Point& l1 = p1[i1];
    const Point& l2 = p1[(i1+1)%p1.size()];
    double delta = crossp(l2-l1, vec);
    if (fabs(delta) < EPS) continue;
    for (int i2 = 0; i2 < p2.size(); i2++) {
      double dist = crossp(l2-l1, p2[i2]-l1) / -delta;
      if (dotp(l2-l1, p2[i2]+vec*dist-l1) < -EPS) continue;
      if (dotp(l1-l2, p2[i2]+vec*dist-l2) < -EPS) continue;
//cout << " cand1 " << dist << ": " << l1 << '-' << l2 << ' ' << p2[i2] << endl;
      cand.push_back(crossp(l2-l1, p2[i2]-l1) / -delta);
    }
  }
  for (int i2 = 0; i2 < p2.size(); i2++) {
    const Point& l1 = p2[i2];
    const Point& l2 = p2[(i2+1)%p2.size()];
    double delta = crossp(l2-l1, vec);
    if (fabs(delta) < EPS) continue;
    for (int i1 = 0; i1 < p1.size(); i1++) {
      double dist = crossp(l2-l1, p1[i1]-l1) / delta;
      if (dotp(l2-l1, p1[i1]-vec*dist-l1) < -EPS) continue;
      if (dotp(l1-l2, p1[i1]-vec*dist-l2) < -EPS) continue;
//cout << " cand2 " << dist << ": " << p1[i1] << ' ' << l1 << '-' << l2 << endl;
      cand.push_back(crossp(l2-l1, p1[i1]-l1) / delta);
    }
  }
  sort(cand.begin(), cand.end());

  for (int ci = 0; ci < cand.size(); ci++) {
    if (cand[ci] < -EPS) continue;
    if (cand[ci] > maxdist+EPS) continue;
    if (ci && cand[ci]-cand[ci-1] < EPS) continue;
    vector<Point> p3(p2.size());
    for (int k = 0; k < p2.size(); k++) p3[k] = p2[k] + vec*cand[ci];
//for (int k = 0; k < p1.size(); k++) cout << p1[k] << ' ';
//cout << endl;
//for (int k = 0; k < p2.size(); k++) cout << p3[k] << ' ';
//cout << endl;
//cout << (PolyIntersection(p1, p3) ? "intersect" : "don't intersect") << endl;
    if (!PolyIntersection(p1, p3)) {
//cout << PolyCommonBoundary(p1, p3) << endl;
      ret = max(ret, PolyCommonBoundary(p1, p3));
    }
//cout << endl;
  }

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


  int N1, N2;
  while (in >> N1) {
    vector<Point> p1(N1);
    for (int i = 0; i < N1; i++) in >> p1[i].x >> p1[i].y;
    in >> N2;
    vector<Point> p2(N2);
    for (int i = 0; i < N2; i++) in >> p2[i].x >> p2[i].y;
    reverse(p1.begin(), p1.end());
    reverse(p2.begin(), p2.end());

    double ret = 0.0;
    for (int i = 0; i < N1; i++)
    for (int j = 0; j < N2; j++) {
      const Point& a1 = p1[i];
      const Point& a2 = p1[(i+1)%N1];
      const Point& b1 = p2[j];
      const Point& b2 = p2[(j+1)%N2];

      // Rotate b1-b2 then slide it along a1-a2.
//cout << " base " << a1 << '-' << a2 << ", sliding " << b1 << '-' << b2 << endl;
      double th = angle(b1-b2, a2-a1);
      vector<Point> p3(N2);
      for (int k = 0; k < N2; k++) p3[k] = p2[k].rotate(th);
      Point diff = p1[i] - p3[j];
      for (int k = 0; k < N2; k++) p3[k] = p3[k] + diff;
      Point vec = (a2-a1)/(a2-a1).len();
      double maxdist = (a2-a1).len() + (b2-b1).len();
//cout << " vec=" << vec << " maxdist=" << maxdist << endl;

      ret = max(ret, PolySlide(p1, p3, vec, maxdist));
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

