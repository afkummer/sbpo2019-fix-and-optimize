/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019
 * Alberto Francisco Kummer Neto (afkneto@inf.ufrgs.br),
 * Luciana Salete Buriol (buriol@inf.ufrgs.br) and
 * Olinto César Bassi de Araújo (olinto@ctism.ufsm.br)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "FixAndOptimize.h"
#include "Timer.h"

#include <algorithm>
#include <iostream>


using namespace std;


FixAndOptimize::FixAndOptimize(MipModel& model): m_inst(model.instance()), m_model(model) {
   // Empty
}

FixAndOptimize::~FixAndOptimize() {
   // Empty
}

void FixAndOptimize::solve(const int seed, const int maxIterNoImpr, const int maxIterSeconds) {
   // Initialize the PRGN using the seed.
   m_prng.seed(seed);

   Timer timer;
   timer.start();
   double timeBest = 0.0;

   m_model.timeLimit(maxIterSeconds);
   int itersWoImpr = 0;
   double currentObj = m_model.objValue();

   for (int iter = 1;; ++iter) {
      chooseDecomp();
      selectDecompVehicles();

      m_model.fixCurrentSolution();

      m_model.unfixVehicleSolution(m_vehiDecomp[0]);
      m_model.unfixVehicleSolution(m_vehiDecomp[1]);

      double newObj = m_model.solve();

      timer.finish();
      cout << "Iteration: " << iter << "  Decomp: " << m_currentDecompName <<
         "  Elapsed: " << fixed << setprecision(1) << timer.elapsed() << " secs  Obj: " << newObj << "  Improved: " <<
         (currentObj/newObj - 1.0) * 100 << "%  IWoI: " << itersWoImpr << endl;

      if (currentObj - newObj > 0.5) {
         itersWoImpr = 0;
         timeBest = timer.elapsed();
      } else {
         ++itersWoImpr;
         if (itersWoImpr >= maxIterNoImpr) {
            break;
         }
      }

      currentObj = newObj;
   }

   timer.finish();

   m_model.fixCurrentSolution();
   m_model.solve();

   // Registers the solution into a CSV file.
   ofstream csv("results-fixAndOptimize.csv", ios::out | ios::app);

   // Write the header if the file is created in this run.
   if (csv.tellp() == 0) {
      csv <<
         "instance," <<
         "seed," <<
         "time.best," <<
         "time.total," <<
         "cost" <<
      endl;
   }

   csv <<
      m_inst.fileName() << "," <<
      seed << "," <<
      timeBest << "," <<
      timer.elapsed() << "," <<
      m_model.objValue() <<
   endl;
}

void FixAndOptimize::chooseDecomp() {
   uniform_int_distribution <int> decDistr(0, (int) DecompMethod::MAX_-1);
   m_currentDecomp = (DecompMethod) decDistr(m_prng);
}

void FixAndOptimize::selectDecompVehicles() {
   if (m_currentDecomp == DecompMethod::RANDOM) {
      m_currentDecompName = "random";
      uniform_int_distribution <int> vdistr(0, m_inst.numVehicles()-1);

      m_vehiDecomp[0] = vdistr(m_prng);
      do {
         m_vehiDecomp[1] = vdistr(m_prng);
      } while (m_vehiDecomp[0] == m_vehiDecomp[1]);

   } else {
      m_currentDecompName = "guided";

      // Get the service start time for each service and service type requested.
      // 0: node
      // 1: vehicle
      // 2: skill
      // 3: service start time
      vector <tuple <int,int,int, double>> startTimes;
      for (int i = 1; i < m_inst.numNodes()-1; ++i) {
         for (int v = 0; v < m_inst.numVehicles(); ++v) {
            for (int s = 0; s < m_inst.numSkills(); ++s) {
               double st = m_model.serviceStartTime(i, v, s);
               if (st != numeric_limits<double>::infinity()) {
                  startTimes.push_back(make_tuple(i, v, s, st));
               }
            }
         }
      }

      // Sort the start times by non-increasing order.
      sort(startTimes.rbegin(), startTimes.rend(), [] (const auto &a, const auto &b) {
         return get<3>(a) < get<3>(b);
      });

      // Choose two vehicles from the 50% largest start times.
      int cutoff = int(floor(int(startTimes.size()) / 2.0));
      uniform_int_distribution <unsigned> chooser(0, cutoff);

      m_vehiDecomp[0] = get<1>(startTimes[chooser(m_prng)]);
      do {
         m_vehiDecomp[1] = get<1>(startTimes[chooser(m_prng)]);
      } while (m_vehiDecomp[0] == m_vehiDecomp[1]);
   }
}

