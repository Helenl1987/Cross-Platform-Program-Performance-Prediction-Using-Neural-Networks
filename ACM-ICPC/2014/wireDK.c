#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <map>
#include <set>
#include <vector>
using namespace std;

#define EPS 1e-8

struct Point {
  double x, y;
  int cost;
  Point(double x = 0.0, double y = 0.0) : x(x), y(y), cost(0) {}
  Point operator-(const Point& p) const {return Point(x-p.x, y-p.y);}
  Point operator+(const Point& p) const {return Point(x+p.x, y+p.y);}
  Point operator*(double c) const {return Point(x*c, y*c);}
  Point operator/(double c) const {return Point(x/c, y/c);}
  double len() {return hypot(x, y);}
  bool operator==(const Point& p) const {
    return fabs(x-p.x) <= EPS && fabs(y-p.y) <= EPS;
  }
  bool operator<(const Point& p) const {
    if (fabs(x-p.x) > EPS) return x < p.x;
    if (fabs(y-p.y) > EPS) return y < p.y;
    return false;
  }
};

double dotp(const Point& a, const Point& b) {
  return a.x*b.x + a.y*b.y;
}
double crossp(const Point& a, const Point& b) {
  return a.x*b.y - a.y*b.x;
}

Point intersect(const Point& l1a, const Point& l1b,
                const Point& l2a, const Point& l2b) {
  double cp1 = crossp(l2b-l2a, l1a-l2a);
  double cp2 = crossp(l2b-l2a, l1b-l2a);
  if (cp1 < 0 && cp2 < 0 || cp1 > 0 && cp2 > 0) return Point(1e10, 1e10);
  if (cp1 == 0 && cp2 == 0) {
    double dp1 = dotp(l2b-l2a, l1a-l2a);
    double dp2 = dotp(l2b-l2a, l1b-l2a);
    if (dp1 < 0 && dp2 < 0) return Point(1e10, 1e10);
    if (dp1 <= 0 && dp2 <= 0) return l2a;
    dp1 = dotp(l2a-l2b, l1a-l2b);
    dp2 = dotp(l2a-l2b, l1b-l2b);
    if (dp1 < 0 && dp2 < 0) return Point(1e10, 1e10);
    if (dp1 <= 0 && dp2 <= 0) return l2b;
    return Point(1e11, 1e11);
  }
  cp1 = crossp(l1b-l1a, l2a-l1a);
  cp2 = crossp(l1b-l1a, l2b-l1a);
  if (cp1 < 0 && cp2 < 0 || cp1 > 0 && cp2 > 0) return Point(1e10, 1e10);
  return (l2a * cp2 - l2b * cp1) / (cp2 - cp1);
}

double PolyArea(const vector<Point>& p) {
  double ret = 0.0;
  for (int i = 0; i < p.size(); i++) ret += crossp(p[i], p[(i+1)%p.size()]);
  return ret/2;
}

Point origin;
bool anglecmp(const Point& a, const Point& b) {
  return atan2(a.y-origin.y, a.x-origin.x) <
         atan2(b.y-origin.y, b.x-origin.x);
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
        Point S, E;
        in >> S.x >> S.y >> E.x >> E.y;
        vector<Point> W1(N), W2(N);
        for (int i = 0; i < N; i++) in >> W1[i].x >> W1[i].y >> W2[i].x >> W2[i].y;
        if (S == E) {cout << 0 << endl; continue;}
        W1.push_back(S);
        W2.push_back(S + Point(37, 2e7));
        W1.push_back(E);
        W2.push_back(E + Point(37, 2e7));
        W1.push_back(Point(-1.5e7, 1e7));
        W2.push_back(Point( 1.5e7, 1e7));
        set<Point> Wa;
        for (int i = 0; i < N; i++) Wa.insert(W1[i]);
        for (int i = 0; i < N; i++) Wa.insert(W2[i]);
        for (set<Point>::iterator it = Wa.begin(); it != Wa.end(); ++it) {
          W1.push_back(*it);
          W2.push_back(*it + Point(37, 2e7));
        }

        map<Point, vector<Point> > m;
        pair<Point, Point> sp, ep;
        for (int i = 0; i < W1.size(); i++) {
          vector<Point> v;
          v.push_back(W1[i]);
          v.push_back(W2[i]);
          for (int j = 0; j < W1.size(); j++) if (j != i) {
            Point p = intersect(W1[i], W2[i], W1[j], W2[j]);
            assert(p.x != 1e11);
            if (p.x != 1e10) v.push_back(p);
          }
          sort(v.begin(), v.end());
          v.erase(unique(v.begin(), v.end()), v.end());
          if (i < N) for (int j = 0; j < v.size(); j++) v[j].cost = 1;
          for (int j = 0; j+1 < v.size(); j++) {
            m[v[j]].push_back(v[j+1]);
            m[v[j+1]].push_back(v[j]);
          }
          if (i == N)   {sp.first = v[0]; sp.second = v[1];}
          if (i == N+1) {ep.first = v[0]; ep.second = v[1];}
        }

        for (map<Point, vector<Point> >::iterator it = m.begin();
            it != m.end(); ++it) {
          origin = it->first;
          sort(it->second.begin(), it->second.end(), anglecmp);
        }

        set<pair<Point, Point> > seen;
        vector<pair<Point, Point> > q;
        q.push_back(sp);
        int ret = 0;
        for(;;) {
          vector<pair<Point, Point> > q2;
          while (!q.empty()) {
            pair<Point, Point> cur = q.back();
            q.pop_back();
            if (cur == ep) goto done;
            if (seen.count(cur)) continue;
            seen.insert(cur);
            pair<Point, Point> next(cur.second, cur.first);
            if (!cur.second.cost) q.push_back(next); else q2.push_back(next);
            const vector<Point>& v = m[cur.second];
            int i;
            for (i = 0; i < v.size(); i++) if (v[i] == cur.first) break;
            assert(i < v.size());
            next.second = v[(i+1)%v.size()];
            q.push_back(next);
          }
          q.swap(q2);
          ret++;
        }

    done:;
        cout << ret << endl;
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

