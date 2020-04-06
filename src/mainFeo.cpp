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

#include "FixAndOptimize.h"
#include "InitialRouting.h"
#include "Instance.h"
#include "MipModel.h"
#include "SolutionCopy.h"

#include <cstdlib>
#include <iostream>
#include <memory>


using namespace std;

int main(int argc, char **argv) {
   if (argc != 3) {
      std::cout << "Usage: " << argv[0] << " <1:instance path> <2: PRNG seed>" << std::endl;
      return EXIT_FAILURE;
   }

   const char *instPath = argv[1];
   const long seed = std::stol(argv[2]);

   cout << "=== Fix-and-Optimize solver for HHCRSP ===\n";
   cout << "Instance: " << instPath << endl;
   cout << "PRGN seed: " << seed << endl;

   unique_ptr<Instance> inst(new Instance(instPath));

   cout << "Creating MIP model... " << flush;
   unique_ptr <MipModel> model(new MipModel(*inst));
   cout << "Done!" << endl;

   model->setQuiet(true);
   model->maxThreads(1);

   if (!getenv("INITIAL")) {
      cout << "Creating initial constructive solution... " << flush;
      unique_ptr<InitialRouting> iniSol(new InitialRouting(*inst));
      iniSol->solve();
      solutionCopy(*iniSol, *model);
      cout << "Done!" << endl;
   } else {
      const char *fname = getenv("INITIAL");
      cout << "Reading initial solution from " << fname << "... " << flush;
      ifstream fid(fname);
      solutionCopy(fid, *model);
      cout << "Done!" << endl;
   }

   cout << "Setting solution to MIP model..." << endl;
   model->solve();
   cout << "Done! Initial solution cost: " << model->objValue() << "." << endl;

   unique_ptr <FixAndOptimize> feoSolver(new FixAndOptimize(*model));
   feoSolver->solve(seed, (inst->numNodes()-2)/2, 25);

   cout << "\nBest solution found: " << model->objValue() << endl;
   cout << "\n" << model->objValue() << endl;

   return EXIT_SUCCESS;
}

