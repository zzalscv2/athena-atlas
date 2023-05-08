//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_TOPOAUTOMATONSPLITTING_CUDA_H
#define CALORECGPU_TOPOAUTOMATONSPLITTING_CUDA_H

#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "CaloRecGPU/DataHolders.h"
#include "CaloRecGPU/Helpers.h"
#include "ExtraTagDefinitions.h"

#include "FPHelpers.h"

struct TASTag : public CaloRecGPU::Tag_1_1_12_32_18
//Any %?
{
  using CaloRecGPU::Tag_1_1_12_32_18::Tag_1_1_12_32_18;

 protected:

  static constexpr uint32_t s_start_counter = 0xFFFU;
  //12 bits.

  static constexpr carrier s_tag_propagation_delta = carrier(1) << s_12_bit_offset;

  using EnergyFPFormat = FloatingPointHelpers::StandardFloat;

  [[nodiscard]] constexpr static uint32_t energy_to_storage(const uint32_t energy_pattern)
  {
    return EnergyFPFormat::template to_total_ordering<uint32_t>(energy_pattern);
  }

  [[nodiscard]] constexpr static uint32_t storage_to_energy(const uint32_t storage_pattern)
  {
    return EnergyFPFormat::template from_total_ordering<uint32_t>(storage_pattern);
  }

 public:

  [[nodiscard]] constexpr static carrier counter_delta()
  {
    return s_tag_propagation_delta;
  }

  [[nodiscard]] constexpr int32_t index() const
  {
    return this->get_18_bits();
  }

  [[nodiscard]] constexpr static int32_t index(const TASTag tag)
  {
    return tag.index();
  }

  [[nodiscard]] constexpr uint32_t energy_bits() const
  {
    return this->get_32_bits();
  }

  [[nodiscard]] constexpr static uint32_t energy_bits(const TASTag tag)
  {
    return tag.energy_bits();
  }

  [[nodiscard]] constexpr int32_t counter() const
  {
    return this->get_12_bits();
  }

  [[nodiscard]] static constexpr int32_t counter(const TASTag tag)
  {
    return tag.counter();
  }

  [[nodiscard]] static constexpr int32_t max_counter()
  {
    return s_start_counter;
  }

  [[nodiscard]] constexpr bool is_primary() const
  {
    return this->get_first_flag();
  }

  [[nodiscard]] constexpr static bool is_primary(const TASTag tag)
  {
    return tag.is_primary();
  }

  [[nodiscard]] constexpr bool is_secondary() const
  {
    return !this->is_primary();
  }

  [[nodiscard]] constexpr static bool is_secondary(const TASTag tag)
  {
    return tag.is_secondary();
  }

  /*! Expects @p maximum_energy_pattern to be the bit pattern of the float that represents the energy.
  */
  [[nodiscard]] static constexpr carrier make_maximum_tag(const int32_t index, const uint32_t maximum_energy_pattern, const bool is_primary)
  {
    return make_generic_tag(index, energy_to_storage(maximum_energy_pattern), s_start_counter, true, is_primary);
  }

  [[nodiscard]] static constexpr carrier make_original_cluster_tag(const uint16_t original_cluster_index)
  {
    return make_generic_tag(original_cluster_index, 0xFFFFFFFFU, s_start_counter, true, true);
  }

  /*! Expects @p energy_pattern to be the bit pattern of the float that represents the energy.
  */
  [[nodiscard]] static constexpr carrier make_shared_tag(const int32_t index, const uint32_t energy_pattern, const int32_t counter)
  {
    return make_generic_tag(index, energy_to_storage(energy_pattern), counter, false, true);
  }

  [[nodiscard]] static constexpr carrier make_cluster_cell_tag(const uint16_t original_cluster_index)
  {
    return make_generic_tag(original_cluster_index, 1, 0, 0, 0);
  }

 protected:

  static constexpr carrier s_last_non_assigned_tag = (s_18_bit_mask << 1) | s_18_bit_mask;

 public:

  [[nodiscard]] constexpr carrier set_primary() const
  {
    return this->set_first_flag();
  }

  [[nodiscard]] static constexpr carrier set_primary(const TASTag tag)
  {
    return tag.set_primary();
  }

  [[nodiscard]] constexpr carrier clear_primary() const
  {
    return this->unset_first_flag();
  }

  [[nodiscard]] static constexpr carrier clear_primary(const TASTag tag)
  {
    return tag.clear_primary();
  }

  [[nodiscard]] constexpr carrier set_shared() const
  {
    return this->unset_second_flag();
  }

  [[nodiscard]] static constexpr carrier set_shared(const TASTag tag)
  {
    return tag.set_shared();
  }

  [[nodiscard]] constexpr carrier clear_shared() const
  {
    return this->set_second_flag();
  }

  [[nodiscard]] static constexpr carrier clear_shared(const TASTag tag)
  {
    return tag.clear_shared();
  }

  [[nodiscard]] constexpr bool is_valid() const
  {
    return value > 0ULL;
  }

  [[nodiscard]] static constexpr bool is_valid(const TASTag tag)
  {
    return tag.is_valid();
  }

  [[nodiscard]] constexpr bool is_invalid() const
  {
    return !this->is_valid();
  }

  [[nodiscard]] static constexpr bool is_invalid(const TASTag tag)
  {
    return tag.is_invalid();
  }

  [[nodiscard]] constexpr bool is_part_of_original_cluster() const
  {
    return (value & s_32_bit_mask) == s_32_bit_mask;
    //As original cluster cells have all bits of the energy set to 1
    //(which would be a NaN for normal energy)
  }

  [[nodiscard]] static constexpr bool is_part_of_original_cluster(const TASTag tag)
  {
    return tag.is_part_of_original_cluster();
  }

  [[nodiscard]] constexpr bool is_part_of_splitter_cluster() const
  {
    return value > s_last_non_assigned_tag && !this->is_part_of_original_cluster();
  }

  [[nodiscard]] static constexpr bool is_part_of_splitter_cluster(const TASTag tag)
  {
    return tag.is_part_of_splitter_cluster();
  }

  [[nodiscard]] constexpr bool is_shared() const
  {
    return !(this->get_second_flag());
  }

  [[nodiscard]] constexpr static bool is_shared(const TASTag tag)
  {
    return tag.is_shared();
  }

  [[nodiscard]] constexpr bool is_non_assigned_part_of_split_cluster() const
  {
    return value > 0 && value <= s_last_non_assigned_tag;
  }

  [[nodiscard]] static constexpr bool is_non_assigned_part_of_split_cluster(const TASTag tag)
  {
    return tag.is_non_assigned_part_of_split_cluster();
  }

  [[nodiscard]] constexpr carrier propagate() const
  {
    return (value - s_tag_propagation_delta) & (~s_first_flag_mask);
  }

  [[nodiscard]] static constexpr carrier propagate(const TASTag tag)
  {
    return tag.propagate();
  }

  [[nodiscard]] constexpr carrier update_cell(const uint32_t new_index, const uint32_t new_energy)
  {
    return make_generic_tag(new_index, energy_to_storage(new_energy), 0, 0, 0) | (value & ~(s_18_bit_mask | s_32_bit_mask));
  }

  [[nodiscard]] constexpr carrier update_index(const uint32_t new_index) const
  {
    return (value & (~s_18_bit_mask)) | ( (carrier(new_index) << s_18_bit_offset) & s_18_bit_mask);
  }

  [[nodiscard]] constexpr carrier update_counter(const uint32_t new_counter) const
  {
    return (value & (~s_12_bit_mask)) | (carrier(new_counter) << s_12_bit_offset);
  }

  [[nodiscard]] constexpr carrier ordering_bits() const
  {
    return value & (s_32_bit_mask | s_18_bit_mask);
  }

  [[nodiscard]] static constexpr carrier ordering_bits(const TASTag tag)
  {
    return tag.ordering_bits();
  }

  [[nodiscard]] constexpr carrier prepare_for_sharing(const TASTag other_tag) const
  {
    return (other_tag & (~s_second_flag_mask)) | s_first_flag_mask | s_12_bit_mask;
  }

 protected:

  static constexpr carrier s_eliminated_secondary_tag = 0xFFFFFFFFFFFFFFFFULL;

 public:

  /*! Just to be semantically clearer for the maxima exclusion.
  */
  [[nodiscard]] constexpr bool is_secondary_maximum_seed() const
  {
    return this->is_part_of_splitter_cluster() && this->is_secondary();
  }

  [[nodiscard]] constexpr static bool is_secondary_maximum_seed(const TASTag tag)
  {
    return tag.is_secondary_maximum_seed();
  }

  [[nodiscard]] static constexpr carrier secondary_maxima_eliminator()
  {
    return s_eliminated_secondary_tag;
  }

  [[nodiscard]] constexpr bool is_secondary_maxima_eliminator() const
  {
    return value == s_eliminated_secondary_tag;
  }

  [[nodiscard]] static constexpr bool is_secondary_maxima_eliminator(const TASTag tag)
  {
    return tag.is_secondary_maxima_eliminator();
  }

};

