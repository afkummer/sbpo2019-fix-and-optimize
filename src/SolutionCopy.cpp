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

#include "SolutionCopy.h"

#include <iostream>
#include <fstream>
#include <string>


using namespace std;


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

void solutionCopy(ifstream &file, MipModel& dest) {
   if (!file) abort();

   // Ignore header lines.
   string line;
   getline(file, line);
   getline(file, line);
   getline(file, line);
   getline(file, line);

   // Read the structured solution file.
   int v, len;
   while (file >> v >> len) {
      for (int row = 0; row < len; ++row) {
         int i, j, s;
         file >> i >> j >> v >> s;
         dest.setVarX(i, j, v, s, 1.0, 1.0);
      }
   }
}

