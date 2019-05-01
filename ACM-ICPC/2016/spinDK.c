#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>
using namespace std;

#define NUM_EVENTS 1
#define THRESHOLD 10000
#define ERROR_RETURN(retval) { fprintf(stderr, "Error %d %s:line %d: \n", retval,__FILE__,__LINE__);  exit(retval); }



struct Point {
  long long x, y;

  Point(int x = 0, int y = 0) : x(x), y(y) {}
  Point operator-(const Point& p) const {return Point(x-p.x, y-p.y);}
  bool operator==(const Point& p) const {return x == p.x && y == p.y;}
  
  Point slope() const {
    return (x < 0 || x == 0 && y < 0) ? Point(-x, -y) : Point(x, y);
  }
};

long long crossp(const Point& a, const Point& b) {
  return (long long)a.x*b.y - (long long)a.y*b.x;
}
long long dotp(const Point& a, const Point& b) {
  return (long long)a.x*b.x + (long long)a.y*b.y;
}


bool xcmp(const Point& a, const Point& b) {
  return a.x < b.x || a.x == b.x && a.y < b.y;
}

bool slopecmp(const Point& a, const Point& b) {
  return (long long)a.y*b.x < (long long)b.y*a.x;
}

bool eventcmp(const pair<Point, int>& a, const pair<Point, int>& b) {
  if (slopecmp(a.first, b.first)) return true;
  if (slopecmp(b.first, a.first)) return false;
  return a.second < b.second;
}


vector<Point> ConvexHull(vector<Point> p) {
  sort(p.begin(), p.end(), xcmp);

  vector<Point> top(1, p[0]), bot(1, p[0]);
  for (int i = 1; i < p.size(); i++) {
    if (p[i] == p[i-1]) continue;
    while (top.size() > 1 && crossp(p[i] - top[top.size()-2],
                                    top.back() - top[top.size()-2]) <= 0) {
      top.pop_back();
    }
    while (bot.size() > 1 && crossp(p[i] - bot[bot.size()-2],
                                    bot.back() - bot[bot.size()-2]) >= 0) {
      bot.pop_back();
    }
    top.push_back(p[i]);
    bot.push_back(p[i]);
  }

  for (int i = top.size()-2; i >= 1; i--) bot.push_back(top[i]);
  return bot;
}

bool InsideHull(const vector<Point>& hull, const Point& p,
                int& first, int& last) {
  first = last = 0;
  if (hull.size() == 1) return p == hull[0];
  int base = 0;
  if (crossp(hull[0]-p, hull[1]-p) == 0) {
    if (hull.size() == 2) {
      return dotp(p-hull[0], hull[1]-hull[0]) >= 0 &&
             dotp(p-hull[1], hull[0]-hull[1]) >= 0;
    }
    if (p == hull[1]) return true;
    base++;
  }

  int H = hull.size();
  int sgn = (crossp(hull[base%H]-p, hull[(base+1)%H]-p) > 0) ? 1 : -1;
  assert(crossp(hull[base%H]-p, hull[(base+1)%H]-p) != 0);

  // base->base+1 is strictly up/down.  Find first strictly down/up edge.
  // Iff the point's outside the hull, there must be one.
  int lo = base+1, hi = base + hull.size()-1;
  while (hi > lo) {
    int mid = (lo+hi)/2;
    if (crossp(hull[mid%H]-p, hull[(mid+1)%H]-p) * sgn < 0) {
      hi = mid;  // On downward/upward curve.
    } else if (crossp(hull[mid%H]-p, hull[(base+1)%H]-p) * sgn > 0) {
      hi = mid;  // On second upward/downward curve.
    } else {
      lo = mid+1;
    }
  }
  if (crossp(hull[lo%H]-p, hull[(lo+1)%H]-p) * sgn >= 0) return true;

  (sgn > 0 ? last : first) = base = lo%H;
  sgn = -sgn;

  lo = base+1, hi = base + hull.size()-1;
  while (hi > lo) {
    int mid = (lo+hi)/2;
    if (crossp(hull[mid%H]-p, hull[(mid+1)%H]-p) * sgn < 0) {
      hi = mid;  // On upward/downward curve.
    } else if (crossp(hull[mid%H]-p, hull[(base+1)%H]-p) * sgn > 0) {
      hi = mid;  // On second downward/upward curve.
    } else {
      lo = mid+1;
    }
  }

  (sgn > 0 ? last : first) = lo%H;
  return false;
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


  int N, a, b, c;
  while (in >> N) {
    vector<Point> Pyes, Pno;
    for (int i = 0; i < N; i++) {
      in >> a >> b >> c;
      (c ? Pyes : Pno).push_back(Point(a, b));
    }

  if (Pyes.size() == 1) {
    cout << 1 << endl;
    continue;
  }

    vector<Point> hull = ConvexHull(Pyes);

    int cur = 0;
    vector<pair<Point, int> > events;
    for (int i = 0; i < Pno.size(); i++) {
      int first = 0, last = 0;
      if (InsideHull(hull, Pno[i], first, last)) {
//cerr << Pno[i].x << ',' << Pno[i].y << " inside" << endl;
        continue;
      }
      events.push_back(make_pair((hull[first]-Pno[i]).slope(), -1));
      events.push_back(make_pair((hull[last]-Pno[i]).slope(), 1));
      if (!slopecmp(events[events.size()-1].first,
                    events[events.size()-2].first)) cur++;
    }

    int best = cur;
    sort(events.begin(), events.end(), eventcmp);
    for (int i = 0; i < events.size(); i++) {
//cerr << events[i].first.x << ',' << events[i].first.y << ' '
//     << events[i].second << endl;
      cur += events[i].second;
      if (0 && cur >= best) {
        int j = (i+1)%events.size();
        cerr << "best=" << N-best
             << " S=" << events[i].first.y + events[j].first.y
             << " T=" << -events[i].first.x - events[j].first.x << endl;
      }
      best = max(best, cur);
    }
    cout << N-best << endl;


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