struct TopoAutomatonSplittingTemporaries
{
  CaloRecGPU::tag_type secondaryArray[CaloRecGPU::NCaloCells];
  //Temporarily used as an int array
  //to find local maxima
  //and as a float array to hold
  //cluster (absolute) energies
  //for the shared cell weighting.

  template <class T>
  constexpr T * get_extra_array()
  {
    static_assert(sizeof(T) <= sizeof(CaloRecGPU::tag_type), "Can only get extra arrays of sufficiently small types...");

    void * vptr = (void *) secondaryArray;
    return ((T *) vptr);
    //I know, I know.
    //Should be safe given the lack of padding, though,
    //and given that the malloc'd memory
    //is cast from void * anyway...
    //We avoid using reinterpret_cast
    //and employ direct C-style casting instead
    //to prevent compilers from complaining about
    //it not being constexpr-safe.
    //And we need to declare as constexpr
    //to automatically get it on the GPU
    //without the usual shenanigans
    //while keeping compatibility with non-CUDA code.
  }

  template <class T>
  constexpr const T * get_extra_array() const
  {
    static_assert(sizeof(T) <= sizeof(CaloRecGPU::tag_type), "Can only get extra arrays of sufficiently small types...");

    void * vptr = (void *) secondaryArray;
    return ((const T *) vptr);
  }

