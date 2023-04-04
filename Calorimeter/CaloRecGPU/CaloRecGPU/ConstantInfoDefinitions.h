//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_CONSTANTINFODEFINITIONS_H
#define CALORECGPU_CONSTANTINFODEFINITIONS_H

#include "BaseDefinitions.h"
#include <climits>
//For CHAR_BIT

namespace CaloRecGPU
{

  constexpr inline int NumNeighOptions = 12;
  //The non-combined choices for LArNeighbours::neighbourOption:
  //prevInPhi      = 0x0001,  -> starts at 0
  //nextInPhi      = 0x0002,  -> starts at NeighOffsets.get_number(0)
  //prevInEta      = 0x0004,  -> starts at NeighOffsets.get_number(1)
  //nextInEta      = 0x0008,  -> starts at NeighOffsets.get_number(2)
  //corners2D      = 0x0010,  -> starts at NeighOffsets.get_number(3)
  //prevInSamp     = 0x0020,  -> starts at NeighOffsets.get_number(4)
  //nextInSamp     = 0x0040,  -> starts at NeighOffsets.get_number(5)
  //prevSubDet     = 0x0080,  -> starts at NeighOffsets.get_number(6)
  //nextSubDet     = 0x0100,  -> starts at NeighOffsets.get_number(7)
  //corners3D      = 0x0200,  -> starts at NeighOffsets.get_number(8)
  //prevSuperCalo  = 0x0400,  -> starts at NeighOffsets.get_number(9)
  //nextSuperCalo  = 0x0800   -> starts at NeighOffsets.get_number(10)

  /*! @class NeighOffsets
      A class that expresses the offset from the beginning of the neighbours list
      for the several neighbour options.

    Bit packed to store all cumulative offsets,
    since they fit in 5 bits. (NMaxNeighbours == 26)
    For a 64-bit integer, the last 4 bits are empty.
    We use the highest of those to signal that the cell
    fulfills the conditions for its neighbourhood relations
    to be reduced to `nextInSamp`, as used in the clustering algorithms.
  */
  struct NeighOffsets
  {
    using carrier = unsigned long long int;
    carrier value;

   protected:

    constexpr static carrier s_numbers_to_keep = NumNeighOptions - 2;
    //This is the number of starting cell offsets we actually need to keep.

    constexpr static int s_bits_per_offset = 5;
    constexpr static int s_bits_per_last_offset = 6;
    constexpr static int s_more_bits_begin = 9;
    //We start giving an extra bit to the offsets from
    //the 9th number (corresponding to prevSuperCalo)
    //since corners3D can push some cells to have more than 32 neighbours.


    constexpr static carrier s_offset_mask = 0x1FULL;
    constexpr static carrier s_offset_mask_last = 0x3FULL;
    constexpr static carrier s_offset_delta_pattern = 0x0008210842108421ULL;
    //This has one set bit every five,
    //except for the last two, that are one every six.

    constexpr static carrier s_last_bit_mask = 0x8000000000000000ULL;
    constexpr static carrier s_beforelast_bit_mask = 0x4000000000000000ULL;

    constexpr static carrier s_limited_bits_mask = s_last_bit_mask | s_beforelast_bit_mask;

    constexpr static carrier s_unused_mask = 0x3E00000000000000ULL;
    //We actually have five spare bits!
    //Not enough to store number of neighbours, though...
    //(Maybe it'll be useful for other limited neighbours
    // besides HEICW and FCAL?)

    constexpr static carrier s_only_numbers_mask = ~(s_limited_bits_mask | s_unused_mask);

    static_assert( s_more_bits_begin * s_bits_per_offset +
                   (s_numbers_to_keep - s_more_bits_begin) * s_bits_per_last_offset +
                   2 < sizeof(carrier) * CHAR_BIT, "All the offsets must fit within the carrier!" );
    //The first offset is 0, by definition!
    //We also need the last two bits free to mark the cells to be limited to nextInSamp
    //according to the restrict(...) options.

   public:

    constexpr operator carrier () const
    {
      return value;
    }

    constexpr NeighOffsets (const carrier v): value(v)
    {
    }

    constexpr NeighOffsets & operator = (const carrier v)
    {
      value = v;
      return (*this);
    }

    constexpr int get_number(const int i) const
    {
      carrier ret = value & s_only_numbers_mask;
      if (i >= s_more_bits_begin)
        {
          ret = ret >> (s_bits_per_offset * s_more_bits_begin);
          ret = ret >> (s_bits_per_last_offset * (i - s_more_bits_begin));
          ret &= s_offset_mask_last;
        }
      else
        {
          ret = ret >> (s_bits_per_offset * i);
          ret &= s_offset_mask;
        }
      return (uint32_t) ret;
    }

