#include "Instance.h"
#include <limits>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <numeric> // std::accumulate

using namespace std;

Instance::Instance(const char* fname) {
   m_fname = fname;
   std::ifstream fid(fname);
   if (!fid) {
      std::cout << "Instance file " << fname << " could not be read." << std::endl;
      std::exit(EXIT_FAILURE);
   }

   std::vector <int> dscheck;
   std::string buf, lastenv;
   while (std::getline(fid, buf)) {
      if (buf.length() == 0 || buf.find_first_not_of(" ") == std::string::npos) {
         // Skip empty lines
      } else if (buf == "nbNodes") {
         lastenv = buf;
         fid >> m_numNodes;
      } else if (buf == "nbVehi") {
         lastenv = buf;
         fid >> m_numVehicles;
      } else if (buf == "nbServi") {
         lastenv = buf;
         fid >> m_numSkills;
         resize();
      } else if (buf == "r") {
         lastenv = buf;
         for (auto &i: m_nodeReqSkills)
            for (auto &s: i)
               fid >> s;
      } else if (buf == "DS") {
         lastenv = buf;
         std::getline(fid, buf);
         std::stringstream stream(buf);
         int id;
         while (stream >> id) {
            dscheck.push_back(id-1);
         }
         for (int i = 1; i < m_numNodes-1; ++i) {
            if (std::find(dscheck.begin(), dscheck.end(), i) != dscheck.end()) {
               continue;
            }
            m_nodeSvcType[i] = SvcType::SINGLE;

            int sksum = std::accumulate(m_nodeReqSkills[i].begin(), m_nodeReqSkills[i].end(), 0);
            if (sksum != 1) {
               std::cout << "Single service node " << i << " requiring a invalid amount of "
                  << sksum << " service types." << std::endl;
               std::abort();
            }
         }
      } else if (buf == "a") {
         lastenv = buf;
         for (auto &v: m_vehicleSkills)
            for (auto &s: v)
               fid >> s;
      } else if (buf == "x") {
         lastenv = buf;
         for (int i = 0; i < m_numNodes; ++i)
            fid >> std::get<0>(m_nodePos[i]);
      } else if (buf == "y") {
         lastenv = buf;
         for (int i = 0; i < m_numNodes; ++i)
            fid >> std::get<1>(m_nodePos[i]);
      } else if (buf == "d") {
         lastenv = buf;
         for (int i = 0; i < m_numNodes; ++i)
            for (int j = 0; j < m_numNodes; ++j)
               fid >> m_distances[i][j];
      } else if (buf == "p") {
         lastenv = buf;
         for (int i = 0; i < m_numNodes; ++i) {
            for (int v = 0; v < m_numVehicles; ++v) {
               std::getline(fid, buf);
               // Use only processing time of first vehicle.
               // This is a issue with the instance format.
               if (v != 0)
                  continue;
               std::stringstream stream(buf);
               for (int s = 0; s < m_numSkills; ++s) {
                  stream >> m_nodeProcTime[i][s];
               }
            }
         }
      } else if (buf == "mind") {
         lastenv = buf;
         for (auto &i: m_nodeDelta)
            fid >> std::get<0>(i);
      } else if (buf == "maxd") {
         lastenv = buf;
         for (auto &i: m_nodeDelta)
            fid >> std::get<1>(i);
      } else if (buf == "e") {
         lastenv = buf;
         for (auto &i: m_nodeTw)
            fid >> std::get<0>(i);
      } else if (buf == "l") {
         lastenv = buf;
         for (auto &i: m_nodeTw)
            fid >> std::get<1>(i);
      } else {
         std::cout << "Unknow line content: " << buf << std::endl;
         std::cout << "Line length: " << buf.length() << std::endl;
         std::cout << "Last section read: " << lastenv << std::endl;
         std::exit(EXIT_FAILURE);
      }
   }

   fid.close();

   // Detect service type of double service nodes.
   for (int i: dscheck) {
      int sksum = std::accumulate(m_nodeReqSkills[i].begin(), m_nodeReqSkills[i].end(), 0);
      int dmin = std::get<0>(m_nodeDelta[i]);

      if (sksum == 2) {
         if (dmin <= 0.001) {
            m_nodeSvcType[i] = SvcType::SIM;
         } else {
            m_nodeSvcType[i] = SvcType::PRED;
         }
      } else {
         std::cout << "Double service node " << i << " requiring a invalid amount of "
            << sksum << " service types." << std::endl;
         std::abort();
      }
   }

   // Create personnel data without any heuristic.
   for (int v = 0; v < numVehicles(); ++v) {

      if (std::accumulate(m_vehicleSkills[v].begin(), m_vehicleSkills[v].begin()+3, 0) > 0) {
         m_pSkill.push_back(std::vector<int>());
         for (int s = 0; s < 3; ++s) {
            if (vehicleHasSkill(v,s)) {
               m_pSkill.back().push_back(s);
            }
         }
         m_originalV.push_back(v);
      }

      if (std::accumulate(m_vehicleSkills[v].begin()+3, m_vehicleSkills[v].end(), 0) > 0) {
         m_pSkill.push_back(std::vector<int>());
         for (int s = 3; s < 6; ++s) {
            if (vehicleHasSkill(v,s)) {
               m_pSkill.back().push_back(s);
            }
         }
         m_originalV.push_back(v);
      }
   }

//    for (int p = 0; p < personnelSize(); ++p) {
//       std::cout << "Skills for caregiver " << p << ": ";
//       for (int s = 0; s < numSkills(); ++s) {
//          if (personnelSkill(p, s))
//             std::cout << s << " ";
//       }
//       std::cout << "    Default vehicle: " << personnelDefaultVehicle(p) << std::endl;
//    }
//
//    std::cout << "m_originalV = ";
//    for (int &v: m_originalV)
//       std::cout << v << " ";
//    std::cout << std::endl;
}

