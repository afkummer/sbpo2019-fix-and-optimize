#pragma once

#include <iosfwd>
#include <vector>
#include <tuple>

/**
 * @brief Implentation of a instance to HHC problem.
 *
 * Conventions:
 * <ul>
 *    <li> Node 0 means depot source </li>
 *    <li> Last node meand depot sink </li>
 * </ul>
 */
class Instance {
public:
   enum SvcType {
      NONE   = -1,
      SINGLE = 0,
      PRED   = 1,
      SIM = 2
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
   
   int personnelSize() const;
   int personnelSkill(int p, int s) const;
   int personnelDefaultVehicle(int p) const;

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

   // Team designation generated data.
   std::vector <std::vector<int>> m_pSkill;
   std::vector <int> m_originalV;

};

std::vector<std::vector<std::tuple<int,int>>> readSimpleSolution(const std::string &fname);
void writeSimpleSolution(const std::vector<std::vector<std::tuple<int,int>>> &routes, const std::string &fname);
