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

   const Instance &instance() const;

   void setQuiet(bool toggle);
   void writeLp(const char *fname);
   void writeSolution(const char *fname);

   void maxThreads(int value);
   void timeLimit(int maxSeconds);

   void setVarX(int i, int j, int v, int s, double lb, double ub);
   void fixCurrentSolution();
   void unfixSolution();
   int unfixVehicleSolution(int v);

   double solve();
   double objValue() const;
   double relativeGap() const;
   double objLb() const;

   double serviceStartTime(int i, int v, int s) const;

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

   IloNumVarArray m_xSeq;
   IloNumArray m_solXSeq;
};