  template <class T>
  constexpr T & get_extra_array(const int i)
  {
    return this->get_extra_array<T>()[i];
  }

  template <class T>
  constexpr const T & get_extra_array(const int i) const
  {
    return this->get_extra_array<T>()[i];
  }

  template <class T>
  constexpr T * get_second_extra_array()
  {
    static_assert(sizeof(T) * 2 <= sizeof(CaloRecGPU::tag_type), "Can only get extra arrays of sufficiently small types...");

    void * vptr = (void *) secondaryArray;
    return ((T *) vptr) + CaloRecGPU::NCaloCells;
  }

  template <class T>
  constexpr const T * get_second_extra_array() const
  {
    static_assert(sizeof(T) * 2 <= sizeof(CaloRecGPU::tag_type), "Can only get extra arrays of sufficiently small types...");

    void * vptr = (void *) secondaryArray;
    return ((const T *) vptr) + CaloRecGPU::NCaloCells;
  }

  template <class T>
  constexpr T & get_second_extra_array(const int i)
  {
    return this->get_second_extra_array<T>()[i];
  }

  template <class T>
  constexpr const T & get_second_extra_array(const int i) const
  {
    return this->get_second_extra_array<T>()[i];
  }

  template <size_t i = 0>
  constexpr const float * get_cluster_property_aux_array() const
  {
    static_assert((i + 1) * sizeof(float) * CaloRecGPU::NMaxClusters <= sizeof(TopoAutomatonSplittingTemporaries),
                  "Cannot access outside of secondaryArray bounds...");

    return this->get_extra_array<float>() + i * CaloRecGPU::NMaxClusters;
  }

