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

#include <iosfwd>
#include <vector>
#include <tuple>

class Instance {
public:
   enum SvcType {
      NONE   = -1,
      SINGLE = 0,
      PRED   = 1,
      SIM = 2,
      MAX_
   };

   Instance(const char *fname);
   virtual ~Instance();

   int numVehicles() const;
   int numNodes() const;
   int numSkills() const;

   bool vehicleHasSkill(int vehicle, int skill) const;
   bool nodeReqSkill(int node, int skill) const;

   SvcType nodeSvcType(int node) const;

   double nodeDeltaMin(int node) const;
   double nodeDeltaMax(int node) const;

   double nodeTwMin(int node) const;
   double nodeTwMax(int node) const;

   double nodeProcTime(int node, int skill) const;

   double nodePosX(int node) const;
   double nodePosY(int node) const;

   double distance(int fromNode, int toNode) const;

   const std::string &fileName() const;

   friend std::ostream &operator<<(std::ostream &out, const Instance &inst);

protected:
   /**
    * Resize data structures to acommodate all instance parameters.
    */
   void resize();
   void resize(int numNodes, int numVehicles, int numSkills);

private:
   std::string m_fname;
   int m_numNodes;
   int m_numVehicles;
   int m_numSkills;

   std::vector <std::vector<int>> m_vehicleSkills;
   std::vector <std::vector<int>> m_nodeReqSkills;
   std::vector <SvcType> m_nodeSvcType;

   std::vector <std::tuple<double, double>> m_nodeDelta;
   std::vector <std::tuple<double, double>> m_nodeTw;
   std::vector <std::vector<double>> m_nodeProcTime;
   std::vector <std::tuple<double, double>> m_nodePos;

   std::vector <std::vector <double>> m_distances;
};
