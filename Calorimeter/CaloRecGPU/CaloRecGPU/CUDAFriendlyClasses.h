/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//Dear emacs, this is -*-c++-*-

#ifndef CALORECGPU_CUDAFRIENDLYCLASSES_H
#define CALORECGPU_CUDAFRIENDLYCLASSES_H

#include <cstddef>
#include <iostream>
#include <limits>

//#include "CaloRecGPU/Helpers.h"

namespace CaloRecGPU
{

  constexpr inline int NMaxNeighbours = 26;
  constexpr inline int NCaloCells = 187652;
  //Max index will be 0x0002 DD03
  constexpr inline int NMaxClusters = 0x10000;
  //So it all fits in an int16_t
  //Would only start getting fishy if all the cells
  //were part of a cluster and all the clusters
  //had an average number of cells lower than 2.87...
  //(Or, in general, for N cells not part of a cluster,
  // an average number of cells per cluster lower than 2.87 - N/65536...)

  constexpr inline int NMaxPairs = NMaxNeighbours * NCaloCells;

  constexpr inline int NumGainStates = 4;
  //This is the number of different gain states a cell can have.
  //(For the noise calculation.)

  //Migrated from macro defines to constexpr, for type safety.

  using tag_type = unsigned long long int;
  //uint64_t is unsigned long long int in CUDA.

  class Tags
  {
   protected:

    constexpr static tag_type tag_counter_mask = 0x7FFF800000000000ULL;
    //0x 7FFF 8000 0000 0000

    constexpr static tag_type tag_snr_mask = 0x00007FFFFFFF0000ULL;
    //0x 0000 7FFF FFFF 0000

    constexpr static tag_type tag_index_mask = 0xFFFFULL;

    constexpr static tag_type tag_propagation_delta = 0x0000800000000000ULL;
    //0x 0000 8000 0000 0000

   public:

    constexpr static tag_type ValidPropagationMark = 0x8000000000000000ULL;
    //0x 8000 0000 0000 0000

    constexpr static tag_type GrowTag = ValidPropagationMark;

    constexpr static tag_type TerminalTag = 0x1000000000000000ULL;
    //0x 1000 0000 0000 0000

    constexpr static tag_type InvalidTag = tag_propagation_delta;
    //0x 0000 8000 0000 0000

    inline static constexpr bool is_invalid(const tag_type tag)
    {
      return tag <= InvalidTag;
    }

    inline static constexpr bool is_valid(const tag_type tag)
    {
      return !is_invalid(tag);
    }

    inline static constexpr bool is_assignable_terminal(const tag_type tag)
    {
      return tag >= TerminalTag && tag < GrowTag;
    }

    inline static constexpr bool is_non_assigned_terminal(const tag_type tag)
    {
      return tag == TerminalTag;
    }

    inline static constexpr bool is_assigned_terminal(const tag_type tag)
    {
      return tag > TerminalTag && tag < GrowTag;
    }

    inline static constexpr bool is_terminal(const tag_type tag)
    {
      return tag >= TerminalTag && tag < GrowTag;
    }

    inline static constexpr bool is_non_assigned_growing(const tag_type tag)
    {
      return tag == GrowTag;
    }

    inline static constexpr bool is_growing_or_seed(const tag_type tag)
    {
      return tag >= GrowTag;
    }

    inline static constexpr bool is_part_of_cluster(const tag_type tag)
    {
      return tag > GrowTag;
    }

    inline static constexpr tag_type make_seed_tag(const int32_t snr_as_int, const int32_t /*seed_cell_index*/, const int32_t cluster_index)
    //This works because the numbers will always be non-negative when we make a signature.
    {
      constexpr uint32_t two_byte_mask = 0xFFFFU;
      //By design, the numbers should be clamped to acceptable values.
      //However, no harm in preventing explicit clobbering?
      //Might aid in optimizations, upon naive check on Godbolt/CUDA,
      //due to the existance of "bfi" instructions that do bit-wise insertion.

      const uint32_t index = uint32_t(cluster_index & two_byte_mask);

      tag_type ret = two_byte_mask;


      ret = (ret << 31) | snr_as_int;

      ret = (ret << 16) | index;

      return ret | ValidPropagationMark;
    }

    inline static constexpr int32_t get_snr_from_tag(const tag_type tag)
    {
      const tag_type masked = tag & tag_snr_mask;

      const uint32_t ret = (masked >> 16);

      return ret;
    }

    inline static constexpr int32_t get_index_from_tag(const tag_type tag)
    {
      const uint32_t ret = tag & tag_index_mask;

      return ret;
    }

    inline static constexpr int32_t get_counter_from_tag(const tag_type tag)
    {
      const tag_type masked = tag & tag_counter_mask;

      const uint32_t ret = (masked >> 47);

      return ret;

    }

    inline static constexpr tag_type clear_counter(const tag_type tag)
    {
      return tag & (~tag_counter_mask);
    }

