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

#include <cmath>

template <class T>
struct RealSymmetricMatrixSolver
//See https://hal.science/hal-01501221/document
{
  T a, b, c, d, e, f;
  // +--     --+
  // | a  d  f |
  // | d  b  e |
  // | f  e  c |
  // +--     --+

  constexpr static T sqr(const T & x)
  {
    return x * x;
  }

  constexpr static T cub(const T & x)
  {
    return x * x * x;
  }

  constexpr T x_1() const
  {
    return sqr(a) + sqr(b) + sqr(c) - a * b - a * c - b * c + 3 * (sqr(d) + sqr(e) + sqr(f));
    //sqr(a - b) / 2 + sqr(a - c) / 2 + sqr(b - c) / 2 + 3 * (sqr(d) + sqr(e) + sqr(f));
    //The sum of half-squares can be (re)written as
    //sqr(a) + sqr(b) + sqr(c) - a * b - a * c - b*c
    //but I think this gives a greater numerical error
    //due to potential catastrophic cancellations.
  }

  constexpr T x_2() const
  {
    const T t1 = 2 * a - b - c,
            t2 = 2 * b - a - c,
            t3 = 2 * c - a - b;
    return 9 * (t1 * sqr(e) + t2 * sqr(f) + t3 * sqr(d)) - (t1 * t2 * t3) - 54 * d * e * f;
  }

  CUDA_HOS_DEV T phi() const
  {
    const T temp = x_2();
    using namespace std;
    //This allows correct square root
    //on CPU and CUDA without casting to double.
    return atan2(sqrt(4 * cub(x_1()) - sqr(temp)), temp);
  }

  CUDA_HOS_DEV T lambda_1() const
  {
    using namespace std;
    return ( a + b + c - 2 * sqrt(x_1()) * cos( phi() / 3 ) ) / 3;
    //Could this be simplified?
  }

  CUDA_HOS_DEV T lambda_2() const
  {
    using namespace std;
    return ( a + b + c + 2 * sqrt(x_1()) * cos( (phi() - CaloRecGPU::Helpers::Constants::pi<T>) / 3 ) ) / 3;
    //Could this be simplified?
  }

  CUDA_HOS_DEV T lambda_3() const
  {
    using namespace std;
    return ( a + b + c + 2 * sqrt(x_1()) * cos( (phi() + CaloRecGPU::Helpers::Constants::pi<T>) / 3 ) ) / 3;
    //Could this be simplified?
  }

  template <class OtherT>
  CUDA_HOS_DEV void get_values(OtherT & l1, OtherT & l2, OtherT & l3) const
  {
    using namespace std;

    const T sum = a + b + c;
    const T troot = 2 * sqrt(x_1());
    const T angl = phi();

    l1 = ( sum - troot * cos(angl / 3) ) / 3;
    l2 = ( sum + troot * cos( (angl - CaloRecGPU::Helpers::Constants::pi<T>) / 3 ) ) / 3;
    l3 = ( sum + troot * cos( (angl + CaloRecGPU::Helpers::Constants::pi<T>) / 3 ) ) / 3;

  }

  constexpr T m(const T lambda) const
  {
    return (d * (c - lambda) - e * f) / (f * (b - lambda) - d * e);
    //Probably alternative formulae
    //without catastrophic cancellation?
  }

  constexpr T m_1() const
  {
    return m(lambda_1());
  }
  constexpr T m_2() const
  {
    return m(lambda_2());
  }
  constexpr T m_3() const
  {
    return m(lambda_3());
  }

  template <class OtherT>
  constexpr void get_v(OtherT (&a)[3], const T v, const bool normalized = false) const
  {
    const T m_v = m(v);
    a[0] = (v - c - e * m_v) / f;
    a[1] = m_v;
    a[2] = 1;
    if (normalized)
    {
      using namespace std;
      T norm = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
      a[0] /= norm;
      a[1] /= norm;
      a[2] /= norm;
    }
    
  }

  template <class OtherT>
  constexpr void get_v_1(OtherT (&a)[3], const bool normalized = false) const
  {
    get_v(a, lambda_1(), normalized);
  }
  template <class OtherT>
  constexpr void get_v_2(OtherT (&a)[3], const bool normalized = false) const
  {
    get_v(a, lambda_2(), normalized);
  }
  template <class OtherT>
  constexpr void get_v_3(OtherT (&a)[3], const bool normalized = false) const
  {
    get_v(a, lambda_3(), normalized);
  }

  template <class OtherT>
  constexpr void get_solution_pair_1(OtherT & lambda, OtherT (&v)[3], const bool normalized = false)
  {
    lambda = lambda_1();
    get_v(v, lambda, normalized);
  }
  template <class OtherT>
  constexpr void get_solution_pair_2(OtherT & lambda, OtherT (&v)[3], const bool normalized = false)
  {
    lambda = lambda_2();
    get_v(v, lambda, normalized);
  }
  template <class OtherT>
  constexpr void get_solution_pair_3(OtherT & lambda, OtherT (&v)[3], const bool normalized = false)
  {
    lambda = lambda_3();
    get_v(v, lambda, normalized);
  }

  template <class OtherT>
  constexpr void get_full_solution( OtherT & lambda_1, OtherT (&v_1)[3],
                                    OtherT & lambda_2, OtherT (&v_2)[3],
                                    OtherT & lambda_3, OtherT (&v_3)[3],
                                    const bool normalized = false  ) const
  {
    get_values(lambda_1, lambda_2, lambda_3);
    get_v(v_1, lambda_1, normalized);
    get_v(v_2, lambda_2, normalized);
    get_v(v_3, lambda_3, normalized);
  }

  constexpr bool ill_defined(const T la1, const T la2, const T la3) const
  {
    return ( f == 0                  ||
             f * (b - la1) == d * e  ||
             f * (b - la2) == d * e  ||
             f * (b - la3) == d * e     );
  }

  constexpr bool ill_defined() const
  {
    T la1, la2, la3;
    get_values(la1, la2, la3);
    return ill_defined(la1, la2, la3);
  }

  constexpr bool well_defined(const T la1, const T la2, const T la3) const
  {
    return !ill_defined(la1, la3, la3);
  }

  constexpr bool well_defined() const
  {
    return !ill_defined();
  }

};

struct ClusterMomentCalculationOptions
{
  bool use_abs_energy;
  bool use_two_gaussian_noise;
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
  void sendToGPU(const bool clear_CPU = true);
};

void calculateClusterPropertiesAndMoments(CaloRecGPU::EventDataHolder & holder, const CaloRecGPU::ConstantDataHolder & instance_data,
                                          const CMCOptionsHolder & options, const bool synchronize = false, CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream = {});

#endif