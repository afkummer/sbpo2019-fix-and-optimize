#include "Instance.h"
#include "MipModel.h"

#include <iostream>
#include <cstdlib>

int main(int argc, char **argv) {
   if (argc != 3) {
      std::cout << "Usage: " << argv[0] << " <1:instance path> <2: PRNG seed>" << std::endl;
      return EXIT_FAILURE;
   }

   Instance *inst = new Instance(argv[1]);
   const long seed = std::stol(argv[2]);

   MipModel *model = new MipModel(*inst);
   model->writeLp("model.lp");
   delete model;

   return EXIT_SUCCESS;
}

