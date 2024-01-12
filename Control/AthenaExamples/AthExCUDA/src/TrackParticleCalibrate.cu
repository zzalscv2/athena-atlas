// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "TrackParticleCalibrate.h"

// CUDA include(s).
#include <cuda_runtime.h>

// System include(s).
#include <cassert>
#include <sstream>
#include <stdexcept>

namespace {

void throw_error(cudaError_t errorCode, const char* expression,
                 const char* file, int line) {

   // Create a nice error message.
   std::ostringstream errorMsg;
   errorMsg << file << ":" << line << " Failed to execute: " << expression
            << " (" << cudaGetErrorString(errorCode) << ")";

   // Now throw a runtime error with this message.
   throw std::runtime_error(errorMsg.str());
}

}  // private namespace

/// Helper macro used for checking @c cudaError_t type return values.
#define CUDA_ERROR_CHECK(EXP)                                             \
    do {                                                                  \
        cudaError_t errorCode = EXP;                                      \
        if (errorCode != cudaSuccess) {                                   \
            ::throw_error(errorCode, #EXP, __FILE__, __LINE__);           \
        }                                                                 \
    } while (false)

namespace AthCUDAExamples {
namespace kernels {

/// Dummy kernel performing a trivial transformation on the track particle
/// parameters.
__global__
void trackParticleCalibrate(const TrackParticleContainer::const_view input_view,
                            TrackParticleContainer::view output_view) {

   // Get the current thread's index.
   const unsigned int index = blockIdx.x * blockDim.x + threadIdx.x;

   // Create the device containers.
   TrackParticleContainer::const_device input(input_view);
   TrackParticleContainer::device output(output_view);
   assert(input.size() == output.size());

   // Check that the index is in range.
   if (index < input.size()) {
      // Copy the angle parameters as they are.
      output.theta()[index] = input.theta()[index];
      output.phi()[index] = input.phi()[index];

      // Transform the momentum in some silly way.
      output.qOverP()[index] =
         input.qOverP()[index] * std::abs((input.theta()[index] -
                                           input.phi()[index]) /
                                          input.phi()[index]);
   }

   return;
}


}  // namespace kernels

void calibrate(const TrackParticleContainer::const_view& input,
               TrackParticleContainer::view output) {

   // Launch the kernel.
   static const unsigned int block_size = 256;
   const unsigned int num_blocks =
      (input.capacity() + block_size - 1) / block_size;
   kernels::trackParticleCalibrate<<<num_blocks, block_size>>>(input, output);

   // Check for errors.
   CUDA_ERROR_CHECK(cudaGetLastError());
   CUDA_ERROR_CHECK(cudaDeviceSynchronize());
}

}  // namespace AthCUDAExamples