Instance::~Instance() {
   // Empty by design
}

int Instance::numVehicles() const {
   return m_numVehicles;
}

int Instance::numNodes() const {
   return m_numNodes;
}

int Instance::numSkills() const {
   return m_numSkills;
}

bool Instance::vehicleHasSkill(int vehicle, int skill) const {
   return m_vehicleSkills[vehicle][skill];
}

bool Instance::nodeReqSkill(int node, int skill) const {
   return m_nodeReqSkills[node][skill];
}

Instance::SvcType Instance::nodeSvcType(int node) const {
   return m_nodeSvcType[node];
}

double Instance::nodeDeltaMin(int node) const {
   return std::get<0>(m_nodeDelta[node]);
}

double Instance::nodeDeltaMax(int node) const {
   return std::get<1>(m_nodeDelta[node]);
}

double Instance::nodeTwMin(int node) const {
   return std::get<0>(m_nodeTw[node]);
}

double Instance::nodeTwMax(int node) const {
   return std::get<1>(m_nodeTw[node]);
}

double Instance::nodeProcTime(int node, int skill) const {
   return m_nodeProcTime[node][skill];
}

double Instance::nodePosX(int node) const {
   return std::get<0>(m_nodePos[node]);
}

double Instance::nodePosY(int node) const {
   return std::get<1>(m_nodePos[node]);
}

double Instance::distance(int fromNode, int toNode) const {
   return m_distances[fromNode][toNode];
}

int Instance::personnelSize() const {
   return m_pSkill.size();
}

int Instance::personnelSkill(int p, int s) const {
   auto iter = std::find(std::begin(m_pSkill[p]), std::end(m_pSkill[p]), s);
   return iter != m_pSkill[p].end();
}

int Instance::personnelDefaultVehicle(int p) const {
   return m_originalV[p];
}

const std::string & Instance::fileName() const {
   return m_fname;
}

void Instance::resize() {
   m_vehicleSkills.resize(m_numVehicles);
   for (auto &row: m_vehicleSkills)
      row.resize(m_numSkills, 0);

   m_nodeReqSkills.resize(m_numNodes);
   for (auto &row: m_nodeReqSkills)
      row.resize(m_numSkills, 0);

   m_nodeSvcType.resize(m_numNodes, SvcType::NONE);


   const double dblInf = std::numeric_limits<double>::infinity();
   m_nodeDelta.resize(m_numNodes, std::make_tuple(-dblInf, dblInf));

   m_nodeTw.resize(m_numNodes, std::make_tuple(-dblInf, dblInf));

   m_nodeProcTime.resize(m_numNodes);
   for (auto &row: m_nodeProcTime)
      row.resize(m_numSkills, dblInf);

   m_nodePos.resize(m_numNodes, std::make_tuple(-dblInf, dblInf));

   m_distances.resize(m_numNodes);
   for (auto &row: m_distances)
      row.resize(m_numNodes, dblInf);
}

