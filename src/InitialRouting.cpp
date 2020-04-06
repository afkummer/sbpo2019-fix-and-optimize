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

#include "InitialRouting.h"

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <numeric> //iota


using namespace std;


InitialRouting::InitialRouting(const Instance &inst): m_inst(inst) {
   m_solMatrix.resize(m_inst.numVehicles());
   for (auto &row: m_solMatrix)
      row.resize(m_inst.numNodes()-2);
   m_lastNode.resize(m_inst.numVehicles(), 0);
}

InitialRouting::~InitialRouting() {
   // Empty
}

const Instance & InitialRouting::instance() const {
   return m_inst;
}

void InitialRouting::solve() {
   // Reset solution structure
   for (auto &row: m_solMatrix)
      for (auto &col: row)
         col.clear();
   fill(m_lastNode.begin(), m_lastNode.end(), 0);

   // Compute sorting.
   vector <int> sort(m_inst.numNodes()-2);
   iota(sort.begin(), sort.end(), 1);
   std::sort(sort.begin(), sort.end(), [&] (int a, int b) {
      return m_inst.nodeTwMax(a) < m_inst.nodeTwMax(b);
   });

   // Constructive loop.
   for (int k = 0; k < m_inst.numNodes()-2; ++k) {

      if (m_inst.nodeSvcType(sort[k]) == Instance::SINGLE) {
         int s = get<0>(findReqSkills(sort[k]));
         assert(s != -1);

         auto vTuple = findVehicle(s, sort[k]);
         int v = get<0>(vTuple);
         assert(v != -1);

         m_solMatrix[v][k].m_node = sort[k];
         m_solMatrix[v][k].m_skill = s;
         m_lastNode[v] = k;

         updateVehicle(v, get<1>(vTuple));

      } else { // Both simultaneous and double services.

         auto sTuple = findReqSkills(sort[k]);
         int s1 = get<0>(sTuple);
         assert(s1 != -1);

         int s2 = get<1>(sTuple);
         assert(s2 != -1);

         auto v1Tuple = findVehicle(s1, sort[k]);
         int v1 = get<0>(v1Tuple);
         assert(v1 != -1);

         auto v2Tuple = findVehicle(s2, sort[k], v1);
         int v2 = get<0>(v2Tuple);
         assert(v2 != -1);

         m_solMatrix[v1][k].m_node = sort[k];
         m_solMatrix[v1][k].m_skill = s1;
         m_lastNode[v1] = k;

         m_solMatrix[v2][k].m_node = sort[k];
         m_solMatrix[v2][k].m_skill = s2;
         m_lastNode[v2] = k;

         updateVehicles(v1, v2, get<1>(v1Tuple), get<1>(v2Tuple));
      }
   }
}

void InitialRouting::printSolutionMatrix() const {
   for (int v = 0; v < m_inst.numVehicles(); ++v) {
      for (int i = 1; i < m_inst.numNodes()-1; ++i) {
         cout << setw(3) << m_solMatrix[v][i-1].m_node << '(' << setw(2) << m_solMatrix[v][i-1].m_skill << ")  ";
      }
      cout << endl;
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

tuple<int, int> InitialRouting::findReqSkills(int node) const {
   array <int, 2> reqs = {-1, -1};
   int apos = 0;

   for (int s = 0; s < m_inst.numSkills(); ++s) {
      if (m_inst.nodeReqSkill(node, s)) {
         assert(apos < 2 && "Node requiring more than 2 skill. (Maybe it is the depot...)");
         reqs[apos] = s;
         ++apos;
      }
   }

   return make_tuple(reqs[0], reqs[1]);
}

tuple <int,double> InitialRouting::findVehicle(int skill, int targetNode, int forbiddenVehicle) const {
   int bestV = -1;
   double bestArrival = numeric_limits<double>::infinity();

   for (int v = 0; v < m_inst.numVehicles(); ++v) {
      if (v == forbiddenVehicle)
         continue;

      if (m_inst.vehicleHasSkill(v, skill) == 0)
         continue;

      double arrivalTime = -1.;

      // First patient on vs' route.
      if (m_lastNode[v] == 0) {
         arrivalTime = m_inst.distance(0, targetNode);
      } else {
         int last = m_lastNode[v];
         arrivalTime = m_solMatrix[v][last].m_startTime
            + m_inst.nodeProcTime(m_solMatrix[v][last].m_node, m_solMatrix[v][last].m_skill)
            + m_inst.distance(m_solMatrix[v][last].m_node, targetNode);

      }

      if (arrivalTime < bestArrival) {
         bestV = v;
         bestArrival = arrivalTime;
      }
   }

   assert(bestArrival != numeric_limits<double>::infinity());

   return make_tuple(bestV, bestArrival);
}

void InitialRouting::updateVehicle(int v, double arr) {
   int last = m_lastNode[v];
   Cell &cell = m_solMatrix[v][last];
   cell.m_arrivalTime = arr;
   cell.m_startTime = max(cell.m_arrivalTime, m_inst.nodeTwMin(cell.m_node));
}

void InitialRouting::updateVehicles(int v1, int v2, double arr1, double arr2) {
   int last1 = m_lastNode[v1];
   Cell &cell1 = m_solMatrix[v1][last1];

   int last2 = m_lastNode[v2];
   Cell &cell2 = m_solMatrix[v2][last2];

   if (m_inst.nodeSvcType(cell1.m_node) == Instance::SIM) {
      double st = max(m_inst.nodeTwMin(cell1.m_node), max(arr1, arr2));
      cell1.m_startTime = cell2.m_startTime = st;
   } else if (m_inst.nodeSvcType(cell1.m_node) == Instance::PRED) {
      cell1.m_startTime = max(m_inst.nodeTwMin(cell1.m_node), cell1.m_arrivalTime);
      cell2.m_startTime = max(cell1.m_startTime + m_inst.nodeProcTime(cell1.m_node, cell1.m_skill), cell2.m_arrivalTime);
   } else {
      cout << "Unknown service type for node " << cell1.m_node << ": " << m_inst.nodeSvcType(cell1.m_node) << endl;
      exit(EXIT_FAILURE);
   }
}



