#include <fstream>

/**
 * Get current memory used by the process, in megabytes.
 *
 * Extracts the data from the prof filesystem.
 *
 */
extern inline float memUsage() {
   std::ifstream fid("/proc/self/statm");
   long mem;
   fid >> mem;
   return mem/1024.0;
}

