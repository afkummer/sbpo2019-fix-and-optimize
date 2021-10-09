#include "FixAndOptimize.h"
#include "Instance.h"
#include "MemUsage.h"
#include "Timer.h"

#include <algorithm>
#include <iostream>
#include <random>

using namespace std;

FixAndOptimize::FixAndOptimize(const Instance &inst): MipModel(inst) {
   buildModel();
   // Create the indicators used in the algorithms.
   for (int i = 0; i < m_inst.numNodes()-1; ++i) {
      for (int v = 0; v < m_inst.numVehicles(); ++v) {
         for (int s = 0; s < m_inst.numSkills(); ++s) {
            if (m_t[i][v][s].getImpl()) {
               m_tSeq.push_back(m_t[i][v][s]);
               m_indT.push_back(make_tuple(i, v, s));
            }
         }
      }
   }

   // Set the object pointers, and fix memory hogs.
   m_indT.shrink_to_fit();
   for (unsigned i = 0; i < m_indT.size(); ++i) {
      m_tSeq[i].setObject(&m_indT[i]);
   }

   cout << "Guided decomposition set to use a static list of service start times.\n";
}

FixAndOptimize::~FixAndOptimize() {
   // Empty.
}

void FixAndOptimize::solve(int seed, double maxSecs, double spTime) {
   mt19937 mtRng(seed);

   fstream csv("results-fixAndOptimize.csv", ios::app);
   if (csv.tellp() == 0) {
      csv <<
      "instance," <<
      "seed," <<
      "method," <<
      "iter," <<
      "time," <<
      "iter_time," <<
      "memusage," <<
      "v1," <<
      "v2," <<
      "nvars," <<
      "single," <<
      "sync," <<
      "pred," <<
      "best_bound," <<
      "rel_gap," <<
      "obj," <<
      "dist," <<
      "tard," <<
      "tmax" <<
      endl;
   }
   int nRelaxed, nSingle, nSync, nPred;

   // Utilities to control TL of matheuristic.
   Timer totalTimer;
   Timer stepTimer;
   totalTimer.start();

   IloNumArray solution(m_env);
   double objValue[2];
   double dist[2];
   double tard[2], tardMax[2];
   std::string methodStr;

   auto extractSolutionInd = [&] {
      dist[1] = computeDist();
      tard[1] = computeTard();
      tardMax[1] = m_cplex.getValue(m_tmax);
      objValue[1] = m_cplex.getObjValue();
   };

   auto cycleSolutionInd = [&] {
      dist[0] = dist[1];
      tard[0] = tard[1];
      tardMax[0] = tardMax[1];
      objValue[0] = objValue[1];
   };

   auto solutionIndGap = [&](const double * ind) {
      return (ind[0] - ind[1]) / (ind[1] + 0.001);
   };

   auto extractAndFix = [&] {
      m_cplex.getValues(solution, m_xSeq);
      for (IloInt i = 0; i < m_xSeq.getSize(); ++i)
         m_xSeq[i].setBounds(solution[i], solution[i]);
   };

   auto relaxVars = [&](int v1, int v2) {
      nRelaxed = nSingle = nSync = nPred = 0;
      for (int i = 0; i < m_inst.numNodes() - 1; ++i) {
         for (int j = 0; j < m_inst.numNodes() - 1; ++j) {
            for (int s = 0; s < m_inst.numSkills(); ++s) {
               if (m_x[i][j][v1][s].getImpl()) {
                  m_x[i][j][v1][s].setBounds(0.0, 1.0);
                  ++nRelaxed;
                  switch(m_inst.nodeSvcType(j)) {
                     case Instance::SvcType::SINGLE:
                        ++nSingle;
                        break;
                     case Instance::SvcType::SIM:
                        ++nSync;
                        break;
                     case Instance::SvcType::PRED:
                        ++nPred;
                        break;
                     case Instance::SvcType::NONE:
                        break;
                  }
               }
               if (m_x[i][j][v2][s].getImpl()) {
                  m_x[i][j][v2][s].setBounds(0.0, 1.0);
                  ++nRelaxed;
                  switch(m_inst.nodeSvcType(j)) {
                     case Instance::SvcType::SINGLE:
                        ++nSingle;
                        break;
                     case Instance::SvcType::SIM:
                        ++nSync;
                        break;
                     case Instance::SvcType::PRED:
                        ++nPred;
                        break;
                     case Instance::SvcType::NONE:
                        break;
                  }
               }
            }
         }
      }
      return nRelaxed;
   };

   // Assumes a fixed warmstart solution.
   stepTimer.start();
   if (!m_cplex.solve()) std::abort(); 
   stepTimer.finish();
   totalTimer.finish();

   csv <<
      m_inst.fileName() << "," <<
      seed << "," <<
      "initial" << "," <<
      0 << "," <<
      totalTimer.elapsed() << "," <<
      stepTimer.elapsed() << "," <<
      memUsage() << "," <<
      -1 << "," <<
      -1 << "," <<
      -1 << "," <<
      -1 << "," <<
      -1 << "," <<
      -1 << "," <<
      m_cplex.getBestObjValue() << "," <<
      m_cplex.getMIPRelativeGap() << "," <<
      m_cplex.getObjValue() << "," <<
      computeDist() << "," <<
      computeTard() << "," <<
      m_cplex.getValue(m_tmax) <<
   endl;

   std::cout << "Initial solution found with cost " << m_cplex.getObjValue() << std::endl;
   const double objInitial = m_cplex.getObjValue();

   // Register the progress of current solution.
   extractSolutionInd();

   // Refresh solution indicators.
   cycleSolutionInd();

   // Set CPLEX optimization parameters.
   m_cplex.setParam(IloCplex::TiLim, spTime);
   m_cplex.setParam(IloCplex::Threads, 1);
   m_cplex.setParam(IloCplex::ParallelMode, CPX_PARALLEL_DETERMINISTIC);
   {
      std::uniform_int_distribution <int> cplexDistr(0, std::numeric_limits<int>::max()-1);
      m_cplex.setParam(IloCplex::RandomSeed, cplexDistr(mtRng));
      std::cout << "Seeding CPLEX with " << m_cplex.getParam(IloCplex::RandomSeed) << std::endl;
   }

   // Random distribution objects.
   std::uniform_int_distribution <int> distrVehicles(0, m_inst.numVehicles() - 1);
   std::uniform_int_distribution <int> distrMethod(0, 1); // 0: random, 1: by tardiness

   // Control variables to stopping criteria.
   int iterWoImprove = 0;
   auto stopCriteriaReached = [&] () {
      if (maxSecs > 0) {
         return totalTimer.elapsed() >= maxSecs;
      } else {
         int tol = ceil(m_inst.numNodes()/2);
         if (iterWoImprove >= tol)
            return true;
         return false;
      }
   };

   int iter = 1;
   for (; ; ++iter) {

      std::cout << "\n**************************\n";
      std::cout << "* Starting iteration " << setw(3) << iter << " *" << std::endl;
      std::cout << "**************************\n";

      int v1 = -1;
      int v2 = -1;
      const int method = distrMethod(mtRng);
      if (method == 0) {
         methodStr = "random";
         v1 = distrVehicles(mtRng);
         v2 = distrVehicles(mtRng);
         while (v2 == v1) {
            v2 = distrVehicles(mtRng);
         }
      } else if (method == 1) {
         methodStr = "guided";
         m_tSeq.clear();
         for (int v = 0; v < m_inst.numVehicles(); ++v) {
            for (auto [i,s]: extractRoute(v)) {
               m_tSeq.push_back(m_t[i][v][s]);
            }
         }

         // Sort by nonincreasing tardiness.
         std::sort(m_tSeq.rbegin(), m_tSeq.rend(), [&](auto &a, auto &b) {
            return m_cplex.getValue(a) < m_cplex.getValue(b);
         });

         std::uniform_int_distribution <int> chooser(0, std::floor(m_tSeq.size() / 2.)); // Choose from first 50% worst tardiness.
         int sel = chooser(mtRng);
         auto tp1 = *static_cast<tuple<int,int,int>*>(m_tSeq[sel].getObject());
         v1 = get<1>(tp1);

         do {
            sel = chooser(mtRng);
            auto tp2 = *static_cast<tuple<int,int,int>*>(m_tSeq[sel].getObject());
            v2 = get<1>(tp2);
         } while (v2 == v1);

      } else {
         std::cout << "Unknown relaxation method: " << method << ".\n"; std::abort();
      }

      // Check if two vehicles have been chosen.
      if (v1 == -1) std::abort();
      if (v2 == -1) std::abort();

//       printRouteInd(v1);
//       printRouteInd(v2);
      extractAndFix();

      int nvars = relaxVars(v1, v2);
      std::cout << "Relaxation method chosen: " << methodStr << std::endl;
      std::cout << "Relaxing " << nvars << " variables from v1 = " << v1 << " and v2 = " << v2 << std::endl;

      stepTimer.start();
      if (!m_cplex.solve()) std::abort();
      stepTimer.finish();
      totalTimer.finish();

      extractSolutionInd();

      std::cout << "Progress on iteration " << iter << ":\n";
      std::cout << "Current obj value: " << objValue[1] << std::endl;
      std::cout << "Time elapsed:      " << totalTimer.elapsed() << std::endl;
      std::cout << "Improvement on obj:            " << solutionIndGap(objValue) * 100 << "%" << std::endl;
      std::cout << "Improvement on dist:           " << solutionIndGap(dist) * 100 << "%" << std::endl;
      std::cout << "Improvement on tardiness:      " << solutionIndGap(tard) * 100 << "%" << std::endl;
      std::cout << "Improvement on max tardiness : " << solutionIndGap(tardMax) * 100 << "%" << std::endl;
      std::cout << "IterWoImprove:                 " << iterWoImprove << std::endl;
      std::cout << "Reset criteria:                " << (solutionIndGap(objValue)*100 > 0.01) << std::endl;

      csv <<
         m_inst.fileName() << "," <<
         seed << "," <<
         methodStr << "," <<
         iter << "," <<
         totalTimer.elapsed() << "," <<
         stepTimer.elapsed() << "," <<
         memUsage() << "," <<
         v1 << "," <<
         v2 << "," <<
         nRelaxed << "," <<
         nSingle << "," <<
         nSync << "," <<
         nPred<< "," <<
         m_cplex.getBestObjValue() << "," <<
         m_cplex.getMIPRelativeGap() << "," <<
         m_cplex.getObjValue() << "," <<
         computeDist() << "," <<
         computeTard() << "," <<
         m_cplex.getValue(m_tmax) <<
      endl;

      if (solutionIndGap(objValue)*100 > 0.01) {
         iterWoImprove = 0;
      } else {
         ++iterWoImprove;
         if (stopCriteriaReached()) {
            std::cout << "Stop criteria reached after " << iter << " iterations (" << totalTimer.elapsed() << " s.)" << std::endl;
            break;
         }
      }

      cycleSolutionInd();
   }

   std::cout << "Finishing matheuristic.\n";

   totalTimer.finish();

   std::cout << "Final obj = " << m_cplex.getObjValue() << ".\n";
   std::cout << "Time spent on solver: " << totalTimer.elapsed() << ".\n";

   m_iter = iter;
   m_elapsedTime = totalTimer.elapsed();
   m_objInitial = objInitial;
   solution.end();
}




