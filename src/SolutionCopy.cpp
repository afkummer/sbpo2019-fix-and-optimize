#include "SolutionCopy.h"

void solutionCopy(const InitialRouting& origin, MipModel& dest) {
   const Instance &inst = origin.instance();

   // Set the routes for each vehicle.
   for (int v = 0; v < inst.numVehicles(); ++v) {
      int prevNode = 0;
      int prevSkill = -1;

      for (int pos = 0; pos < origin.maxPos(); ++pos) {
         int currNode =  origin.node(v, pos);
         if (currNode != -1) {
            int currSkill = origin.skill(v, pos);

            dest.setVarX(prevNode, currNode, v, currSkill, 1.0, 1.0);

            prevNode = currNode;
            prevSkill = currSkill;
         }
      }

      // The link between last node and the depot is not set by the
      // previous loop. Do it manually.
      dest.setVarX(prevNode, 0, v, prevSkill, 1.0, 1.0);
   }
}

