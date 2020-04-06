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

/**
 * Class to build constructive solutions to HHCRSP.
 */
class InitialRouting {
public:
   struct Cell {
      int m_node;
      int m_skill;
      double m_arrivalTime;
      double m_startTime;

      inline void clear() {
         m_node  = -1;
         m_skill = -1;
         m_arrivalTime = 0;
         m_startTime = 0;
      }
   };

   InitialRouting(const Instance &inst);
   virtual ~InitialRouting();

   const Instance& instance() const;

   /**
    * Construct a solution using Mankowska et al. (2014) initial routing heuristic.
    */
   void solve();

   void printSolutionMatrix() const;

   int node(int v, int pos) const;
   int skill(int v, int pos) const;

   int maxPos() const;

private:
   const Instance &m_inst;

   std::vector <std::vector<Cell>> m_solMatrix;
   std::vector <int> m_lastNode;

   std::tuple <int,int> findReqSkills(int node) const;
   std::tuple <int,double> findVehicle(int skill, int targetNode, int forbiddenVehicle = -1) const;

   void updateVehicle(int v, double arr);
   void updateVehicles(int v1, int v2, double arr1, double arr2);
};

