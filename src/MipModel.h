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
   using Var2D = IloArray <IloArray <IloNumVar>>;
   using Var3D = IloArray <Var2D>;
   using Var4D = IloArray <Var3D>;


   MipModel(const Instance &inst);
   virtual ~MipModel();

   void writeLp(const char *fname);

//    void writeSolution() {
//       m_cplex.writeSolutions("solution.sol");
//    }

//    void setXBounds(int i, int j, int v, int s, double lb = 0.0, double ub = 1.0) {
//       m_x[i][j][v][s].setBounds(lb, ub);
//    }

//    void maxThreads(int value);
//    void timeLimit(int maxSeconds);

//    void resetBoundsX();

//    void extractInitialSolution(const InitialRouting &ini);

//    void fixAndOptimize(std::mt19937 &mtRng, const int maxSecs, const int maxIterSecs);
//    void fixAndOptimize2(int maxTimeIter, int maxIter, unsigned seed);
//    void fixAndOptimize3(int maxTimeIter, int maxIter, unsigned seed);

//    void redirectCplexOutput(const std::string fname = "cplex.out");

   // Carrega uma solução para as variáveis de decisão do modelo.
   // Serve para promover warm start ao solver.
//    void loadSolution(const Solution &sol);

//    void solve();
//    double bestObjValue() const;
//    double gap() const; // Relative, not percentual.
//    double lb() const;

//    double solverSeconds() const;

//    int numCols() const;
//    int numRows() const;
//    long numNonZeros() const;

//    bool isAssigned(int caregiver, int vehicle) const;
//    double svcStartTime(int vehicle, int node, int skill) const;
//    void printCostSummary() const;

//    std::vector <std::tuple<int,int>> extractRoute(int v) const;
//    std::vector<std::vector <std::tuple<int,int>>> extractRoutes() const;
//    void printCsvRow() const;

//    void relaxX();

protected:
   const Instance &m_inst;

   IloEnv m_env;
//    std::ofstream m_outRedir;
   IloModel m_model;
   IloCplex m_cplex;
   IloObjective m_obj;

   IloNumVar m_Tmax;

   Var2D m_z;
   Var3D m_t;
   Var4D m_x;

//    IloNumVarArray m_xSeq;


//    Timer m_solverTimer;
//    long m_iter;
//    double m_elapsedTime;
//    double m_objInitial;
};




