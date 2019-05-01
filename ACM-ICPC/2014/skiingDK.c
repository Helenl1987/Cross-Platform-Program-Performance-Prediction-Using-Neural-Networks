#include <stdio.h>
#include <stdlib.h>
#include "papi.h"
#include <fstream>


#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <set>
#include <vector>
using namespace std;

int N, Vy;
double Amax, tx[251], ty[251];

void velocity_interval(double x1, double y1, double x2, double y2,
    double sv1, double sv2, double& fv1, double& fv2) {
  if (x1 == x2 && y1 == y2) {fv1 = sv1; fv2 = sv2; return;}
  double d = x2-x1, t = y2-y1;
  sv1 = max(sv1, d/t - 0.5*Amax*t);
  sv2 = min(sv2, d/t + 0.5*Amax*t);
  if (sv1 > sv2) {fv1 = 1e50; fv2 = -1e50; return;}
  if (Amax == 0) {fv1 = sv1; fv2 = sv2; return;}

  // For maximal final velocity, start with min speed, decelerate for time u,
  // then accelerate.
  double a = Amax;
  double b = -2*t*Amax;
  double c = sv1*t + 0.5*Amax*t*t - d;
  double u = (-b - sqrt(max(0.0, b*b - 4*a*c))) / (2*a);
  fv2 = sv1 + Amax * ((t-u) - u);
//cout << sv1 << " -> " << fv2 << ": u = " << u << endl;

  // For minimal final velocity, start with max speed, accelerate for time u,
  // then decelerate.
  a = -Amax;
  b = 2*t*Amax;
  c = sv2*t - 0.5*Amax*t*t - d;
  u = (-b + sqrt(max(0.0, b*b - 4*a*c))) / (2*a);
  fv1 = sv2 + Amax * (u - (t-u));
//cout << sv2 << " -> " << fv1 << ": u = " << u << endl;
}

struct Interval {
  double v1, v2;
  int nt;
  Interval(double v1, double v2, int nt) : v1(v1), v2(v2), nt(nt) {}
  bool operator<(const Interval& i) const {
    if (nt != i.nt) return nt > i.nt;
    return v1 < i.v1;
  }
  friend ostream& operator<<(ostream& out, const Interval& i) {
    out << i.v1 << " to " << i.v2 << " (" << i.nt << ")";
    return out;
  }
};

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

      while (in >> N) {
        in >> Vy >> Amax;
        Amax /= Vy;
        vector<pair<double, int> > yv;
        yv.push_back(make_pair(0, 0));
        vector<vector<Interval> > v(N+1);
        for (int i = 1; i <= N; i++) {
          in >> tx[i] >> ty[i];
          tx[i] /= Vy; ty[i] /= Vy;
          yv.push_back(make_pair(ty[i], i));
          v[i].push_back(Interval(-1e50, 1e50, 1));
        }
        sort(yv.begin(), yv.end());

        for (int yi = N; yi >= 0; yi--) {
          int y = yv[yi].second;

          // Merge intervals.
          sort(v[y].begin(), v[y].end());
          vector<Interval> v2;
          for (int i = 0; i < v[y].size(); i++) {
            if (v2.size() && v[y][i].nt == v2.back().nt
                          && v[y][i].v1 < v2.back().v2) {
              v2.back().v2 = max(v2.back().v2, v[y][i].v2);
            } else {
              v2.push_back(v[y][i]);
            }
          }
          v[y].swap(v2);
          //for (int i = 0; i < v[y].size(); i++) cout << y << ' ' << v[y][i] << endl;

          for (int x = 0; x <= N; x++) {
            if (ty[x] >= ty[y] && !(tx[x] == tx[y] && ty[x] == ty[y] && x < y))
              continue;
            for (int i = 0; i < v[y].size(); i++) {
              Interval& in = v[y][i];

              Interval in2(0.0, 0.0, in.nt+1);
              velocity_interval(tx[x], ty[x], tx[y], ty[y],
                  in.v1, in.v2, in2.v1, in2.v2);
              if (in2.v1 <= in2.v2) v[x].push_back(in2);
            }
          }
        }

        int i, j;
        for (i = 0; i < v[0].size(); i++) {
          if (v[0][i].v1 <= 0.0 && v[0][i].v2 >= 0.0) break;
        }
        if (i == v[0].size()) {
          cout << "Cannot visit any targets" << endl;
        } else {
          double v1 = 0.0, v2 = 0.0, fv1, fv2;
          int t = 0, x = 0, y;
          for (int nt = v[0][i].nt-1; nt > 0; nt--) {
            for (y = 1; y <= N; y++) {
              if (ty[x] >= ty[y] && !(tx[x] == tx[y] && ty[x] == ty[y] && x < y))
                continue;
              velocity_interval(tx[x], ty[x], tx[y], ty[y], v1, v2, fv1, fv2);
              if (fv1 > fv2) continue;
              for (j = 0; j < v[y].size(); j++) {
                if (v[y][j].nt < nt) break;
                if (fv1 <= v[y][j].v2 && fv2 >= v[y][j].v1) break;
              }
              if (j < v[y].size() && v[y][j].nt == nt) {
                cout << y;
                if (nt == 1) cout << endl; else cout << ' ';
                x = y; v1 = fv1; v2 = fv2;
                break;
              }
              assert(y < N);
            }
          }
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


