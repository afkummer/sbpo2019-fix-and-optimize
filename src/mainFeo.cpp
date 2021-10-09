#include "FixAndOptimize.h"
#include "Instance.h"
#include "InitialRouting.h"
#include "Timer.h"

#include <memory>
#include <random>

using namespace std;

int main(int argc, char **argv) {
   if (argc != 5) {
      cout << "Usage: " << argv[0] << " <instance> <seed> <tl> <tl iter>" << endl;
      return EXIT_FAILURE;
   }

   const int seed = stol(argv[2]);
   const int maxSeconds = stoi(argv[3]);
   const int maxIterSecs = stoi(argv[4]);

   cout << "#################################\n";
   cout << "### Fix & Optimize for HHCRSP ###\n";
   cout << "#################################\n";
   cout << "-- Instance: " << argv[1] << "\n";
   cout << "-- Seed: " << seed << "\n";
   cout << "-- Time limit: " << maxSeconds << " sec.\n";
   cout << "-- Subproblem TL: " << maxIterSecs << " sec.\n";
   cout << "-- Stop criteria: " << (maxIterSecs == -1 ? "Iterations without improvement" : "Time limit") << endl;

   try {
      unique_ptr<Instance> inst(new Instance(argv[1]));
      unique_ptr<FixAndOptimize> mip(new FixAndOptimize(*inst));

      // mip->setQuiet(true);
      {
         cout << "\nSolving initial routing.\n";
         
         // Option 1: mankowska initial solution algorithm.
         cout << "Initial routing algorithm: Mankowska et al. (2014).\n";
         unique_ptr<InitialRouting> iniRouter(new InitialRouting(*inst));
         iniRouter->solve();
         iniRouter->copyTo(*mip);
      }

      mip->solve(seed, maxSeconds, maxIterSecs);

      mip->printCostSummary();
   } catch (IloException &e) {
      cout << "IloException: " << e.getMessage() << endl;
      return EXIT_FAILURE;
   } catch (...) {
      cout << "Unknown exception thrown." << endl;
   }
   return EXIT_SUCCESS;
}