  template <size_t i = 0>
  constexpr float * get_cluster_property_aux_array()
  {
    static_assert((i + 1) * sizeof(float) * CaloRecGPU::NMaxClusters <= sizeof(TopoAutomatonSplittingTemporaries),
                  "Cannot access outside of secondaryArray bounds...");

    return this->get_extra_array<float>() + i * CaloRecGPU::NMaxClusters;
  }

  template <size_t i = 0>
  constexpr const float & get_cluster_property_aux_array(const int j) const
  {
    return this->get_cluster_property_aux_array<i>()[j];
  }

  template <size_t i = 0>
  constexpr float & get_cluster_property_aux_array(const int j)
  {
    return this->get_cluster_property_aux_array<i>()[j];
  }

};

struct TopoAutomatonSplittingOptions
{
  unsigned int valid_sampling_primary;
  unsigned int valid_sampling_secondary;

  int min_num_cells;
  float min_maximum_energy;

  float EM_shower_scale;

  bool share_border_cells;
  bool use_absolute_energy;
  bool treat_L1_predicted_as_good;

  bool limit_HECIW_and_FCal_neighs;
  bool limit_PS_neighs;
  //WARNING: the CPU version of the algorithm does not seem to have this option. Given the description,
  //         maybe it makes some sense to still allow this here? In our configuration we'll keep it disabled, but...

  unsigned int neighbour_options;

  constexpr bool uses_primary_sampling(const unsigned int sampling) const
  {
    return (valid_sampling_primary >> sampling) & 1U;
  }
  constexpr bool uses_secondary_sampling(const unsigned int sampling) const
  {
    return (valid_sampling_secondary >> sampling) & 1U;
  }
  constexpr bool uses_sampling(const unsigned int sampling) const
  {
    return uses_primary_sampling(sampling) || uses_secondary_sampling(sampling);
  }
};

struct TASOptionsHolder
{
  CaloRecGPU::Helpers::CPU_object<TopoAutomatonSplittingOptions> m_options;

  CaloRecGPU::Helpers::CUDA_object<TopoAutomatonSplittingOptions> m_options_dev;

  void allocate();
  void sendToGPU(const bool clear_CPU = false);
};

void fillNeighbours(CaloRecGPU::EventDataHolder & holder,
                    CaloRecGPU::Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temps,
                    const CaloRecGPU::ConstantDataHolder & instance_data,
                    const TASOptionsHolder & options, const bool synchronize = false,
                    CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream_to_use = {});

void findLocalMaxima(CaloRecGPU::EventDataHolder & holder,
                     CaloRecGPU::Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temps,
                     const CaloRecGPU::ConstantDataHolder & instance_data,
                     const TASOptionsHolder & options, const bool synchronize = false,
                     CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream_to_use = {});

void excludeSecondaryMaxima(CaloRecGPU::EventDataHolder & holder,
                            CaloRecGPU::Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temps,
                            const CaloRecGPU::ConstantDataHolder & instance_data,
                            const TASOptionsHolder & options, const bool synchronize = false,
                            CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream_to_use = {});

void splitClusterGrowing(CaloRecGPU::EventDataHolder & holder,
                         CaloRecGPU::Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temps,
                         const CaloRecGPU::ConstantDataHolder & instance_data,
                         const TASOptionsHolder & options, const bool synchronize = false,
                         CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream_to_use = {});

void cellWeightingAndFinalization(CaloRecGPU::EventDataHolder & holder,
                                  CaloRecGPU::Helpers::CUDA_kernel_object<TopoAutomatonSplittingTemporaries> temps,
                                  const CaloRecGPU::ConstantDataHolder & instance_data,
                                  const TASOptionsHolder & options, const bool synchronize = false,
                                  CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream_to_use = {});

#endif //CALORECGPU_TOPOAUTOMATONSPLITTING_CUDA_H
