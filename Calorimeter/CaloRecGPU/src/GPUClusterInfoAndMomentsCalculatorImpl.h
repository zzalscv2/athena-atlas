//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_GPUCLUSTERINFOANDMOMENTSCALCULATOR_CUDA_H
#define CALORECGPU_GPUCLUSTERINFOANDMOMENTSCALCULATOR_CUDA_H

#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "CaloRecGPU/DataHolders.h"
#include "CaloRecGPU/Helpers.h"

#include "CaloRecGPU/IGPUKernelSizeOptimizer.h"

#include <cmath>

namespace ClusterMomentsCalculator
{

  struct RealSymmetricMatrixSolver
//See https://hal.science/hal-01501221/document
  {
    float a, b, c, d, e, f;
    // +--     --+
    // | a  d  f |
    // | d  b  e |
    // | f  e  c |
    // +--     --+

    constexpr static float sqr(const float & x)
    {
      return x * x;
    }

    constexpr static float cub(const float & x)
    {
      return x * x * x;
    }

    CUDA_HOS_DEV float x_1() const
    {
      return sqr(a) + sqr(b) + sqr(c) - a * b - a * c - b * c + 3.f * (sqr(d) + sqr(e) + sqr(f));
      //sqr(a - b) / 2 + sqr(a - c) / 2 + sqr(b - c) / 2 + 3 * (sqr(d) + sqr(e) + sqr(f));
      //The sum of half-squares can be (re)written as
      //sqr(a) + sqr(b) + sqr(c) - a * b - a * c - b*c
      //but I think this gives a greater numerical error
      //due to potential catastrophic cancellations.
    }

    CUDA_HOS_DEV float x_2() const
    {
      const float t1 = 2.f * a - b - c,
                  t2 = 2.f * b - a - c,
                  t3 = 2.f * c - a - b;
      return 9.f * (t1 * sqr(e) + t2 * sqr(f) + t3 * sqr(d)) - (t1 * t2 * t3) - 54.f * d * e * f;
    }

    CUDA_HOS_DEV float phi() const
    {
      const float temp = x_2();

      using namespace std;

#ifdef __CUDA_ARCH__
      return atan2f(1.0f, temp * rsqrtf(4.f * cub(x_1()) - sqr(temp)));
#else
      return atan2f(sqrtf(4.f * cub(x_1()) - sqr(temp)), temp);
#endif
    }

    CUDA_HOS_DEV float lambda_1() const
    {
      using namespace std;
      return ( a + b + c - 2.f * sqrtf(x_1()) * cosf( phi() / 3.f ) ) / 3.f;
      //Could this be simplified?
    }

    CUDA_HOS_DEV float lambda_2() const
    {
      using namespace std;
      return ( a + b + c + 2.f * sqrtf(x_1()) * cosf( (phi() - CaloRecGPU::Helpers::Constants::pi<float>) / 3.f ) ) / 3.f;
      //Could this be simplified?
    }

    CUDA_HOS_DEV float lambda_3() const
    {
      using namespace std;
      return ( a + b + c + 2.f * sqrtf(x_1()) * cosf( (phi() + CaloRecGPU::Helpers::Constants::pi<float>) / 3.f ) ) / 3.f;
      //Could this be simplified?
    }

    CUDA_HOS_DEV void get_values(float & l1, float & l2, float & l3) const
    {
      using namespace std;

      const float sum = a + b + c;
      const float troot = 2.f * sqrtf(x_1());
      const float angl = phi();

      l1 = ( sum - troot * cosf(angl / 3.f) ) / 3.f;
      l2 = ( sum + troot * cosf( (angl - CaloRecGPU::Helpers::Constants::pi<float>) / 3.f ) ) / 3.f;
      l3 = ( sum + troot * cosf( (angl + CaloRecGPU::Helpers::Constants::pi<float>) / 3.f ) ) / 3.f;

    }

    CUDA_HOS_DEV float m(const float lambda) const
    {
      return (d * (c - lambda) - e * f) / (f * (b - lambda) - d * e);
      //Probably alternative formulae
      //without catastrophic cancellation?
    }

    CUDA_HOS_DEV float m1() const
    {
      return m(lambda_1());
    }
    CUDA_HOS_DEV float m2() const
    {
      return m(lambda_2());
    }
    CUDA_HOS_DEV float m3() const
    {
      return m(lambda_3());
    }

    CUDA_HOS_DEV void get_v(float (&a)[3], const float v, const bool normalized = false) const
    {
      const float mv = m(v);
      const float first = (v - c - e * mv) / f;
      if (normalized)
        {
          using namespace std;
#ifdef __CUDA_ARCH__
          float norm = rnorm3df(first, mv, 1.f) * (first < 0.f ? -1.f : 1.f);
#else
          float norm = (first < 0.f ? -1.0f : 1.0f) / hypot(first, mv, 1.0f);
#endif
          a[0] = first * norm;
          a[1] = mv * norm;
          a[2] = norm;
        }
      else
        {
          a[0] = first;
          a[1] = mv;
          a[2] = 1.f;
        }
    }

    CUDA_HOS_DEV void get_v_1(float (&a)[3], const bool normalized = false) const
    {
      get_v(a, lambda_1(), normalized);
    }

    CUDA_HOS_DEV void get_v_2(float (&a)[3], const bool normalized = false) const
    {
      get_v(a, lambda_2(), normalized);
    }

    CUDA_HOS_DEV void get_v_3(float (&a)[3], const bool normalized = false) const
    {
      get_v(a, lambda_3(), normalized);
    }

    CUDA_HOS_DEV void get_solution_pair_1(float & lambda, float (&v)[3], const bool normalized = false)
    {
      lambda = lambda_1();
      get_v(v, lambda, normalized);
    }

    CUDA_HOS_DEV void get_solution_pair_2(float & lambda, float (&v)[3], const bool normalized = false)
    {
      lambda = lambda_2();
      get_v(v, lambda, normalized);
    }

    CUDA_HOS_DEV void get_solution_pair_3(float & lambda, float (&v)[3], const bool normalized = false)
    {
      lambda = lambda_3();
      get_v(v, lambda, normalized);
    }

    CUDA_HOS_DEV void get_full_solution( float & lambda_1, float (&v_1)[3],
                                         float & lambda_2, float (&v_2)[3],
                                         float & lambda_3, float (&v_3)[3],
                                         const bool normalized = false  ) const
    {
      get_values(lambda_1, lambda_2, lambda_3);
      get_v(v_1, lambda_1, normalized);
      get_v(v_2, lambda_2, normalized);
      get_v(v_3, lambda_3, normalized);
    }

    CUDA_HOS_DEV bool well_defined(const float la1, const float la2, const float la3, const float tolerance = 0.f) const
    {
      using namespace std;

      const float factor = d * e;
      const float real_tolerance = fabsf(factor) * tolerance;

      return ( fabsf(f) >= tolerance                            &&
               fabsf(f * (b - la1) - factor) >= real_tolerance  &&
               fabsf(f * (b - la2) - factor) >= real_tolerance  &&
               fabsf(f * (b - la3) - factor) >= real_tolerance     );
    }

    CUDA_HOS_DEV bool well_defined(const float tolerance = 0.f) const
    {
      float la1, la2, la3;
      get_values(la1, la2, la3);
      return well_defined(la1, la2, la3, tolerance);
    }

    CUDA_HOS_DEV bool ill_defined(const float la1, const float la2, const float la3, const float tolerance = 0.f) const
    {
      return !well_defined(la1, la2, la3, tolerance);
    }

    CUDA_HOS_DEV bool ill_defined(const float tolerance = 0.f) const
    {
      return !well_defined(tolerance);
    }

  };

  struct ClusterMomentCalculationOptions
  {
    bool  use_abs_energy;
    bool  use_two_gaussian_noise;
    bool  skip_invalid_clusters;
    float min_LAr_quality;
    float max_axis_angle;
    float eta_inner_wheel;
    float min_l_longitudinal;
    float min_r_lateral;
  };

  struct CMCOptionsHolder
  {
    CaloRecGPU::Helpers::CPU_object<ClusterMomentCalculationOptions> m_options;

    CaloRecGPU::Helpers::CUDA_object<ClusterMomentCalculationOptions> m_options_dev;

    void allocate();
    void sendToGPU(const bool clear_CPU = false);
  };

  void register_kernels(IGPUKernelSizeOptimizer & optimizer);

  void calculateClusterPropertiesAndMoments(CaloRecGPU::EventDataHolder & holder,
                                            const CaloRecGPU::ConstantDataHolder & instance_data,
                                            const CMCOptionsHolder & options,
                                            const IGPUKernelSizeOptimizer & optimizer,
                                            const bool synchronize = false,
                                            CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream = {},
                                            const bool defer_instead_of_oversize = false);
}
#endif