    inline static constexpr tag_type clear_everything_but_counter(const tag_type tag)
    {
      return tag & tag_counter_mask;
    }

    inline static constexpr tag_type set_for_propagation(const tag_type tag)
    {
      return tag - tag_propagation_delta;
    }

    inline static constexpr tag_type set_for_terminal_propagation(const tag_type tag)
    {
      return tag - tag_propagation_delta;
    }

    inline static constexpr tag_type update_non_terminal_tag(const tag_type old_tag, const tag_type new_tag)
    {
      return clear_everything_but_counter(old_tag) | new_tag;
    }

    inline static constexpr tag_type update_terminal_tag(const tag_type old_tag, const tag_type new_tag)
    {
      return clear_everything_but_counter(old_tag) | new_tag;
    }

    inline static constexpr tag_type terminal_to_seed_tag(const tag_type old_tag)
    {
      return old_tag;
    }
  };



  /*
  //The following definitions are used to signal certain cells through their energy
  //as being ineligible for being seeds, or for being part of a cluster,
  //if the cells are being cut according to time (option m_seedCutsInT)

  class EnergyManip
  {
   protected:

    constexpr static int invalid_seed_min_exponent = 64;
    constexpr static int invalid_seed_factor_exponent = 96;

    constexpr static float invalid_seed_min = Helpers::compile_time_pow2<float, int>(invalid_seed_min_exponent);
    constexpr static float invalid_seed_factor = Helpers::compile_time_pow2<float, int>(invalid_seed_factor_exponent);

    constexpr static float invalid_for_all_value = std::numeric_limits<float>::max();

    //Cheap trick to allow these functions to be called from a CUDA context
    //without actually placing __host__ __device__ decorations
    //that would be invalid in C++ and cause different signature shenanigans:
    //given that constexpr functions can be called from device code
    //with exprt-relaxed-constexpr,
    //we shall declare them as such even if they are not used in such a way.
   public:

    inline static constexpr bool is_ineligible_for_any(const float energy)
    {
      return (energy == invalid_for_all_value);
    }

    inline static constexpr bool is_ineligible_for_seed(const float energy)
    //Might be ineligible for other things as well.
    {
      using namespace std;
      return abs(energy) > invalid_seed_min;
    }

    inline static constexpr bool is_valid_cell(const float energy)
    {
      using namespace std;
      return abs(energy) < invalid_seed_min;
    }

    inline static constexpr float correct_energy(const float energy)
    //May change bad and invalid for all cell energies too...
    {
      if (is_ineligible_for_seed(energy))
        {
          return energy / invalid_seed_factor;
        }
      else
        {
          return energy;
        }
    }

    inline static constexpr void mark_ineligible_for_seed(float & energy)
    {
      energy *= invalid_seed_factor;
    }

    inline static constexpr void mark_ineligible_for_all(float & energy)
    {
      energy = invalid_for_all_value;
    }

    //Note: the fact that we use abs(energy) to classify this validity
    //      does not change the results when we use the signed energy
    //      for the cluster growing in itself: as energy < 0 means
    //      the cells will be invalid, they will continue being invalid too...
  };

  */


  struct CellNoiseArr
  {
    float noise[NumGainStates][NCaloCells];
  };

  class GainConversion
  {
   protected:

    enum class GainType : int
    {
      TileLowLow = 0, TileLowHigh = 1, TileHighLow = 2, TileHighHigh = 3,
      TileOneLow = 0, TileOneHigh = 3, //Are these valid/used?
      LArHigh = 0, LArMedium = 1, LArLow = 2,
      DefaultOrInvalid = 0, //A sane value just to ensure no out-of-bounds reading.
      BadCell = 5,
      InvalidCell = 6,
      InvalidSeedInv0 = -1,
      InvalidSeedInv1 = -2,
      InvalidSeedInv2 = -3,
      InvalidSeedInv3 = -4,
      GainMinValue = -4,
      GainMaxValue = 6,
      NumGainValues = GainMaxValue - GainMinValue + 1

    };

   public:

    template <class T = int>
    inline static constexpr T max_gain_value()
    {
      return (T) GainType::GainMaxValue;
    }

    template <class T = int>
    inline static constexpr T min_gain_value()
    {
      return (T) GainType::GainMinValue;
    }

    template <class T = int>
    inline static constexpr T num_gain_values()
    {
      return (T) GainType::NumGainValues;
    }

    inline static constexpr GainType from_standard_gain(const int gain)
    //Basically, CaloCondUtils::getDbCaloGain without the Athena logging.
    {
      switch (gain)
        {
          case -16: //Tile LOWLOW
            return GainType::TileLowLow;
          case -15: //Tile LOWHIGH
            return GainType::TileLowHigh;
          case -12: //Tile HIGHLOW
            return GainType::TileHighLow;
          case -11: //Tile HIGHHIGH
            return GainType::TileHighHigh;
          case -4 : //Tile ONELOW
            return GainType::TileOneLow;
          case -3 : //Tile ONEHIGH
            return GainType::TileOneHigh;
          case 0  : //LAr High
            return GainType::LArHigh;
          case 1  : //LAr Medium
            return GainType::LArMedium;
          case 2  : //Lar Low
            return GainType::LArLow;
          default:
            return GainType::DefaultOrInvalid;
        }
    }

