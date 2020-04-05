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

   cout << "=== Fix-and-Optimize solver for HHCRSP ===\n";
   cout << "Instance: " << argv[1] << endl;
   cout << "PRGN seed: " << argv[2] << endl;
   cout << endl;

   unique_ptr<Instance> inst(new Instance(argv[1]));
   const long seed = std::stol(argv[2]);

   cout << "Creating MIP model... " << flush;
   unique_ptr <MipModel> model(new MipModel(*inst));
   cout << "Done!" << endl;

   cout << "Creating initial solution... " << flush;
   unique_ptr<InitialRouting> iniSol(new InitialRouting(*inst));
   iniSol->solve();
   cout << "Done!" << endl;

   cout << "Setting solution to MIP model..." << endl;
   solutionCopy(*iniSol, *model);
   model->solve();
   model->unfixSolution();
   cout << "Done! Initial solution cost: " << model->objValue() << "." << endl;



   return EXIT_SUCCESS;
}

