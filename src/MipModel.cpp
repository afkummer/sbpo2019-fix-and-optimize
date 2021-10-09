#include "MipModel.h"

#include "Instance.h"
#include "MemUsage.h"
#include "Timer.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>

using namespace std;

MipModel::MipModel(const Instance &inst): m_inst(inst) {
   // Basic CPLEX variables.
   m_model = IloModel(m_env);
   m_cplex = IloCplex(m_model);
   m_expr = IloExpr(m_env);
   m_model.setName("HHCRSP");
}

MipModel::~MipModel() {
   m_env.end();
}

void MipModel::buildModel() {
   createVarsAndObj();
   createAllConstraints();
}

void MipModel::setQuiet(bool toggle) {
   m_cplex.setOut(toggle ? m_env.getNullStream() : m_env.out());
}

void MipModel::writeLp(const char *fname) {
   m_cplex.exportModel(fname);
}

void MipModel::writeSolution(const std::string &fname) const {
   m_cplex.writeSolution(fname.c_str());
}

int MipModel::numCols() const {
   return m_cplex.getNcols();
}

int MipModel::numRows() const {
   return m_cplex.getNrows();
}

long MipModel::numNonZeros() const {
   return m_cplex.getNNZs64();
}

void MipModel::maxThreads(int value) {
   m_cplex.setParam(IloCplex::Threads, value);
}

void MipModel::timeLimit(int maxSeconds) {
   m_cplex.setParam(IloCplex::TiLim, maxSeconds);
}

void MipModel::solve() {
   Timer tm;
   tm.start();

   if (!m_cplex.solve()) {
      std::cout << "MIP model is infeasible." << std::endl;
      std::cout << "Problematic model written to infeasible.lp." << std::endl;
      writeLp("infeasible.lp");
      std::abort();
   }

   tm.finish();
   m_solveTime = tm.elapsed();
}

double MipModel::solveSeconds() const {
   return m_solveTime;
}

void MipModel::printCostSummary() const {
   cout << "\nSolution status from MIP model:\n";
   cout << "-- Processing time: " << m_solveTime << " sec.\n";
   cout << "-- Memory used: " << memUsage() << " MB\n";
   cout << "-- Best bound: " << m_cplex.getBestObjValue() << "\n";
   cout << "-- Obj value: " << m_cplex.getObjValue() << "\n";
   cout << "-- Gap: " << m_cplex.getMIPRelativeGap()*100 << "%\n";
   cout << "-- Total distance: " << computeDist() << "\n";
   cout << "-- Total tardiness: " << computeTard()<< "\n";
   cout << "-- Largest tardiness: " << m_cplex.getValue(m_tmax) << endl;
}

double MipModel::objValue() const {
   // Following instructions of https://www.ibm.com/support/knowledgecenter/SSSA5P_12.6.2/ilog.odms.ide.help/refcppopl/html/classes/IloCplex.html
   return m_cplex.getObjValue();
}

double MipModel::gap() const {
   // Following instructions of https://www.ibm.com/support/knowledgecenter/SSSA5P_12.6.2/ilog.odms.ide.help/refcppopl/html/classes/IloCplex.html
   return m_cplex.getMIPRelativeGap();
}

double MipModel::lb() const {
   // Following instructions of https://www.ibm.com/support/knowledgecenter/SSSA5P_12.6.2/ilog.odms.ide.help/refcppopl/html/classes/IloCplex.html
   return m_cplex.getBestObjValue();
}

double MipModel::computeDist() const {
   double dist = 0.0;
   for (int i = 0; i < m_inst.numNodes() - 1; ++i) {
      for (int j = 0; j < m_inst.numNodes() - 1; ++j) {
         for (int v = 0; v < m_inst.numVehicles(); ++v) {
            for (int s = 0; s < m_inst.numSkills(); ++s) {
               if (m_x[i][j][v][s].getImpl()) {
                  dist += m_inst.distance(i,j) * m_cplex.getValue(m_x[i][j][v][s]);
               }
            }
         }
      }
   }
   return dist;
}

