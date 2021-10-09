#pragma once

#include "MipModel.h"

class FixAndOptimize: public MipModel {
public:
   FixAndOptimize(const Instance &inst);
   virtual ~FixAndOptimize();

   // Supposes that the MIP model has a initial solution
   // fixed by binding the decision variables x.
   void solve(int seed, double maxTime, double spTime);

protected:
   std::vector <IloNumVar> m_tSeq;
   std::vector <std::tuple <int, int, int>> m_indT;

   int m_iter {0};
   double m_elapsedTime {0.0};
   double m_objInitial {0.0};
};
