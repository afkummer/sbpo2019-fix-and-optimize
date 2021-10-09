#include "InitialRouting.h"
#include "MipModel.h"

#include <iostream>
#include <algorithm>
#include <iomanip>
#include <numeric> //iota

InitialRouting::InitialRouting(const Instance &inst):
m_inst(inst) {
   m_solMatrix.resize(m_inst.numVehicles());
   for (auto &row: m_solMatrix)
      row.resize(m_inst.numNodes()-2);
   m_lastPos.resize(m_inst.numVehicles(), 0);
}

InitialRouting::~InitialRouting() {
   // Empty
}

void InitialRouting::solve() {
   // Reset solution structure
   for (auto &row: m_solMatrix)
      for (auto &col: row)
         col.clear();
   std::fill(m_lastPos.begin(), m_lastPos.end(), 0);

   // Compute sorting.
   std::vector <int> sort(m_inst.numNodes()-2);
   std::iota(sort.begin(), sort.end(), 1);
   std::sort(sort.begin(), sort.end(), [&] (int a, int b) {
      return m_inst.nodeTwMax(a) < m_inst.nodeTwMax(b);
   });

//    std::cout << "Nodes: \n";
//    for (int i: sort)
//       std::cout << i <<  "  twmin = " << m_inst.nodeTwMin(i) << std::endl;

   // Constructive loop.
   for (int k = 0; k < m_inst.numNodes()-2; ++k) {
      if (m_inst.nodeSvcType(sort[k]) == Instance::SINGLE) {
         int s = std::get<0>(findReqSkills(sort[k]));
         if (s == -1) std::abort();
         auto vTuple = findVehicle(s, sort[k]);
         int v = std::get<0>(vTuple);
         if (v == -1) std::abort();
         m_solMatrix[v][k].m_node = sort[k];
         m_solMatrix[v][k].m_skill = s;
         m_lastPos[v] = k;
         updateVehicle(v, std::get<1>(vTuple));
      } else { // Both simultaneous and double services.
         auto sTuple = findReqSkills(sort[k]);
         int s1 = std::get<0>(sTuple);
         int s2 = std::get<1>(sTuple);
         if (s1 == -1) std::abort();
         if (s2 == -1) std::abort();
         auto v1Tuple = findVehicle(s1, sort[k]);
         int v1 = std::get<0>(v1Tuple);
         if (v1 == -1) std::abort();
         auto v2Tuple = findVehicle(s2, sort[k], v1);
         int v2 = std::get<0>(v2Tuple);
         if (v2 == -1) std::abort();
         m_solMatrix[v1][k].m_node = sort[k];
         m_solMatrix[v1][k].m_skill = s1;
         m_lastPos[v1] = k;
         m_solMatrix[v2][k].m_node = sort[k];
         m_solMatrix[v2][k].m_skill = s2;
         m_lastPos[v2] = k;
         updateVehicles(v1, v2, std::get<1>(v1Tuple), std::get<1>(v2Tuple));
      }
   }
}

void InitialRouting::printSolutionMatrix() const {
   for (int v = 0; v < m_inst.numVehicles(); ++v) {
      for (int i = 1; i < m_inst.numNodes()-1; ++i) {
         std::cout << std::setw(3) << m_solMatrix[v][i-1].m_node << '(' << std::setw(2) << m_solMatrix[v][i-1].m_skill << ")  ";
      }
      std::cout << std::endl;
   }
}

int InitialRouting::node(int v, int pos) const {
   return m_solMatrix[v][pos].m_node;
}

int InitialRouting::skill(int v, int pos) const {
   return m_solMatrix[v][pos].m_skill;
}

int InitialRouting::maxPos() const {
   return m_inst.numNodes()-2;
}

void InitialRouting::copyTo(MipModel &m) const {
   for (int v = 0; v < m_inst.numVehicles(); ++v) {
      int prevNode = 0;
      for (int pos = 0; pos < m_inst.numNodes() - 2; ++pos) {
         int node_ = node(v, pos);
         int skill_ = skill(v, pos);
         if (node_ == -1)
            continue;
         m.setBoundX(prevNode, node_, v, skill_, 1.0, 1.0);
         prevNode = node_;
      }
   }
}


std::tuple<int, int> InitialRouting::findReqSkills(int node) const {
   std::tuple <int,int> reqs = {-1, -1};
   for (int s = 0; s < m_inst.numSkills(); ++s) {
      if (m_inst.nodeReqSkill(node, s)) {
         if (std::get<0>(reqs) == -1) {
            std::get<0>(reqs) = s;
         } else if (std::get<1>(reqs) == -1) {
            std::get<1>(reqs) = s;
         } else {
            std::cout << "Node " << node << " requires more than 2 skills." << std::endl;
            std::abort();
         }
      }
    }
   return reqs;
}

std::tuple <int,double> InitialRouting::findVehicle(int skill, int targetNode, int forbiddenVehicle) const {
   int bestV = -1;
   double bestArrival = 0.;

   for (int v = 0; v < m_inst.numVehicles(); ++v) {
      if (v == forbiddenVehicle)
         continue;
      if (m_inst.vehicleHasSkill(v, skill) == 0)
         continue;

      double arrivalTime = -1.;
      // First patient on vs' route.
      if (m_lastPos[v] == 0) {
         arrivalTime = m_inst.distance(0, targetNode);
      } else {
         int last = m_lastPos[v];
         arrivalTime = m_solMatrix[v][last].m_startTime
            + m_inst.nodeProcTime(m_solMatrix[v][last].m_node, m_solMatrix[v][last].m_skill)
            + m_inst.distance(m_solMatrix[v][last].m_node, targetNode);

      }

      if (arrivalTime >= 0 && (bestV == -1 || arrivalTime < bestArrival)) {
         bestV = v;
         bestArrival = arrivalTime;
      }
   }

   return std::make_tuple(bestV, bestArrival);
}

void InitialRouting::updateVehicle(int v, double arr) {
   int last = m_lastPos[v];
   Cell &cell = m_solMatrix[v][last];
   cell.m_arrivalTime = arr;
   cell.m_startTime = std::max(cell.m_arrivalTime, m_inst.nodeTwMin(cell.m_node));
}

void InitialRouting::updateVehicles(int v1, int v2, double arr1, double arr2) {
   int last1 = m_lastPos[v1];
   Cell &cell1 = m_solMatrix[v1][last1];
   int last2 = m_lastPos[v2];
   Cell &cell2 = m_solMatrix[v2][last2];

   if (m_inst.nodeSvcType(cell1.m_node) == Instance::SIM) {
      double st = std::max(m_inst.nodeTwMin(cell1.m_node), std::max(arr1, arr2));
      cell1.m_startTime = cell2.m_startTime = st;
   } else if (m_inst.nodeSvcType(cell1.m_node) == Instance::PRED) {
      cell1.m_startTime = std::max(m_inst.nodeTwMin(cell1.m_node), cell1.m_arrivalTime);
      cell2.m_startTime = std::max(cell1.m_startTime + m_inst.nodeProcTime(cell1.m_node, cell1.m_skill), cell2.m_arrivalTime);
   } else {
      std::cout << "Unknown service type for node " << cell1.m_node << ": " << m_inst.nodeSvcType(cell1.m_node) << std::endl;
      std::abort();
   }
}