double MipModel::computeTard() const {
   double tard = 0.0;
   for (int i = 1; i < m_inst.numNodes() - 1; ++i) {
      for (int s = 0; s < m_inst.numSkills(); ++s) {
         if (m_z[i-1][s].getImpl()) {
            tard += m_cplex.getValue(m_z[i-1][s]);
         }
      }
   }
   return tard;
}

double MipModel::tmax() const {
   return m_cplex.getValue(m_tmax);
}

void MipModel::setBoundX(int i, int j, int v, int s, double lb, double ub) {
   if (m_x[i][j][v][s].getImpl())
      m_x[i][j][v][s].setBounds(lb, ub);
}

void MipModel::resetBoundsX() {
   for (IloInt i = 0; i < m_xSeq.getSize(); ++i) {
      m_xSeq[i].setBounds(0.0, 1.0);
   }
}

tuple <int,double> MipModel::svcStartTime(int i, int s) {
   tuple <int, double> res = make_tuple(-1, IloInfinity);
   for (int j = 0; j < m_inst.numNodes(); ++j) {
      for (int v = 0; v < m_inst.numVehicles(); ++v) {
         if (!m_x[j][i][v][s].getImpl())
            continue;
         if (m_cplex.getValue(m_x[j][i][v][s]) >= 0.5) {
            res = make_tuple(v, m_cplex.getValue(m_t[i][v][s]));
            goto finish;
         }
      }
   }
finish:
   return res;
}

double MipModel::assignment(int i, int j, int v, int s) {
   if (!m_x[i][j][v][s].getImpl())
      return 0.0;
   return m_cplex.getValue(m_x[i][j][v][s]);
}

std::vector <std::tuple<int, int>> MipModel::extractRoute(int v) const {
   std::vector <std::tuple<int, int>> route;
   route.push_back(std::make_tuple(0, 0)); // Push depot on route
   for (;;) {
      int next = -1;
      int skill = -1;
      for (int j = 0; j < m_inst.numNodes(); ++j) {
         for (int s = 0; s < m_inst.numSkills(); ++s) {
            IloNumVar x = m_x[std::get<0>(route.back())][j][v][s];
            if (x.getImpl() && m_cplex.getValue(x) > 0.5) {
               next = j;
               skill = s;
               goto endloop;
            }
         }
      }
   endloop:
      if (next == -1) std::abort();
      route.push_back(std::make_tuple(next, skill));
      if (next == 0)
         break; // End-of-route reached.
   }
   return route;
}

void MipModel::loadRoutes(const std::vector<std::vector<std::tuple<int, int>>> &r) {
   int vid = 0;
   for (auto &vr: r) {
      for (size_t i = 1; i < vr.size(); ++i) {
         setBoundX(get<0>(vr[i - 1]), get<0>(vr[i]), vid, get<1>(vr[i]), 1.0, 1.0);
      }
      ++vid;
   }
}

