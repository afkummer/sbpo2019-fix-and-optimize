#pragma once

#include <chrono>

class Timer {
public:

   inline void start() {
      m_t0 = std::chrono::high_resolution_clock::now();
   }

   inline void finish() {
      m_t1 = std::chrono::high_resolution_clock::now();
   }

   inline double elapsed() const {
      auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(m_t1-m_t0).count();
      return ms/1000.0;
   }

private:
   std::chrono::high_resolution_clock::time_point m_t0, m_t1;
};

