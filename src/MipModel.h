#pragma once

#include "Instance.h"

#define IL_STD
#include <ilcplex/ilocplex.h>

class MipModel {
public:
   /**
    * Lambda values from obj function.
    * L1: Distance weight
    * L2: Tardiness weight
    * L3: Maximum tardiness weight
    */
   constexpr const static double L1 = 1./3.;
   constexpr const static double L2 = 1./3.;
   constexpr const static double L3 = 1./3.;

   /** Shortcuts to define multi-dimensional variables. */
   using Var1D = IloArray <IloNumVar>;
   using Var2D = IloArray <Var1D>;
   using Var3D = IloArray <Var2D>;
   using Var4D = IloArray <Var3D>;


   MipModel(const Instance &inst);
   virtual ~MipModel();

   void writeLp(const char *fname);
   void writeSolution(const char *fname);

   void maxThreads(int value);
   void timeLimit(int maxSeconds);

   void setVarX(int i, int j, int v, int s, double lb, double ub);
   void unfixSolution();

   double solve();
   double objValue() const;
   double relativeGap() const;
   double objLb() const;

protected:
   const Instance &m_inst;

   IloEnv m_env;
   IloModel m_model;
   IloCplex m_cplex;
   IloObjective m_obj;

   IloNumVar m_Tmax;

   Var2D m_z;
   Var3D m_t;
   Var4D m_x;
};




