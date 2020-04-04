#include "MipModel.h"

#include <random>
#include <cassert>
#include <algorithm>
#include <numeric>

using namespace std;

MipModel::MipModel(const Instance &inst): m_inst(inst) {
   // Basic CPLEX variables.
   m_model = IloModel(m_env);
   m_cplex = IloCplex(m_model);
   m_model.setName("routing_cost");

   // Helper objects
   IloExpr expr(m_env);
   char buf[128];

   // Maximum tardiness variable.
   m_Tmax = IloNumVar(m_env, 0., IloInfinity, IloNumVar::Float, "tmax");

   double bigM = 1e6;

   // Create decision variables x
   m_x = Var4D(m_env, m_inst.numNodes() - 1);
   for (int i = 0; i < m_inst.numNodes() - 1; ++i) {
      m_x[i] = Var3D(m_env, m_inst.numNodes() - 1);

      for (int j = 0; j < m_inst.numNodes() - 1; ++j) {
         m_x[i][j] = Var2D(m_env, m_inst.numVehicles());

         for (int v = 0; v < m_inst.numVehicles(); ++v) {
            m_x[i][j][v] = Var1D(m_env, m_inst.numSkills());

            for (int s = 0; s < m_inst.numSkills(); ++s) {

               // Does not create unnecessary variables.
               if (i == j)
                  continue;
               if (m_inst.vehicleHasSkill(v, s) == 0) {
                  continue;
               }
               if (j != 0) {
                  if (m_inst.nodeReqSkill(j, s) == 0) {
                     continue;
                  }
               } else {
                  if (m_inst.nodeReqSkill(i, s) == 0)
                     continue;
               }

               snprintf(buf, sizeof buf, "x(%d,%d,%d,%d)", i, j, v, s);
               m_x[i][j][v][s] = IloNumVar(m_env, 0.0, 1.0, IloNumVar::Bool, buf);

               // Embeds Constraints (2).
               expr += L1 * m_inst.distance(i, j) * m_x[i][j][v][s];
            }
         }
      }
   }

   // Create aux variables z
   m_z = Var2D(m_env, m_inst.numNodes() - 2);
   for (int i = 1; i < m_inst.numNodes() - 1; ++i) {
      m_z[i-1] = Var1D(m_env, m_inst.numSkills());

      for (int s = 0; s < m_inst.numSkills(); ++s) {
         if (m_inst.nodeReqSkill(i, s) == 0)
            continue;

         snprintf(buf, sizeof buf, "z(%d,%d)", i, s);
         m_z[i-1][s] = IloNumVar(m_env, 0., IloInfinity, IloNumVar::Float, buf);

         // Embeds Constraints (3).
         expr += L2 * m_z[i-1][s];
      }
   }

   // Create aux variables t
   m_t = Var3D(m_env, m_inst.numNodes() - 1);
   for (int i = 0; i < m_inst.numNodes() - 1; ++i) {
      m_t[i] = Var2D(m_env, m_inst.numVehicles());

      for (int v = 0; v < m_inst.numVehicles(); ++v) {
         m_t[i][v] = Var1D(m_env, m_inst.numSkills());

         for (int s = 0; s < m_inst.numSkills(); ++s) {

            // Does not generate unneeded variables.
            if (m_inst.nodeReqSkill(i, s) == 0)
               continue;
            if (m_inst.vehicleHasSkill(v, s) == 0)
               continue;

            snprintf(buf, sizeof buf, "t(%d,%d,%d)", i, v, s);
            m_t[i][v][s] = IloNumVar(m_env, 0, IloInfinity, IloNumVar::Float, buf);

            // Create (9) start time window constraints
            m_t[i][v][s].setLb(m_inst.nodeTwMin(i));
         }
      }
   }

   // Create (1) objective function
   expr += L3 * m_Tmax;
   m_obj = IloObjective(m_env, expr, IloObjective::Minimize, "routingCost");
   m_model.add(m_obj);
   expr.clear();

   // Create (4) greatest tardiness constraints
   for (int i = 1; i < m_inst.numNodes() - 1; ++i) {
      for (int s = 0; s < m_inst.numSkills(); ++s) {

         if (!m_z[i - 1][s].getImpl())
            continue;

         IloConstraint c = m_Tmax - m_z[i-1][s] >= 0;
         snprintf(buf, sizeof buf, "tmax(%d,%d)", i, s);
         c.setName(buf);
         m_model.add(c);
      }
   }

   // Create (5-1) flow on source node
   for (int v = 0; v < m_inst.numVehicles(); ++v) {
      for (int i = 0; i < m_inst.numNodes() - 1; ++i) {
         for (int s = 0; s < m_inst.numSkills(); ++s) {
            if (m_x[0][i][v][s].getImpl())
               expr += m_x[0][i][v][s];
         }
      }
      IloConstraint c = expr == 1;
      snprintf(buf, sizeof buf, "depot_src(%d)", v);
      c.setName(buf);
      m_model.add(c);
      expr.clear();
   }

   // Create (5-2) flow on sink node
   for (int v = 0; v < m_inst.numVehicles(); ++v) {
      for (int i = 0; i < m_inst.numNodes() - 1; ++i) {
         for (int s = 0; s < m_inst.numSkills(); ++s) {
            if (m_x[i][0][v][s].getImpl())
            expr += m_x[i][0][v][s];
         }
      }
      IloConstraint c = expr == 1;
      snprintf(buf, sizeof buf, "depot_sink(%d)", v);
      c.setName(buf);
      m_model.add(c);
      expr.clear();
   }

   // Create (6) flow conservation constraints
   for (int i = 1; i < m_inst.numNodes() - 1; ++i) {
      for (int v = 0; v < m_inst.numVehicles(); ++v) {
         for (int j = 0; j < m_inst.numNodes() - 1; ++j) {
            for (int s = 0; s < m_inst.numSkills(); ++s) {
               if (m_x[j][i][v][s].getImpl())
                  expr += m_x[j][i][v][s];
               if (m_x[i][j][v][s].getImpl())
                  expr -= m_x[i][j][v][s];
            }
         }
         IloConstraint c = expr == 0;
         snprintf(buf, sizeof buf, "flow_conserv(%d,%d)", i, v);
         c.setName(buf);
         m_model.add(c);
         expr.clear();
      }
   }

   // Create (7) assignment constraints
   for (int i = 1; i < m_inst.numNodes() - 1; ++i) {
      for (int s = 0; s < m_inst.numSkills(); ++s) {

         if (!m_inst.nodeReqSkill(i, s))
            continue;

         for (int v = 0; v < m_inst.numVehicles(); ++v) {
            for (int j = 0; j < m_inst.numNodes() - 1; ++j) {
               if (m_x[j][i][v][s].getImpl())
                  expr += m_x[j][i][v][s];
            }
         }
         expr -= m_inst.nodeReqSkill(i, s);
         IloConstraint c = expr == 0;
         snprintf(buf, sizeof buf, "svc_attendance(%d,%d)", i, s);
         c.setName(buf);
         m_model.add(c);
         expr.clear();
      }
   }

   // Create (8) subcycle elimination constraints
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

                  expr += m_t[i][v][s1];
                  expr += m_inst.nodeProcTime(i, s1);
                  expr += m_inst.distance(i, j);
                  expr -= m_t[j][v][s2];
                  expr -= bigM;
                  expr += bigM * m_x[i][j][v][s2];

                  IloConstraint c = expr <= 0;
                  snprintf(buf, 128, "subcycle_elim(%d,%d,%d,%d,%d)", i, j, v, s1, s2);
                  c.setName(buf);
                  m_model.add(c);
                  expr.clear();
               }
            }
         }
      }
   }


   // Create (10) end time window constraints
   for (int i = 1; i < m_inst.numNodes() - 1; ++i) {
      for (int v = 0; v < m_inst.numVehicles(); ++v) {
         for (int s = 0; s < m_inst.numSkills(); ++s) {

            if (m_z[i-1][s].getImpl() == nullptr)
               continue;
            if (m_t[i][v][s].getImpl() == nullptr)
               continue;

            IloConstraint c = m_t[i][v][s] - m_inst.nodeTwMax(i) - m_z[i-1][s] <= 0;
            snprintf(buf, 128, "tw_end(%d,%d,%d)", i, v, s);
            c.setName(buf);
            m_model.add(c);
         }
      }
   }

   // Create the synchronization constraints.
   for (int i = 1, cc = 1; i < m_inst.numNodes() - 1; ++i) {
      if (m_inst.nodeSvcType(i) != Instance::SIM && m_inst.nodeSvcType(i) != Instance::PRED)
         continue;

      for (int v1 = 0; v1 < m_inst.numVehicles(); ++v1) {
         for (int v2 = 0; v2 < m_inst.numVehicles(); ++v2) {
            for (int s2 = 0; s2 < m_inst.numSkills(); ++s2) {
               for (int s1 = 0; s1 < s2; ++s1, ++cc) {
                  if (m_t[i][v2][s2].getImpl() == nullptr)
                     continue;
                  if (m_t[i][v1][s1].getImpl() == nullptr)
                     continue;

                  // Create (11).
                  {
                     expr += m_t[i][v2][s2];
                     expr -= m_t[i][v1][s1];
                     expr -= m_inst.nodeDeltaMin(i);
                     expr += 2 * bigM;

                     for (int j = 0; j < m_inst.numNodes() - 1; ++j) {
                        if (m_x[j][i][v1][s1].getImpl())
                           expr -= bigM * m_x[j][i][v1][s1];
                        if (m_x[j][i][v2][s2].getImpl())
                           expr -= bigM * m_x[j][i][v2][s2];
                     }

                     IloConstraint c = expr >= 0;
                     snprintf(buf, sizeof buf, "sync_a(%d,%d,%d,%d,%d)", i, v1, v2, s1, s2);
                     c.setName(buf);
                     m_model.add(c);
                     expr.clear();
                  }

                  // Create (12).
                  {
                     expr += m_t[i][v2][s2];
                     expr -= m_t[i][v1][s1];
                     expr -= m_inst.nodeDeltaMax(i);
                     expr -= 2 * bigM;

                     for (int j = 0; j < m_inst.numNodes() - 1; ++j) {
                        if (m_x[j][i][v1][s1].getImpl())
                           expr += bigM * m_x[j][i][v1][s1];
                        if (m_x[j][i][v2][s2].getImpl())
                           expr += bigM * m_x[j][i][v2][s2];
                     }

                     IloConstraint c = expr <= 0;
                     snprintf(buf, sizeof buf, "sync_b(%d,%d,%d,%d,%d)", i, v1, v2, s1, s2);
                     c.setName(buf);
                     m_model.add(c);
                     expr.clear();
                  }
               }
            }
         }
      }
   }

   // Implements (13).
   // It is not strictly necessary since such variables are removed from the problem.
   #if 0
   int bcount = 0;
   for (int i = 0 ; i < inst.numNodes()-1; ++i) {
      for (int j = 0; j < inst.numNodes()-1; ++j) {
         for (int v = 0; v < inst.numVehicles(); ++v) {
            for (int s = 0; s < inst.numSkills(); ++s) {
               if (m_x[i][j][v][s].getImpl()) {
                  IloConstraint c = m_x[i][j][v][s] <= m_inst.nodeReqSkill(j, s) * m_inst.vehicleHasSkill(v, s);
                  m_model.add(c);
                  if (m_inst.nodeReqSkill(j, s) * m_inst.vehicleHasSkill(v, s) <= 0) {
                     cout << "Creating (13) -> " << i << "," << j << "," << v << "," << s << endl;
                     cout << "  Has demand? = " << m_inst.nodeReqSkill(j, s) << endl;
                     cout << "  Has qualification? = " << m_inst.vehicleHasSkill(v, s) << endl;
                     ++bcount;
                  }
               }
            }
         }
      }
   }
   cout << "I had to create " << bcount << " (13)-constraints.\n";
   #endif

   // Release helper objects
   expr.end();
}

MipModel::~MipModel() {
   m_env.end();
}

void MipModel::writeLp(const char *fname) {
   m_cplex.exportModel(fname);
}