void Instance::resize(int numNodes, int numVehicles, int numSkills) {
   m_numNodes = numNodes;
   m_numVehicles = numVehicles;
   m_numSkills = numSkills;

   resize();
}

std::ostream &operator<<(std::ostream &out, const Instance &inst) {
   out << "nbNodes\n" << inst.numNodes() << "\n";
   out << "nbVehi\n" << inst.numVehicles() << "\n";
   out << "nbServi\n" << inst.numSkills() << "\n";

   out << "r\n";
   for (int i = 0; i < inst.numNodes(); ++i) {
      for (int s = 0; s < inst.numSkills(); ++s) {
         out << inst.nodeReqSkill(i, s);
         if (s < inst.numSkills()-1)
            out << " ";
      }
      out << "\n";
   }

   out << "DS\n";
   for (int i = inst.numNodes()-2, cnt = 0; i > 0; --i) {
      if (inst.nodeSvcType(i) != Instance::SINGLE) {
         if (cnt > 0)
            out << " ";
         out << i+1;
         ++cnt;
      }
   }

   out << "\na\n";
   for (int v = 0; v < inst.numVehicles(); ++v) {
      for (int s = 0; s < inst.numSkills(); ++s) {
         out << inst.vehicleHasSkill(v, s);
         if (s < inst.numSkills()-1)
            out << " ";
      }
      out << "\n";
   }

   out << "x\n";
   for (int i = 0; i < inst.numNodes(); ++i) {
      out << inst.nodePosX(i);
      if (i < inst.numNodes()-1)
         out << " ";
   }

   out << "\ny\n";
   for (int i = 0; i < inst.numNodes(); ++i) {
      out << inst.nodePosY(i);
      if (i < inst.numNodes()-1)
         out << " ";
   }

   out << "\nd\n";
   out.precision(6);
   for (int i = 0; i < inst.numNodes(); ++i) {
      for (int j = 0; j < inst.numNodes(); ++j) {
         out << inst.distance(i, j);
         if (j < inst.numNodes()-1)
            out << " ";
      }
      out << "\n";
   }

   out << "p\n";
   for (int i = 0; i < inst.numNodes(); ++i) {
      for (int v = 0; v < inst.numVehicles(); ++v) {
         for (int s = 0; s < inst.numSkills(); ++s) {
            out << inst.nodeProcTime(i, s);
            if (s < inst.numSkills()-1)
               out << " ";
         }
         out << "\n";
      }
   }

   out << "mind\n";
   for (int i = 0; i < inst.numNodes(); ++i) {
      out << inst.nodeDeltaMin(i);
      if (i < inst.numNodes()-1)
         out << " ";
   }

   out << "\nmaxd\n";
   for (int i = 0; i < inst.numNodes(); ++i) {
      out << inst.nodeDeltaMax(i);
      if (i < inst.numNodes()-1)
         out << " ";
   }

   out << "\ne\n";
   for (int i = 0; i < inst.numNodes(); ++i) {
      out << inst.nodeTwMin(i);
      if (i < inst.numNodes()-1)
         out << " ";
   }

   out << "\nl\n";
   for (int i = 0; i < inst.numNodes(); ++i) {
      out << inst.nodeTwMax(i);
      if (i < inst.numNodes()-1)
         out << " ";
   }

   return out;
}

std::vector<std::vector<std::tuple<int,int>>> readSimpleSolution(const std::string &fname) {
   std::ifstream fid(fname);
   if (!fid) {
      cerr << "File '" << fname << "' could not be open in read mode.\n";
      abort();
   }
   std::vector<std::vector<std::tuple<int,int>>> routes;
   int size;
   fid >> size;
   routes.resize(size);
   for (auto &r: routes) {
      fid >> size;
      r.resize(size);
      for (auto &i: r) {
         fid >> get<0>(i) >> get<1>(i);
      }
   }
   return routes;
}

void writeSimpleSolution(const std::vector<std::vector<std::tuple<int,int>>> &routes, const std::string &fname) {
   std::ofstream fid(fname);
   if (!fid) {
      cerr << "File '" << fname << "' could not be open in write mode.\n";
      abort();
   }
   fid << routes.size() << endl;
   for (auto &r: routes) {
      fid << r.size() << endl;
      for (auto &i: r) {
         fid << get<0>(i) << ' ' << get<1>(i) << endl;
      }
   }
}