    constexpr int get_start_cell (const int option) const
    {
      if (option == 0)
        {
          return 0;
        }
      return get_number(option - 1);
    }

    /*! To clarify, this cell no longer corresponds to the option,
        so it's one past the end (like the end iterator of a C++ container)...
        Except in the case of the last option
        (corresponding to @c LArNeighbours::nextSuperCalo),
        which simply returns the maximum possible number of neighbours
        (so one should check @c NeighArr::total_number instead).

    */
    constexpr int get_end_cell (const int option) const
    {
      if (option >= NumNeighOptions - 1)
        {
          return NMaxNeighbours;
        }
      return get_number(option);
    }

    constexpr int get_num_cells (const int option) const
    {
      return this->get_end_cell(option) - this->get_start_cell(option);
    }

    /*! Returns, for each @p i, what must be added to the offset
        to increase the stores numbers appropriately when a cell
        belonging to option @c (i-1) is added.

        In other words, the neighbour counts starting from option @p i are incremented.
    */
    static constexpr carrier offset_delta(const int i)
    {
      carrier mask = 0xFFFFFFFFFFFFFFFFULL;
      if (i >= s_more_bits_begin)
        {
          mask = mask << (s_bits_per_offset * s_more_bits_begin);
          mask = mask << (s_bits_per_last_offset * (i - s_more_bits_begin));
        }
      else
        {
          mask = mask << (s_bits_per_offset * i);
        }
      mask &= s_only_numbers_mask;
      return s_offset_delta_pattern & mask;
    }

    constexpr bool is_limited(const bool limit_HECIWandFCal, const bool limit_PS) const
    {
      return value & ((s_last_bit_mask * limit_HECIWandFCal) | (s_beforelast_bit_mask * limit_PS));
    }

    constexpr bool is_limited_HECIWandFCal() const
    {
      return value & s_last_bit_mask;
    }

    constexpr bool is_limited_PS() const
    {
      return value & s_beforelast_bit_mask;
    }

    constexpr bool is_limited_any() const
    {
      return value & s_limited_bits_mask;
    }

    constexpr void set_limited(const bool limited_one = true, const bool limited_two = true)
    {
      value = (value & (~s_limited_bits_mask)) | (limited_one * s_last_bit_mask) | (limited_two * s_beforelast_bit_mask);
    }

    static constexpr carrier limited_HECIWandFCal_bitmask()
    {
      return s_last_bit_mask;
    }

    static constexpr carrier limited_PS_bitmask()
    {
      return s_beforelast_bit_mask;
    }

    static constexpr carrier limited_both_bitmask()
    {
      return s_limited_bits_mask;
    }
  };

  struct NeighArr
  {
    int total_number[NCaloCells];
    NeighOffsets::carrier offsets[NCaloCells];
    int cells[NMaxNeighbours][NCaloCells];
    //This is more efficient for the GPU
    //as long as the threads are accessing
    //the same neighbour of the cell in parallel.
    //It's slightly worse if we add the neighbour options in the mix...

    constexpr bool has_limited_neighbours(const int cell, const bool limited_HECIWandFCal, const bool limited_PS) const
    {
      const NeighOffsets neigh_off = offsets[cell];
      return neigh_off.is_limited(limited_HECIWandFCal, limited_PS);
    }

    constexpr bool has_limited_HECIWandFCal_neighbours(const int cell) const
    {
      const NeighOffsets neigh_off = offsets[cell];
      return neigh_off.is_limited_HECIWandFCal();
    }

    constexpr bool has_limited_PS_neighbours(const int cell) const
    {
      const NeighOffsets neigh_off = offsets[cell];
      return neigh_off.is_limited_PS();
    }

    constexpr bool has_limited_neighbours_any(const int cell) const
    {
      const NeighOffsets neigh_off = offsets[cell];
      return neigh_off.is_limited_any();
    }

   private:

    constexpr static int s_next_in_samp_option = 6;
    constexpr static unsigned int s_next_in_samp_bit_mask = 0x00000040U;

   public:

    /*!
     *  If \p limit is `true`, limits the neighbours to `nextInSamp`, as is done
     *  with certain options to some cells in the CPU cluster growing and splitting.
    */
    constexpr int get_number_of_neighbours(const int cell, const bool limit_HECIWandFCal = false, const bool limit_PS = false) const
    {
      const NeighOffsets neigh_off = offsets[cell];
      if (neigh_off.is_limited(limit_HECIWandFCal, limit_PS))
        {
          return neigh_off.get_num_cells(s_next_in_samp_option);
        }
      else
        {
          return total_number[cell];
        }
    }


