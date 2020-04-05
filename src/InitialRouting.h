#pragma once

#include "Instance.h"

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

