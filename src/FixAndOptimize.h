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

#pragma once

#include "MipModel.h"

#include <random>

class FixAndOptimize {
public:

   enum class DecompMethod: int {
      RANDOM = 0,
      GUIDED = 1,
      MAX_ = 2
   };

   FixAndOptimize(MipModel &model);
   virtual ~FixAndOptimize();

   void solve(const int seed, const int maxIterNoImpr, const int maxIterSeconds);

private:
   const Instance &m_inst;
   MipModel &m_model;

   std::mt19937_64 m_prng;

   DecompMethod m_currentDecomp;
   std::string m_currentDecompName;
   int m_vehiDecomp[2];

   void chooseDecomp();
   void selectDecompVehicles();
};