void MipModel::createVarsAndObj() {
   // Helper objects
   IloExpr expr(m_env);
   char buf[64];

   // Create the sequential arrays.
   m_xSeq = IloNumVarArray(m_env);

   // Create decision variables x.
   int saving = 0;
   m_x.resize(boost::extents[m_inst.numNodes()-1][m_inst.numNodes()-1][m_inst.numVehicles()][m_inst.numSkills()]);
   for (int i = 0; i < m_inst.numNodes() - 1; ++i) {
      for (int j = 0; j < m_inst.numNodes() - 1; ++j) {
         for (int v = 0; v < m_inst.numVehicles(); ++v) {
            for (int s = 0; s < m_inst.numSkills(); ++s) {
                  if (m_inst.nodeReqSkill(j, s) == 0)
                     continue;
                  if (m_inst.vehicleHasSkill(v, s) == 0)
                     continue;

                  if (i != 0 && i == j) {
                     saving++;
                     continue;
                  }

                  if (!isFlowOk(j, v)) {
                     saving++;
                     continue;
                  }

                  if (!isFlowOk(i, v)) {
                     saving++;
                     continue;
                  }

                  snprintf(buf, sizeof buf, "x#%d#%d#%d#%d", i, j, v, s);
                  IloNumVar x(m_env, 0.0, 1.0, IloNumVar::Bool, buf);

                  m_x[i][j][v][s] = x;
                  m_xSeq.add(x);

                  expr += COEFS[0] * m_inst.distance(i, j) * x;
            }
         }
      }
   }
   cout << "Trivial flow violations prevented generating " << saving << " variables." << endl;

   // Create variables z.
   m_z.resize(boost::extents[m_inst.numNodes()-2][m_inst.numSkills()]);
   for (int i = 1; i < m_inst.numNodes() - 1; ++i) {
      for (int s = 0; s < m_inst.numSkills(); ++s) {
         if (m_inst.nodeReqSkill(i, s) == 0)
            continue;

         snprintf(buf, sizeof buf, "z#%d#%d", i, s);
         IloNumVar z(m_env, 0.0, IloInfinity, IloNumVar::Float, buf);
         m_z[i-1][s] = z;

         expr += COEFS[1] * z;
      }
   }

   // Create variables t.
   m_t.resize(boost::extents[m_inst.numNodes()-1][m_inst.numVehicles()][m_inst.numSkills()]);
   for (int i = 0; i < m_inst.numNodes() - 1; ++i) {
      for (int v = 0; v < m_inst.numVehicles(); ++v) {
         for (int s = 0; s < m_inst.numSkills(); ++s) {
            if (i != 0) {
               if (!m_inst.nodeReqSkill(i, s))
                  continue;
               if (m_inst.vehicleHasSkill(v, s) == 0)
                  continue;
            }
            snprintf(buf, sizeof buf, "t#%d#%d#%d", i, v, s);
            IloNumVar t = IloNumVar(m_env, 0, IloInfinity, IloNumVar::Float, buf);
            m_t[i][v][s] = t;
         }
      }
   }

   // Create max tardiness variable.
   m_tmax = IloNumVar(m_env, 0., IloInfinity, IloNumVar::Float, "tmax");
   expr += COEFS[2] * m_tmax;

   // Create the objective function.
   m_obj = IloObjective(m_env, expr, IloObjective::Minimize, "routingCost");
   m_model.add(m_obj);
   expr.clear();

   // Housekeeping.
   expr.end();
}