    inline static constexpr void mark_bad_cell(GainType & gain)
    {
      gain = GainType::BadCell;
    }

    //This somewhat defeats the point of the enum class, but...
    template <class T>
    inline static constexpr void mark_bad_cell(T & gain_rep)
    {
      gain_rep = (T) GainType::BadCell;
    }

    inline static constexpr void mark_invalid_cell(GainType & gain)
    {
      gain = GainType::InvalidCell;
    }

    //Again, enum class sidestepping...
    template <class T>
    inline static constexpr void mark_invalid_cell(T & gain_rep)
    {
      gain_rep = (T) GainType::InvalidCell;
    }

    inline static constexpr void mark_invalid_seed_cell(GainType & gain)
    {
      const int ermediate = (int) gain;

      gain = (GainType) (-ermediate - 1);
    }

    template <class T>
    inline static constexpr void mark_invalid_seed_cell(T & gain_rep)
    {
      gain_rep = -gain_rep - 1;
    }

    inline static constexpr bool is_bad_cell(const GainType & gain)
    {
      return gain == GainType::BadCell;
    }

    template <class T>
    inline static constexpr bool is_bad_cell(const T & gain_rep)
    {
      return gain_rep == (T) GainType::BadCell;
    }

    inline static constexpr bool is_invalid_cell(const GainType & gain)
    {
      return gain == GainType::InvalidCell;
    }

    template <class T>
    inline static constexpr bool is_invalid_cell(const T & gain_rep)
    {
      return gain_rep == (T) GainType::InvalidCell;
    }

    inline static constexpr bool is_invalid_seed_cell(const GainType & gain)
    {
      return ((int) gain < 0);
    }

    template <class T>
    inline static constexpr bool is_invalid_seed_cell(const T & gain_rep)
    {
      return gain_rep < 0;
    }

    template <class T>
    inline static constexpr bool is_normal_cell(const T & gain_maybe_rep)
    {
      return !is_invalid_cell(gain_maybe_rep) && !is_invalid_seed_cell(gain_maybe_rep) && !is_bad_cell(gain_maybe_rep);
    }

    inline static constexpr GainType recover_invalid_seed_cell_gain(const GainType & gain)
    {
      switch (gain)
        {
          case GainType::InvalidSeedInv0:
            return (GainType) 0;
          case GainType::InvalidSeedInv1:
            return (GainType) 1;
          case GainType::InvalidSeedInv2:
            return (GainType) 2;
          case GainType::InvalidSeedInv3:
            return (GainType) 3;
          default:
            break;
        }
      return gain;
    }

    template <class T>
    inline static constexpr T recover_invalid_seed_cell_gain(const T & gain_rep)
    {
      if (gain_rep < 0)
        {
          return -(gain_rep + 1);
        }
      return gain_rep;
    }

  };

  struct CellInfoArr
  {
    float energy[NCaloCells];
    signed char gain[NCaloCells];
  };


  struct GeometryArr
  {
    int    caloSample[NCaloCells];
    float  x[NCaloCells];
    float  y[NCaloCells];
    float  z[NCaloCells];
    float  eta[NCaloCells];
    float  phi[NCaloCells];

    int nNeighbours[NCaloCells];               //Number of neighbours
    int neighbours[NCaloCells][NMaxNeighbours]; //NMaxNeighbours = 26
    //Cell A will appear in neighbours[i] if
    //i appears in the result of the CPU get_neighbours of cell A

    int nReverseNeighbours[NCaloCells];               //Number of neighbours
    int reverseNeighbours[NCaloCells][NMaxNeighbours]; //NMaxNeighbours = 26
    //These last two represent the reverse of what we use,
    //but which corresponds to the same as the neighbours in the CPU:
    //Cell A will appear in neighbours[i] if
    //A appears in the result of the CPU get_neighbours of cell i...

    //This last one is needed to support the GPU splitter in its current implementation.
  };

  struct CellStateArr
  {
    tag_type clusterTag[NCaloCells]; //cluster tag
  };

  struct PairsArr
  {
    int number;
    int reverse_number;
    //This is to store neighbours in the other way around...

    int cellID[NMaxPairs];
    //This is guaranteed growing or seed cell
    int neighbourID[NMaxPairs];
    //This is any cell
  };

  struct ClusterInfoArr
  {
    int number;
    float clusterEnergy[NMaxClusters];
    float clusterEt[NMaxClusters];
    //Also used, as an intermediate value, to store AbsE
    float clusterEta[NMaxClusters];
    float clusterPhi[NMaxClusters];
    int seedCellID[NMaxClusters];
  };
  
}

#endif //CALORECGPU_CUDAFRIENDLYCLASSES_H