    /*! Just for semantic clarity since the neighbours are reversed.
     *  If \p limit is `true`, limits the neighbours to `nextInSamp`, as is done
     *  with certain options to some cells in the CPU cluster growing and splitting.
     *
     */
    constexpr int get_neighbour(const int cell, const int neigh_number, const bool limit_HECIWandFCal = false, const bool limit_PS = false) const
    {
      const NeighOffsets neigh_off = offsets[cell];
      if (neigh_off.is_limited(limit_HECIWandFCal, limit_PS))
        {
          return cells[neigh_number + neigh_off.get_start_cell(s_next_in_samp_option)][cell];
        }
      else
        {
          return cells[neigh_number][cell];
        }
    }

    /*! Just for semantic clarity since the neighbours are reversed.
     *
     */
    constexpr int set_neighbour(const int cell, const int neigh_number, const int neigh_v)
    {
      return cells[neigh_number][cell] = neigh_v;
    }

    /*! Places the neighbours in the array and returns the number of neighbours.
     *  If \p limit is `true`, limits the neighbours to `nextInSamp`, as is done
     *  with certain options to some cells in the CPU cluster growing and splitting.
     *
     *  We're using C arrays for more immediate CUDA compatibility.
     */
    constexpr int get_neighbours(const int cell, int * neigh_arr, const bool limit_HECIWandFCal = false, const bool limit_PS = false) const
    {
      const NeighOffsets neigh_off = offsets[cell];
      if (neigh_off.is_limited(limit_HECIWandFCal, limit_PS))
        {
          const int start = neigh_off.get_start_cell(s_next_in_samp_option);
          const int end = neigh_off.get_end_cell(s_next_in_samp_option);
          const int num_neighs = end - start;
          for (int i = start; i < end; ++i)
            {
              neigh_arr[i - start] = cells[i][cell];
            }
          return num_neighs;
        }
      //We don't do this by default
      //because the splitter doesn't exclude
      //the cells when finding local maxima...
      else
        {
          const int num_neighs = total_number[cell];
          for (int i = 0; i < num_neighs; ++i)
            {
              neigh_arr[i] = cells[i][cell];
            }
          return num_neighs;
        }
    }

    /*! Places the neighbours according to the option(s) in neigh_options in the array
     *  and returns the number of such neighbours (not the total number of neighbours of the cell).
     *
     *  We're using C arrays for more immediate CUDA compatibility.
     */
    constexpr int get_neighbours_with_option(const unsigned int neigh_options, const int cell, int * neigh_arr,
                                             const bool limit_HECIWandFCal = false, const bool limit_PS = false    ) const
    {
      int neigh = 0;
      int limit = 0;
      int neigh_arr_len = 0;
      const int num_neighs = total_number[cell];

      const NeighOffsets neigh_off = offsets[cell];

      if (neigh_off.is_limited(limit_HECIWandFCal, limit_PS) && (neigh_options & s_next_in_samp_bit_mask))
        {
          const int start = neigh_off.get_start_cell(s_next_in_samp_option);
          const int end = neigh_off.get_end_cell(s_next_in_samp_option);
          for (int i = start; i < end; ++i)
            {
              neigh_arr[i - start] = cells[i][cell];
            }
          return end - start;
        }
      else
        {
          for (int i = 0; i < NumNeighOptions; ++i)
            {
              limit = neigh_off.get_end_cell(i);
              if ( neigh_options & (1U << i) )
                {
                  for (; neigh < limit && neigh < num_neighs; ++neigh)
                    {
                      neigh_arr[neigh_arr_len] = cells[neigh][cell];
                      ++neigh_arr_len;
                    }
                }
              neigh = limit;
            }
        }
      return neigh_arr_len;
    }
  };

  struct GeometryArr
  {
    int   caloSample[NCaloCells];
    float x[NCaloCells];
    float y[NCaloCells];
    float z[NCaloCells];
    float eta[NCaloCells];
    float phi[NCaloCells];
    float volume[NCaloCells];

    NeighArr neighbours;
    //Cell A will appear in the neighbours of cell B if
    //A appears in the result of the CPU get_neighbours of cell B...

    constexpr static bool is_tile (const int cell)
    {
      constexpr int tileStart = 182468;
      constexpr int tileEnd = NCaloCells;
      //These values should be constant.
      //If there are any changes to that...
      //Well, we'll have to update them here.
      //But it's not something that would change at run time.
      return cell >= tileStart && cell < tileEnd;
    }

  };

  struct CellNoiseArr
  {
    float noise[NumGainStates][NCaloCells];
  };

}

#endif