void MipModel::createAllConstraints() {
   // Auxiliar objects.
   char buf[128];

   // Create (4) greatest tardiness constraints.
   for (int i = 1; i < m_inst.numNodes() - 1; ++i) {
      for (int s = 0; s < m_inst.numSkills(); ++s) {
         if (!m_z[i-1][s].getImpl())
            continue;

         IloConstraint c = m_tmax - m_z[i - 1][s] >= 0;
         snprintf(buf, sizeof buf, "tmax#%d#%d", i, s);
         c.setName(buf);

         m_model.add(c);
      }
   }

   // Create (5-1) flow on source node.
   for (int v = 0; v < m_inst.numVehicles(); ++v) {
      for (int i = 0; i < m_inst.numNodes() - 1; ++i) {
         for (int s = 0; s < m_inst.numSkills(); ++s) {
            if (m_x[0][i][v][s].getImpl())
               m_expr += m_x[0][i][v][s];
         }
      }

      IloConstraint c = m_expr == 1;
      snprintf(buf, sizeof buf, "source#%d", v);
      c.setName(buf);

      m_model.add(c);
      m_expr.clear();
   }

   // Create (5-2) flow on sink node.
   for (int v = 0; v < m_inst.numVehicles(); ++v) {
      for (int i = 0; i < m_inst.numNodes() - 1; ++i) {
         for (int s = 0; s < m_inst.numSkills(); ++s) {
            if (m_x[i][0][v][s].getImpl())
               m_expr += m_x[i][0][v][s];
         }
      }

      IloConstraint c = m_expr == 1;
      snprintf(buf, 128, "sink#%d", v);
      c.setName(buf);

      m_model.add(c);
      m_expr.clear();
   }

   // Create (6) flow conservation constraints.
   for (int i = 1; i < m_inst.numNodes() - 1; ++i) {
      for (int v = 0; v < m_inst.numVehicles(); ++v) {

         bool hasIn = false, hasOut = false;
         for (int j = 0; j < m_inst.numNodes() - 1; ++j) {
            for (int s = 0; s < m_inst.numSkills(); ++s) {
               if (auto &x = m_x[j][i][v][s]; x.getImpl()) {
                  m_expr += x;
                  hasIn = true;
               }

               if (auto &x = m_x[i][j][v][s]; x.getImpl()) {
                  m_expr -= x;
                  hasOut = true;
               }
            }
         }

         if (hasIn && hasOut) {
            IloConstraint c = m_expr == 0;
            snprintf(buf, sizeof buf, "flow#%d#%d", i, v);
            c.setName(buf);

            m_model.add(c);
         }
         m_expr.clear();
      }
   }

   // Create (7) patient assignment constraints.
   for (int i = 1; i < m_inst.numNodes() - 1; ++i) {
      for (int s = 0; s < m_inst.numSkills(); ++s) {
         if (!m_inst.nodeReqSkill(i,s))
            continue;

         for (int v = 0; v < m_inst.numVehicles(); ++v) {
            for (int j = 0; j < m_inst.numNodes() - 1; ++j) {
               if (m_x[j][i][v][s].getImpl())
                  m_expr += m_x[j][i][v][s];
            }
         }

         IloConstraint c = m_expr == 1;
         snprintf(buf, sizeof buf, "assignment#%d#%d", i, s);
         c.setName(buf);

         m_model.add(c);
         m_expr.clear();
      }
   }

   // Create (8) subcycle elimination constraints.
   for (int i = 0; i < m_inst.numNodes() - 1; ++i) {
      for (int j = 1; j < m_inst.numNodes() - 1; ++j) {
         for (int v = 0; v < m_inst.numVehicles(); ++v) {
            for (int s1 = 0; s1 < m_inst.numSkills(); ++s1) {
               for (int s2 = 0; s2 < m_inst.numSkills(); ++s2) {

                  if (m_x[i][j][v][s2].getImpl() == nullptr)
                     continue;

                  if (m_t[i][v][s1].getImpl() == nullptr)
                     continue;

                  if (m_t[j][v][s2].getImpl() == nullptr)
                     continue;

                  const double bigM = 5e4;

                  m_expr += m_t[i][v][s1];
                  m_expr += m_inst.nodeProcTime(i, s1);
                  m_expr += m_inst.distance(i, j);
                  m_expr -= m_t[j][v][s2];
                  m_expr -= bigM;
                  m_expr += bigM * m_x[i][j][v][s2];

                  IloConstraint c = m_expr <= 0;
                  snprintf(buf, sizeof buf, "subtour#%d#%d#%d#%d#%d", i, j, v, s1, s2);
                  c.setName(buf);

                  m_model.add(c);
                  m_expr.clear();
               }
            }
         }
      }
   }

   // Create (9) start time window constraints.
   for (int i = 1, cc = 1; i < m_inst.numNodes() - 1; ++i) {
      for (int v = 0; v < m_inst.numVehicles(); ++v) {
         for (int s = 0; s < m_inst.numSkills(); ++s, ++cc) {
            if (m_t[i][v][s].getImpl() == nullptr)
               continue;

            // Sets directly the bounds of variables.
            m_t[i][v][s].setLB(m_inst.nodeTwMin(i));
         }
      }
   }

   // Create (10) end time window constraints.
   for (int i = 1; i < m_inst.numNodes() - 1; ++i) {
      for (int v = 0; v < m_inst.numVehicles(); ++v) {
         for (int s = 0; s < m_inst.numSkills(); ++s) {

            if (m_z[i-1][s].getImpl() == nullptr)
               continue;

            if (m_t[i][v][s].getImpl() == nullptr)
               continue;

            IloConstraint c = m_t[i][v][s] - m_z[i-1][s] <= m_inst.nodeTwMax(i);
            snprintf(buf, sizeof buf, "tw_end#%d#%d#%d", i, v, s);
            c.setName(buf);
            m_model.add(c);
         }
      }
   }

   #if 1
   // Create (11) first double service constraint.
   for (int i = 1; i < m_inst.numNodes() - 1; ++i) {
      if (m_inst.nodeSvcType(i) == Instance::SINGLE)
         continue;

      for (int v1 = 0; v1 < m_inst.numVehicles(); ++v1) {
         for (int v2 = 0; v2 < m_inst.numVehicles(); ++v2) {
            for (int s2 = 0; s2 < m_inst.numSkills(); ++s2) {
               for (int s1 = 0; s1 < s2; ++s1) {

                  if (!m_t[i][v2][s2].getImpl())
                     continue;

                  if (!m_t[i][v1][s1].getImpl())
                     continue;

                  const double bigM = 5e4;

                  m_expr += m_t[i][v2][s2];
                  m_expr -= m_t[i][v1][s1];
                  m_expr -= m_inst.nodeDeltaMin(i);
                  m_expr += 2.0 * bigM;

                  for (int j = 0; j < m_inst.numNodes() - 1; ++j) {
                     if (m_x[j][i][v1][s1].getImpl())
                        m_expr -= bigM * m_x[j][i][v1][s1];

                     if (m_x[j][i][v2][s2].getImpl())
                        m_expr -= bigM * m_x[j][i][v2][s2];
                  }

                  IloConstraint c = m_expr >= 0;
                  std::snprintf(buf, sizeof buf, "sync_dmin#%d#%d#%d#%d#%d", i, v1, v2, s1, s2);
                  c.setName(buf);

                  m_model.add(c);
                  m_expr.clear();
               }
            }
         }
      }
   }

   // Create (12) second double service constraint.
   for (int i = 1; i < m_inst.numNodes() - 1; ++i) {
      if (m_inst.nodeSvcType(i) == Instance::SINGLE)
         continue;

      for (int v1 = 0; v1 < m_inst.numVehicles(); ++v1) {
         for (int v2 = 0; v2 < m_inst.numVehicles(); ++v2) {
            for (int s2 = 0; s2 < m_inst.numSkills(); ++s2) {
               for (int s1 = 0; s1 < s2; ++s1) {

                  if (!m_t[i][v2][s2].getImpl())
                     continue;

                  if (!m_t[i][v1][s1].getImpl())
                     continue;

                  const double bigM = 5e4;

                  m_expr += m_t[i][v2][s2];
                  m_expr -= m_t[i][v1][s1];
                  m_expr -= m_inst.nodeDeltaMax(i);
                  m_expr -= 2.0 * bigM;

                  for (int j = 0; j < m_inst.numNodes() - 1; ++j) {
                     if (m_x[j][i][v1][s1].getImpl())
                        m_expr += bigM * m_x[j][i][v1][s1];

                     if (m_x[j][i][v2][s2].getImpl())
                        m_expr += bigM * m_x[j][i][v2][s2];
                  }

                  IloConstraint c = m_expr <= 0;
                  std::snprintf(buf, 128, "sync_dmax#%d#%d#%d#%d#%d", i, v1, v2, s1, s2);
                  c.setName(buf);

                  m_model.add(c);
                  m_expr.clear();
               }
            }
         }
      }
   }
   #endif // Disables synchronization constraints.
}

bool MipModel::isFlowOk(int i, int v) const {
   // To be ok, you need something "coming into" this node,
   // and something else "coming out" out from it.

   // Check for flow in.
   // [j][i][v][s]
   bool hasIn = false;
   for (int s: m_inst.nodeSkills(i)) {
      if (!m_inst.vehicleHasSkill(v, s)) continue;
      hasIn = true;
      goto secondCheck;
   }

secondCheck:
   // Check for flow out.
   // [i][j][v][s]
   bool hasOut = false;
   for (int j = 0; j < m_inst.numNodes()-1; ++j) {
      for (int s : m_inst.nodeSkills(j)) {
         if (!m_inst.vehicleHasSkill(v, s)) continue;
         hasOut = true;
         goto checkBoth;
      }
   }

checkBoth:
   return hasIn && hasOut;
}
