//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_CONSTANTINFODEFINITIONS_H
#define CALORECGPU_CONSTANTINFODEFINITIONS_H

#include "BaseDefinitions.h"
#include "Helpers.h"
#include <climits>
//For CHAR_BIT
#include <utility>
//Forward

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

  /// @class EtaPhiMapEntry
  /// @brief Holds an (eta, phi) to cell map for a given sampling.
  ///
  /// @par eta_grid: number of subdivisions in eta (doubled for non-continuous)
  ///
  /// @par phi_grid: number of subdivisions in phi (doubled for non-continuous)
  ///
  /// @par respect_deltas: if @p true, the cells stretch only as far as their @p deta and @p dphi;
  ///                      if @p false, always return closest cell.
  ///
  /// @par continuous: if @p true, the sampling provides continuous coverage for positive and negative etas.
  ///                  if @p false, the sampling is separated between its positive and negative eta cells.
  template <int eta_grid, int phi_grid, bool respect_deltas, bool continuous>
  struct EtaPhiMapEntry;

  template <int eta_grid, int phi_grid, bool respect_deltas>
  struct EtaPhiMapEntry<eta_grid, phi_grid, respect_deltas, true>
  {
    friend class EtaPhiMapEntry<eta_grid, phi_grid, respect_deltas, false>;

    static constexpr int s_max_overlap_cells = 10;
    //We could/should try to refine things
    //so we get only 8 overlapping cells at most,
    //for more efficient memory transfers
    //(32 bytes more natural than 40...).

    float eta_min;
    float phi_min;
    float eta_max;
    float phi_max;
    float delta_eta;
    float delta_phi;
    int   cells      [eta_grid][phi_grid][s_max_overlap_cells];
    float corner_eta [eta_grid][phi_grid][s_max_overlap_cells];
    float corner_phi [eta_grid][phi_grid][s_max_overlap_cells];
    //Corners in fractions of delta_eta/phi
    //(Or center (eta, phi) of original cells,
    //if respect_deltas == false)

    //If corner_{eta, phi}[h][f][n] > 0,
    //then the cell ends at this fraction.
    //If it < 0, it starts at this fraction.

   private:

    constexpr int eta_coordinate(const float eta) const
    {
      using namespace std;
      return floorf((eta - eta_min) / delta_eta);
    }

    ///Version that returns the floating point remainder too via the second argument.
    constexpr int eta_coordinate(const float eta, float & interval) const
    {
      using namespace std;
      const float frac = (eta - eta_min) / delta_eta;
      const float rounded = floorf(frac);
      interval = frac - rounded;
      return rounded;
    }

    constexpr int phi_coordinate(const float phi) const
    {
      using namespace std;
      return floorf((phi - phi_min) / delta_phi);
    }

    ///Version that returns the floating point remainder too via the second argument.
    constexpr int phi_coordinate(const float phi, float & interval) const
    {
      using namespace std;
      const float frac = (phi - phi_min) / delta_phi;
      const float rounded = floorf(frac);
      interval = frac - rounded;
      return rounded;
    }

    constexpr void add_cell_to_grid(const int cell, const float eta_corner, const float phi_corner, const int eta, const int phi)
    {
      for (int i = 0; i < s_max_overlap_cells; ++i)
        {
          if (cells[eta][phi][i] < 0)
            {
              cells      [eta][phi][i] = cell;
              corner_eta [eta][phi][i] = eta_corner;
              corner_phi [eta][phi][i] = phi_corner;
              return;
            }
        }
      // /*
      printf("Unable to store %d: %d %d %d %d (%d %d %d %d)\n", cell,
             cells[eta][phi][0], cells[eta][phi][1],
             cells[eta][phi][2], cells[eta][phi][3],
             eta, eta_grid, phi, phi_grid                               );
      // */
    }

    constexpr void register_cell_with_deltas(const int cell, const float cell_eta, const float cell_phi, const float cell_deta, const float cell_dphi)
    {
      int eta_coord_start = 0, eta_coord_end = 0, phi_coord_start = 0, phi_coord_end = 0;
      int phi_coord_extra_neg = 0, phi_coord_extra_pos = 0;
      float eta_fraction_start = 0.f, eta_fraction_end = 0.f, phi_fraction_start = 0.f, phi_fraction_end = 0.f;
      float phi_fraction_extra_neg = 0.f, phi_fraction_extra_pos = 0.f;

      eta_coord_start = eta_coordinate(cell_eta - cell_deta / 2, eta_fraction_start);
      eta_coord_end   = eta_coordinate(cell_eta + cell_deta / 2, eta_fraction_end);
      phi_coord_start = phi_coordinate(cell_phi - cell_dphi / 2, phi_fraction_start);
      phi_coord_end   = phi_coordinate(cell_phi + cell_dphi / 2, phi_fraction_end);

      phi_coord_extra_neg = phi_coordinate(cell_phi + cell_dphi / 2 - 2 * Helpers::Constants::pi<float>, phi_fraction_extra_neg);
      phi_coord_extra_pos = phi_coordinate(cell_phi - cell_dphi / 2 + 2 * Helpers::Constants::pi<float>, phi_fraction_extra_pos);

      if ((eta_coord_end == eta_coord_start && (eta_fraction_start > 0 || eta_fraction_end < 1)) ||
          (phi_coord_end == phi_coord_start && (phi_fraction_start > 0 || phi_fraction_end < 1))     )
        {
          // /*
          printf("Something strange going on with cell %d: %f %f %f %f %f %f (%d %d %f %f | %d %d %f %f)\n",
                 cell, cell_eta, cell_deta, delta_eta, cell_phi, cell_dphi, delta_phi,
                 eta_coord_start, eta_coord_end, eta_fraction_start, eta_fraction_end,
                 phi_coord_start, phi_coord_end, phi_fraction_start, phi_fraction_end);
          // */
          return;
        }

      if (eta_coord_start < 0 || eta_coord_end < 0 || phi_coord_start < 0 || phi_coord_end < 0 ||
          eta_fraction_start < 0 || eta_fraction_end < 0 || phi_fraction_start < 0 || phi_fraction_end < 0)
        {
          // /*
          printf("Underflowing cell %d: %f %f %f %f %f %f (%d %d %f %f | %d %d %f %f) [%f %f]\n",
                 cell, cell_eta, cell_deta, cell_eta - cell_deta / 2, cell_phi, cell_dphi, cell_phi - cell_dphi / 2,
                 eta_coord_start, eta_coord_end, eta_fraction_start, eta_fraction_end,
                 phi_coord_start, phi_coord_end, phi_fraction_start, phi_fraction_end,
                 eta_min, phi_min);
          // */
          return;
        }

      if (eta_coord_start >= eta_grid || eta_coord_end >= eta_grid || phi_coord_start >= phi_grid || phi_coord_end >= phi_grid ||
          eta_fraction_start > 1 || eta_fraction_end > 1 || phi_fraction_start > 1 || phi_fraction_end > 1)
        {
          // /*
          printf("Overflowing cell %d: %f %f %f %f %f %f (%d %d %f %f | %d %d %f %f) [%f %f]\n",
                 cell, cell_eta, cell_deta, cell_eta + cell_deta / 2, cell_phi, cell_dphi, cell_phi + cell_dphi / 2,
                 eta_coord_start, eta_coord_end, eta_fraction_start, eta_fraction_end,
                 phi_coord_start, phi_coord_end, phi_fraction_start, phi_fraction_end,
                 eta_min, phi_min);
          // */
          return;
        }

      for (int eta_c = eta_coord_start; eta_c <= eta_coord_end; ++eta_c)
        {
          const float eta_corner = ( eta_c == eta_coord_start ?
                                     -eta_fraction_start : ( eta_c == eta_coord_end ?
                                                             eta_fraction_end : 0.f      )
                                   );
          for (int phi_c = 0; phi_c <= phi_coord_extra_neg; ++phi_c)
            {
              const float phi_corner = ( phi_c == phi_coord_extra_neg ?
                                         phi_fraction_extra_neg : 0.f   );

              add_cell_to_grid(cell, eta_corner, phi_corner, eta_c, phi_c);

            }
          for (int phi_c = phi_coord_start; phi_c <= phi_coord_end; ++phi_c)
            {
              const float phi_corner = ( phi_c == phi_coord_start ?
                                         -phi_fraction_start : ( phi_c == phi_coord_end ?
                                                                 phi_fraction_end : 0.f      )
                                       );
              add_cell_to_grid(cell, eta_corner, phi_corner, eta_c, phi_c);
            }
          for (int phi_c = phi_coord_extra_pos; phi_c < phi_grid; ++phi_c)
            {
              const float phi_corner = ( phi_c == phi_coord_extra_pos ?
                                         -phi_fraction_extra_pos : 0.f   );
              add_cell_to_grid(cell, eta_corner, phi_corner, eta_c, phi_c);
            }
        }
    }

    constexpr void register_cell_center(const int cell, const float cell_eta, const float cell_phi)
    {
      const int eta_coord = eta_coordinate(cell_eta);
      const int phi_coord = phi_coordinate(cell_phi);

      if (eta_coord < 0 || eta_coord >= eta_grid || phi_coord < 0 || phi_coord >= phi_grid)
        {
          // /*
          printf("Out of bounds cell: %d | %f %d %f %f %d | %f %d %f %f %d\n", cell,
                 cell_eta, eta_coord, eta_min, eta_max, eta_grid,
                 cell_phi, phi_coord, phi_min, phi_max, phi_grid                        );
          // */
          return;
        }

      add_cell_to_grid(cell, cell_eta, cell_phi, eta_coord, phi_coord);
    }


    constexpr void initialize(const float min_eta, const float min_phi,
                              const float max_eta, const float max_phi,
                              const float deta, const float dphi)
    {
      for (int i = 0; i < eta_grid; ++i)
        {
          for (int j = 0; j < phi_grid; ++j)
            {
              for (int k = 0; k < s_max_overlap_cells; ++k)
                {
                  cells[i][j][k] = -1;
                }
            }
        }
      eta_min = min_eta;
      phi_min = min_phi;
      eta_max = max_eta;
      phi_max = max_phi;
      delta_eta = deta;
      delta_phi = dphi;
    }

   public:


    constexpr void register_cell(const int cell, const float cell_eta, const float cell_phi, const float cell_deta, const float cell_dphi)
    {
      if (respect_deltas)
        //If we could be sure of C++17 compatibility,
        //this would be a constexpr if...
        {
          register_cell_with_deltas(cell, cell_eta, cell_phi, cell_deta, cell_dphi);
        }
      else
        {
          register_cell_center(cell, cell_eta, cell_phi);
        }
    }

    constexpr void initialize()
    {
      initialize(0, 0, 0, 0, 0, 0);
    }

    constexpr void initialize(const float min_eta_neg, const float min_phi_neg,
                              const float /*max_eta_neg*/, const float /*max_phi_neg*/,
                              const float /*min_eta_pos*/, const float /*min_phi_pos*/,
                              const float max_eta_pos, const float max_phi_pos,
                              const float deta, const float dphi)
    {
      initialize(min_eta_neg, min_phi_neg, max_eta_pos, max_phi_pos, deta, dphi);
    }

    constexpr static size_t finish_initializing_buffer_size()
    {
      return eta_grid * phi_grid * s_max_overlap_cells * sizeof(int);
    }

    ///! @par buffer is casted to a sufficiently large array (minimum size given by @p finish_initializing_buffer_size).
    CUDA_HOS_DEV void finish_initializing(void * buffer)
    {
      if (!respect_deltas)
        //If we could be sure of C++17 compatibility,
        //this would be a constexpr if...
        {
          //This is O( N * M * (N + M) ) for a N * M grid,
          //but gives us reasonable results.
          //And, since this is just an initialization step,
          //not too serious. Still, we could optimize it further...
          //(Or do it on the GPU. Really.
          //Because it ends up being quite cellular automaton-y.)
          //
          //Of course the proper way to do this would be to build
          //a Voronoi diagram based on the cell positions
          //and then discretize it in the grid (overlapping where needed),
          //but the complexity versus performance gains trade-off
          //in this case favours keeping this simpler version.

          int (*casted)[eta_grid][phi_grid][s_max_overlap_cells] = (int (*)[eta_grid][phi_grid][s_max_overlap_cells]) buffer;

          int (&cell_arr_dest)[eta_grid][phi_grid][s_max_overlap_cells] = *casted;

          using namespace std;

          memcpy(&(cell_arr_dest[0][0][0]), &(cells[0][0][0]), finish_initializing_buffer_size());

          bool keep_going = true;
          
          bool second_phase = false;
          //The first phase grows out the clusters,
          //I mean, the elements of the grid that are
          //closest to a cell and stops when they intersect.
          //The second phase grows out those who do intersect
          //to empty elements, to handle the cases where an element
          //is all surrounded by intersections...
          

          auto propagate_cell = [&](const int eta_c, const int phi_c, const int input_cell,
                                    const float input_eta, const float input_phi, const bool only_empty )
          {
            if (only_empty && cells[eta_c][phi_c][0] >= 0)
              {
                return;
              }
            int to_add_index = 0;
            for (; to_add_index < s_max_overlap_cells; ++to_add_index)
              {
                if (cell_arr_dest[eta_c][phi_c][to_add_index] == input_cell)
                  {
                    return;
                  }
                else if (cell_arr_dest[eta_c][phi_c][to_add_index] < 0)
                  {
                    break;
                  }
              }
            if (to_add_index == s_max_overlap_cells)
              {
                // /*
                printf("Unable to store %d: %d %d %d %d (%d %d %d %d)\n", input_cell,
                       cell_arr_dest[eta_c][phi_c][0], cell_arr_dest[eta_c][phi_c][1],
                       cell_arr_dest[eta_c][phi_c][2], cell_arr_dest[eta_c][phi_c][3],
                       eta_c, eta_grid, phi_c, phi_grid                                );
                // */
                return;
              }

            cell_arr_dest [eta_c][phi_c][to_add_index] = input_cell;
            corner_eta    [eta_c][phi_c][to_add_index] = input_eta;
            corner_phi    [eta_c][phi_c][to_add_index] = input_phi;

            keep_going = true;
          };

          auto process_cell = [&](const int eta_c, const int phi_c, const int cell_index, const bool only_empty)
          {
            const int   this_cell = cells      [eta_c][phi_c][cell_index];
            const float this_eta  = corner_eta [eta_c][phi_c][cell_index];
            const float this_phi  = corner_phi [eta_c][phi_c][cell_index];

            for (int delta_eta = -1; delta_eta <= 1; ++delta_eta)
              {
                const int target_eta_c = eta_c + delta_eta;
                if (target_eta_c < 0 || target_eta_c >= eta_grid)
                  {
                    continue;
                  }
                for (int delta_phi = -1; delta_phi <= 1; ++delta_phi)
                  {
                    int target_phi_c = phi_c + delta_phi;
                    if (target_phi_c < 0)
                      {
                        target_phi_c = phi_coordinate(phi_min + delta_phi * target_phi_c + 2 * Helpers::Constants::pi<float>);
                        if (target_phi_c < 0 || target_phi_c >= phi_grid)
                          {
                            continue;
                          }
                      }
                    else if (target_phi_c >= phi_grid)
                      {
                        target_phi_c = phi_coordinate(phi_min + delta_phi * target_phi_c - 2 * Helpers::Constants::pi<float>);
                        if (target_phi_c < 0 || target_phi_c >= phi_grid)
                          {
                            continue;
                          }
                      }
                    propagate_cell(target_eta_c, target_phi_c, this_cell, this_eta, this_phi, only_empty);
                  }
              }
          };

          while (keep_going)
            {
              keep_going = false;
              for (int eta_c = 0; eta_c < eta_grid; ++eta_c)
                {
                  for (int phi_c = 0; phi_c < phi_grid; ++phi_c)
                    {
                      const auto & this_square_cells = cells[eta_c][phi_c];
                      if (!second_phase)
                        {
                          if (this_square_cells[0] >= 0 && this_square_cells[1] < 0)
                            {
                              process_cell(eta_c, phi_c, 0, false);
                            }
                        }
                      else
                        {
                          for (int cell_index = 0; cell_index < s_max_overlap_cells; ++cell_index)
                            {
                              if (this_square_cells[cell_index] < 0)
                                {
                                  break;
                                }
                              process_cell(eta_c, phi_c, cell_index, true);
                            }
                        }
                    }
                }
              memcpy(&(cells[0][0][0]), &(cell_arr_dest[0][0][0]), finish_initializing_buffer_size());
              if (!keep_going && !second_phase)
                {
                  second_phase = true;
                  keep_going = true;
                }
            }
        }
      else
        {
          //Do nothing: when respecting deltas,
          //cells get properly initialized outright...
        }
    }

    ///We assume @p cell_arr is large enough.
    constexpr int get_possible_cells_from_coords(const float test_eta, const float test_phi, int * cell_arr) const
    {
      int num_cells = 0;
      
      float frac_eta = 0.f, frac_phi = 0.f;

      const int eta_coord = eta_coordinate(test_eta, frac_eta);
      const int phi_coord = phi_coordinate(test_phi, frac_phi);

      if ( test_eta < eta_min || test_eta > eta_max    ||
           test_phi < phi_min || test_phi > phi_max    ||
           eta_coord < 0      || eta_coord >= eta_grid ||
           phi_coord < 0      || phi_coord >= phi_grid     )
        {
          /*
          printf("OOB: %f %d %f %f %d | %f %d %f %f %d\n",
                 test_eta, eta_coord, eta_min, eta_max, eta_grid,
                 test_phi, phi_coord, phi_min, phi_max, phi_grid);
          // */
          return 0;
        }

      if (respect_deltas)
        //If we could be sure of C++17 compatibility,
        //this would be a constexpr if...
        {
          auto check_coord = [](const float test, const float target)
          {
            using namespace std;
            if (target > 0 && fabsf(test) <= fabsf(target))
              {
                return true;
              }
            else if (target < 0 && fabsf(test) >= fabsf(target))
              {
                return true;
              }
            else if (target == 0)
              {
                return true;
              }
            else
              {
                return false;
              }
          };

          for (int i = 0; i < s_max_overlap_cells; ++i)
            {
              if (cells[eta_coord][phi_coord][i] < 0)
                {
                  break;
                }
              if ( check_coord(frac_eta, corner_eta[eta_coord][phi_coord][i]) &&
                   check_coord(frac_phi, corner_phi[eta_coord][phi_coord][i])      )
                {
                  cell_arr[num_cells] = cells[eta_coord][phi_coord][i];
                  ++num_cells;
                }
            }
        }
      else
        {
          float distance = 1e38f;
          int ret = -1;

          for (int i = 0; i < s_max_overlap_cells; ++i)
            {
              const int this_cell = cells[eta_coord][phi_coord][i];
              if (this_cell < 0)
                {
                  break;
                }
              const float delta_eta = corner_eta[eta_coord][phi_coord][i] - test_eta;
              const float delta_phi = Helpers::angular_difference(corner_phi[eta_coord][phi_coord][i], test_phi);
              //If respect_deltas == false,
              //corner_coords are actually
              //the center (eta, phi) of the cell.
              const float this_dist = delta_eta * delta_eta + delta_phi * delta_phi;

              if (this_dist < distance || (this_dist == distance && this_cell > ret))
                {
                  distance = this_dist;
                  ret = this_cell;
                }
            }

          if (ret > 0)
            {
              cell_arr[0] = ret;
              num_cells = 1;
            }
        }

      return num_cells;
    }

    constexpr bool has_cell_in_coords(const float test_eta, const float test_phi) const
    {
      float frac_eta = 0.f, frac_phi = 0.f;

      const int eta_coord = eta_coordinate(test_eta, frac_eta);
      const int phi_coord = phi_coordinate(test_phi, frac_phi);

      if ( test_eta  < eta_min || test_eta  >  eta_max    ||
           test_phi  < phi_min || test_phi  >  phi_max    ||
           eta_coord < 0       || eta_coord >= eta_grid   ||
           phi_coord < 0       || phi_coord >= phi_grid      )
        {
          return false;
        }

      if (respect_deltas)
        //If we could be sure of C++17 compatibility,
        //this would be a constexpr if...
        {
          auto check_coord = [](const float test, const float target)
          {
            using namespace std;
            if (fabsf(test) >= fabs(target) && target < 0)
              {
                return true;
              }
            else if (fabs(test) <= fabs(target) && target > 0)
              {
                return true;
              }
            else if (target == 0)
              {
                return true;
              }
            else
              {
                return false;
              }
          };

          for (int i = 0; i < s_max_overlap_cells; ++i)
            {
              if (cells[eta_coord][phi_coord][i] < 0)
                {
                  break;
                }
              if ( check_coord(frac_eta, corner_eta[eta_coord][phi_coord][i]) &&
                   check_coord(frac_phi, corner_phi[eta_coord][phi_coord][i])      )
                {
                  return true;
                }
            }
          return false;
        }
      else
        {
          return true;
          //Except for being out-of-bounds,
          //if respect_deltas == false
          //we can always find a closest cell
          //for each coordinate.
        }
    }

  };

  template <int eta_grid, int phi_grid, bool respect_deltas>
  struct EtaPhiMapEntry<eta_grid, phi_grid, respect_deltas, false>
  {
    EtaPhiMapEntry<eta_grid, phi_grid, respect_deltas, true> pos, neg;

    static constexpr int s_max_overlap_cells = EtaPhiMapEntry<eta_grid, phi_grid, respect_deltas, true>::s_max_overlap_cells;

    constexpr void register_cell(const int cell, const float cell_eta, const float cell_phi, const float cell_deta, const float cell_dphi)
    {
      if (cell_eta >= 0)
        {
          pos.register_cell(cell, cell_eta, cell_phi, cell_deta, cell_dphi);
        }
      else
        {
          neg.register_cell(cell, cell_eta, cell_phi, cell_deta, cell_dphi);
        }
    }

    constexpr void initialize(const float min_eta_neg, const float min_phi_neg,
                              const float max_eta_neg, const float max_phi_neg,
                              const float min_eta_pos, const float min_phi_pos,
                              const float max_eta_pos, const float max_phi_pos,
                              const float deta, const float dphi)
    {
      pos.initialize(min_eta_pos, min_phi_pos, max_eta_pos, max_phi_pos, deta, dphi);
      neg.initialize(min_eta_neg, min_phi_neg, max_eta_neg, max_phi_neg, deta, dphi);
    }

    constexpr void initialize()
    {
      pos.initialize();
      neg.initialize();
    }

    constexpr static size_t finish_initializing_buffer_size()
    {
      return EtaPhiMapEntry<eta_grid, phi_grid, respect_deltas, true>::finish_initializing_buffer_size();
    }

    ///! @par buffer is casted to a sufficiently large array (minimum size given by @p finish_initializing_buffer_size).
    CUDA_HOS_DEV void finish_initializing(void * buffer)
    {
      pos.finish_initializing(buffer);
      neg.finish_initializing(buffer);
    }

    ///We assume @p cell_arr is large enough.
    constexpr int get_possible_cells_from_coords(const float test_eta, const float test_phi, int * cell_arr) const
    {
      int ret = -1;
      if (test_eta >= 0)
        {
          ret = pos.get_possible_cells_from_coords(test_eta, test_phi, cell_arr);
        }
      else
        {
          ret = neg.get_possible_cells_from_coords(test_eta, test_phi, cell_arr);
        }
      return ret;
    }

    constexpr bool has_cell_in_coords(const float test_eta, const float test_phi) const
    {
      if (test_eta >= 0)
        {
          return pos.has_cell_in_coords(test_eta, test_phi);
        }
      else
        {
          return neg.has_cell_in_coords(test_eta, test_phi);
        }
    }

  };

  struct EtaPhiToCellMap
  {
    static constexpr int s_max_overlap_cells = EtaPhiMapEntry<1, 1, true, true>::s_max_overlap_cells;

    //Samplings have custom, hard-coded sizes to save space.
    //Could've gone with one-size-fits all maxima,
    //but it'd likely be unnecessarily wasteful.
    EtaPhiMapEntry<125,  65, false,  true> sampling_0;
    EtaPhiMapEntry<955, 260, false,  true> sampling_1;
    EtaPhiMapEntry<121, 260, false,  true> sampling_2;
    EtaPhiMapEntry< 56, 260, false,  true> sampling_3;
    EtaPhiMapEntry< 14,  68, false, false> sampling_4;
    EtaPhiMapEntry<362,  68, false, false> sampling_5;
    EtaPhiMapEntry< 74, 260, false, false> sampling_6;
    EtaPhiMapEntry< 36, 260, false, false> sampling_7;
    EtaPhiMapEntry< 20,  66, false, false> sampling_8;
    EtaPhiMapEntry< 18,  66, false, false> sampling_9;
    EtaPhiMapEntry< 16,  66, false, false> sampling_10;
    EtaPhiMapEntry< 18,  66, false, false> sampling_11;
    EtaPhiMapEntry< 12,  66,  true, false> sampling_12;
    EtaPhiMapEntry< 12,  66,  true, false> sampling_13;
    EtaPhiMapEntry<  5,  66,  true, false> sampling_14;
    EtaPhiMapEntry<  2,  66,  true, false> sampling_15;
    EtaPhiMapEntry<  2,  66,  true, false> sampling_16;
    EtaPhiMapEntry<  8,  66,  true, false> sampling_17;
    EtaPhiMapEntry<  6,  66,  true, false> sampling_18;
    EtaPhiMapEntry<  6,  66,  true, false> sampling_19;
    EtaPhiMapEntry<  4,  66,  true, false> sampling_20;
    EtaPhiMapEntry<142, 228, false, false> sampling_21;
    EtaPhiMapEntry< 96, 162, false, false> sampling_22;
    EtaPhiMapEntry< 32, 128, false, false> sampling_23;
    EtaPhiMapEntry<  1,   1, false,  true> sampling_24;
    EtaPhiMapEntry<  1,   1, false,  true> sampling_25;
    EtaPhiMapEntry<  1,   1, false,  true> sampling_26;
    EtaPhiMapEntry<  1,   1, false,  true> sampling_27;

    static_assert(NumSamplings == 28, "Written under the assumption there are 28 samplings.");

    ///@p F must be prepared to receive as first argument a EtaPhiMapEntry<N, M>, as well as any arguments.
    template <class Func, class ... Args>
    constexpr void apply_to_all_samplings(Func && F, Args && ... args)
    {
      F(sampling_0, std::forward<Args>(args)...);
      F(sampling_1, std::forward<Args>(args)...);
      F(sampling_2, std::forward<Args>(args)...);
      F(sampling_3, std::forward<Args>(args)...);
      F(sampling_4, std::forward<Args>(args)...);
      F(sampling_5, std::forward<Args>(args)...);
      F(sampling_6, std::forward<Args>(args)...);
      F(sampling_7, std::forward<Args>(args)...);
      F(sampling_8, std::forward<Args>(args)...);
      F(sampling_9, std::forward<Args>(args)...);
      F(sampling_10, std::forward<Args>(args)...);
      F(sampling_11, std::forward<Args>(args)...);
      F(sampling_12, std::forward<Args>(args)...);
      F(sampling_13, std::forward<Args>(args)...);
      F(sampling_14, std::forward<Args>(args)...);
      F(sampling_15, std::forward<Args>(args)...);
      F(sampling_16, std::forward<Args>(args)...);
      F(sampling_17, std::forward<Args>(args)...);
      F(sampling_18, std::forward<Args>(args)...);
      F(sampling_19, std::forward<Args>(args)...);
      F(sampling_20, std::forward<Args>(args)...);
      F(sampling_21, std::forward<Args>(args)...);
      F(sampling_22, std::forward<Args>(args)...);
      F(sampling_23, std::forward<Args>(args)...);
      F(sampling_24, std::forward<Args>(args)...);
      F(sampling_25, std::forward<Args>(args)...);
      F(sampling_26, std::forward<Args>(args)...);
      F(sampling_27, std::forward<Args>(args)...);
    }

    ///@p F must be prepared to receive as first argument a EtaPhiMapEntry<N, M>, as well as any arguments.
    template <class Func, class ... Args>
    constexpr void apply_to_all_samplings(Func && F, Args && ... args) const
    {
      F(sampling_0, std::forward<Args>(args)...);
      F(sampling_1, std::forward<Args>(args)...);
      F(sampling_2, std::forward<Args>(args)...);
      F(sampling_3, std::forward<Args>(args)...);
      F(sampling_4, std::forward<Args>(args)...);
      F(sampling_5, std::forward<Args>(args)...);
      F(sampling_6, std::forward<Args>(args)...);
      F(sampling_7, std::forward<Args>(args)...);
      F(sampling_8, std::forward<Args>(args)...);
      F(sampling_9, std::forward<Args>(args)...);
      F(sampling_10, std::forward<Args>(args)...);
      F(sampling_11, std::forward<Args>(args)...);
      F(sampling_12, std::forward<Args>(args)...);
      F(sampling_13, std::forward<Args>(args)...);
      F(sampling_14, std::forward<Args>(args)...);
      F(sampling_15, std::forward<Args>(args)...);
      F(sampling_16, std::forward<Args>(args)...);
      F(sampling_17, std::forward<Args>(args)...);
      F(sampling_18, std::forward<Args>(args)...);
      F(sampling_19, std::forward<Args>(args)...);
      F(sampling_20, std::forward<Args>(args)...);
      F(sampling_21, std::forward<Args>(args)...);
      F(sampling_22, std::forward<Args>(args)...);
      F(sampling_23, std::forward<Args>(args)...);
      F(sampling_24, std::forward<Args>(args)...);
      F(sampling_25, std::forward<Args>(args)...);
      F(sampling_26, std::forward<Args>(args)...);
      F(sampling_27, std::forward<Args>(args)...);
    }

    ///@p F must be prepared to receive as first argument a EtaPhiMapEntry<N, M>, as well as any arguments.
    template <class Func, class ... Args>
    constexpr void apply_to_sampling(const int sampling, Func && F, Args && ... args)
    {
      switch (sampling)
        {
          case 0:
            F(sampling_0, std::forward<Args>(args)...);
            break;
          case 1:
            F(sampling_1, std::forward<Args>(args)...);
            break;
          case 2:
            F(sampling_2, std::forward<Args>(args)...);
            break;
          case 3:
            F(sampling_3, std::forward<Args>(args)...);
            break;
          case 4:
            F(sampling_4, std::forward<Args>(args)...);
            break;
          case 5:
            F(sampling_5, std::forward<Args>(args)...);
            break;
          case 6:
            F(sampling_6, std::forward<Args>(args)...);
            break;
          case 7:
            F(sampling_7, std::forward<Args>(args)...);
            break;
          case 8:
            F(sampling_8, std::forward<Args>(args)...);
            break;
          case 9:
            F(sampling_9, std::forward<Args>(args)...);
            break;
          case 10:
            F(sampling_10, std::forward<Args>(args)...);
            break;
          case 11:
            F(sampling_11, std::forward<Args>(args)...);
            break;
          case 12:
            F(sampling_12, std::forward<Args>(args)...);
            break;
          case 13:
            F(sampling_13, std::forward<Args>(args)...);
            break;
          case 14:
            F(sampling_14, std::forward<Args>(args)...);
            break;
          case 15:
            F(sampling_15, std::forward<Args>(args)...);
            break;
          case 16:
            F(sampling_16, std::forward<Args>(args)...);
            break;
          case 17:
            F(sampling_17, std::forward<Args>(args)...);
            break;
          case 18:
            F(sampling_18, std::forward<Args>(args)...);
            break;
          case 19:
            F(sampling_19, std::forward<Args>(args)...);
            break;
          case 20:
            F(sampling_20, std::forward<Args>(args)...);
            break;
          case 21:
            F(sampling_21, std::forward<Args>(args)...);
            break;
          case 22:
            F(sampling_22, std::forward<Args>(args)...);
            break;
          case 23:
            F(sampling_23, std::forward<Args>(args)...);
            break;
          case 24:
            F(sampling_24, std::forward<Args>(args)...);
            break;
          case 25:
            F(sampling_25, std::forward<Args>(args)...);
            break;
          case 26:
            F(sampling_26, std::forward<Args>(args)...);
            break;
          case 27:
            F(sampling_27, std::forward<Args>(args)...);
            break;
          default:
            break;
        }
    }

    ///@p F must be prepared to receive as first argument a EtaPhiMapEntry<N, M>, as well as any arguments.
    template <class Func, class ... Args>
    constexpr void apply_to_sampling(const int sampling, Func && F, Args && ... args) const
    {
      switch (sampling)
        {
          case 0:
            F(sampling_0, std::forward<Args>(args)...);
            break;
          case 1:
            F(sampling_1, std::forward<Args>(args)...);
            break;
          case 2:
            F(sampling_2, std::forward<Args>(args)...);
            break;
          case 3:
            F(sampling_3, std::forward<Args>(args)...);
            break;
          case 4:
            F(sampling_4, std::forward<Args>(args)...);
            break;
          case 5:
            F(sampling_5, std::forward<Args>(args)...);
            break;
          case 6:
            F(sampling_6, std::forward<Args>(args)...);
            break;
          case 7:
            F(sampling_7, std::forward<Args>(args)...);
            break;
          case 8:
            F(sampling_8, std::forward<Args>(args)...);
            break;
          case 9:
            F(sampling_9, std::forward<Args>(args)...);
            break;
          case 10:
            F(sampling_10, std::forward<Args>(args)...);
            break;
          case 11:
            F(sampling_11, std::forward<Args>(args)...);
            break;
          case 12:
            F(sampling_12, std::forward<Args>(args)...);
            break;
          case 13:
            F(sampling_13, std::forward<Args>(args)...);
            break;
          case 14:
            F(sampling_14, std::forward<Args>(args)...);
            break;
          case 15:
            F(sampling_15, std::forward<Args>(args)...);
            break;
          case 16:
            F(sampling_16, std::forward<Args>(args)...);
            break;
          case 17:
            F(sampling_17, std::forward<Args>(args)...);
            break;
          case 18:
            F(sampling_18, std::forward<Args>(args)...);
            break;
          case 19:
            F(sampling_19, std::forward<Args>(args)...);
            break;
          case 20:
            F(sampling_20, std::forward<Args>(args)...);
            break;
          case 21:
            F(sampling_21, std::forward<Args>(args)...);
            break;
          case 22:
            F(sampling_22, std::forward<Args>(args)...);
            break;
          case 23:
            F(sampling_23, std::forward<Args>(args)...);
            break;
          case 24:
            F(sampling_24, std::forward<Args>(args)...);
            break;
          case 25:
            F(sampling_25, std::forward<Args>(args)...);
            break;
          case 26:
            F(sampling_26, std::forward<Args>(args)...);
            break;
          case 27:
            F(sampling_27, std::forward<Args>(args)...);
            break;
          default:
            break;
        }
    }

   private:

    //CUDA and lambdas is still a bit tricky,
    //hence we'll use explicitly written functors
    //to implement several things.
    
    struct initialize_all_functor
    {
      template <class Entry>
      constexpr void operator() (Entry & entry) const
      {
        entry.initialize();
      }
    };

    struct initialize_sampling_functor
    {
      template <class Entry>
      constexpr void operator() (Entry & entry,
                                 const float min_eta_neg, const float min_phi_neg,
                                 const float max_eta_neg, const float max_phi_neg,
                                 const float min_eta_pos, const float min_phi_pos,
                                 const float max_eta_pos, const float max_phi_pos,
                                 const float delta_eta, const float delta_phi        ) const
      {
        entry.initialize(min_eta_neg, min_phi_neg, max_eta_neg, max_phi_neg,
                         min_eta_pos, min_phi_pos, max_eta_pos, max_phi_pos,
                         delta_eta, delta_phi                                 );
      }
    };

    struct register_cell_functor
    {
      template <class Entry>
      constexpr void operator() (Entry & entry, const int cell,
                                 const float cell_eta, const float cell_phi,
                                 const float cell_deta, const float cell_dphi ) const
      {
        entry.register_cell(cell, cell_eta, cell_phi, cell_deta, cell_dphi);
      }
    };

    struct buffer_size_functor
    {
      template <class Entry>
      constexpr void operator() (Entry & entry, size_t & ret) const
      {
        using namespace std;
        ret = max(ret, entry.finish_initializing_buffer_size());
      }
    };

    struct finish_functor
    {
      template <class Entry>
      constexpr void operator() (Entry & entry, void * buffer) const
      {
        entry.finish_initializing(buffer);
      }
    };
    //Aliohjelma-olio?
    //(To not go with the obvious funktio-olio...)

    struct get_cell_from_sampling_functor
    {
      template <class Entry>
      constexpr void operator() (Entry & entry, int & ret, const float test_eta, const float test_phi, int * cell_arr) const
      {
        ret = entry.get_possible_cells_from_coords(test_eta, test_phi, cell_arr);
      }
    };

    struct check_cell_in_sampling_functor
    {
      template <class Entry>
      constexpr void operator() (Entry & entry, bool & ret, const float test_eta, const float test_phi) const
      {
        ret = entry.has_cell_in_coords(test_eta, test_phi);
      }
    };

    struct get_cell_from_all_functor
    {
      template <class Entry>
      constexpr void operator() (Entry & entry, int & ret, const float test_eta, const float test_phi, int * cell_arr) const
      {
        ret += entry.get_possible_cells_from_coords(test_eta, test_phi, cell_arr + ret);
      }
    };

    struct check_cell_in_all_functor
    {
      template <class Entry>
      constexpr void operator() (Entry & entry, bool & ret, const float test_eta, const float test_phi) const
      {
        ret = ( ret || entry.has_cell_in_coords(test_eta, test_phi) );
        //Short circuit evaluation?
      }
    };

   public:

    ///Initialize all cells of all samplings.
    constexpr void initialize()
    {
      apply_to_all_samplings(initialize_all_functor{});
    }

    ///Initialize a specific sampling with known eta and phi ranges.
    constexpr void initialize(const int sampling,
                              const float min_eta_neg, const float min_phi_neg,
                              const float max_eta_neg, const float max_phi_neg,
                              const float min_eta_pos, const float min_phi_pos,
                              const float max_eta_pos, const float max_phi_pos,
                              const float delta_eta, const float delta_phi)
    {
      apply_to_sampling(sampling, initialize_sampling_functor{},
                        min_eta_neg, min_phi_neg,
                        max_eta_neg, max_phi_neg,
                        min_eta_pos, min_phi_pos,
                        max_eta_pos, max_phi_pos,
                        delta_eta, delta_phi      );
    }

    constexpr void register_cell(const int cell, const int sampling, const float cell_eta, const float cell_phi, const float cell_deta, const float cell_dphi)
    {
      apply_to_sampling(sampling, register_cell_functor{}, cell, cell_eta, cell_phi, cell_deta, cell_dphi);
    }


    constexpr size_t finish_initializing_buffer_size() const
    {
      size_t ret = 0;

      apply_to_all_samplings(buffer_size_functor{}, ret);

      return ret;
    }

    ///! @par buffer is casted to a sufficiently large array (mininum size given by @p finish_initializing_buffer_size).
    CUDA_HOS_DEV void finish_initializing(void * buffer)
    {
      apply_to_all_samplings(finish_functor{}, buffer);
    }

    ///We assume @p cell_arr is large enough.
    constexpr int get_possible_cells_from_coords(const int sampling, const float test_eta, const float test_phi, int * cell_arr) const
    {
      int ret = 0;
            
      apply_to_sampling(sampling, get_cell_from_sampling_functor{}, ret, test_eta, test_phi, cell_arr);

      return ret;
    }

    constexpr bool has_cell_in_coords(const int sampling, const float test_eta, const float test_phi) const
    {
      bool ret = false;

      apply_to_sampling(sampling, check_cell_in_sampling_functor{}, ret, test_eta, test_phi);

      return ret;
    }

    ///We assume @p cell_arr is large enough.
    constexpr int get_possible_cells_from_coords(const float test_eta, const float test_phi, int * cell_arr) const
    {
      int ret = 0;

      apply_to_all_samplings(get_cell_from_all_functor{}, ret, test_eta, test_phi, cell_arr);

      return ret;
    }

    constexpr bool has_cell_in_coords(const float test_eta, const float test_phi) const
    {
      bool ret = false;

      apply_to_all_samplings(check_cell_in_all_functor{}, ret, test_eta, test_phi);

      return ret;
    }

  };

  struct GeometryArr
  {
    int   caloSample[NCaloCells];
    float x[NCaloCells];
    float y[NCaloCells];
    float z[NCaloCells];
    float r[NCaloCells];
    float eta[NCaloCells];
    float phi[NCaloCells];

    float dx[NCaloCells];
    float dy[NCaloCells];
    float dz[NCaloCells];
    float dr[NCaloCells];
    float deta[NCaloCells];
    float dphi[NCaloCells];

    float volume[NCaloCells];

    NeighArr neighbours;
    //Cell A will appear in the neighbours of cell B if
    //A appears in the result of the CPU get_neighbours of cell B...


    EtaPhiToCellMap etaPhiToCell;

    constexpr static int s_tileStart = 182468;
    constexpr static int s_tileEnd = NCaloCells;
    //These values should be constant.
    //If there are any changes to that...
    //Well, we'll have to update them here.
    //But it's not something that would change at run time.

    constexpr static bool is_tile (const int cell)
    {
      return cell >= s_tileStart && cell < s_tileEnd;
    }

    CUDA_HOS_DEV void fill_eta_phi_map()
    {
      for (int i = 0; i < NCaloCells; ++i)
        {
          etaPhiToCell.register_cell(i, caloSample[i], eta[i], phi[i], deta[i], dphi[i]);
        }

      struct DeAllocWrapper
      {
        int * buf;
        CUDA_HOS_DEV DeAllocWrapper(const size_t new_size)
        {
          buf = new int[Helpers::int_ceil_div(new_size, sizeof(int))];
        }
        CUDA_HOS_DEV  ~DeAllocWrapper()
        {
          delete[] buf;
        }
        CUDA_HOS_DEV DeAllocWrapper (const DeAllocWrapper &) = delete;
        CUDA_HOS_DEV DeAllocWrapper (DeAllocWrapper &&) = delete;
        CUDA_HOS_DEV DeAllocWrapper & operator=(const DeAllocWrapper &) = delete;
        CUDA_HOS_DEV DeAllocWrapper & operator=(DeAllocWrapper &&) = delete;
      };
      //Simple allocation/de-allocation
      //to still be GPU-compatible if needed
      //(new and delete valid on GPU code...),
      //while still preventing leaks,
      //all without pulling in unique_ptr.

      DeAllocWrapper wrapper(etaPhiToCell.finish_initializing_buffer_size());

      etaPhiToCell.finish_initializing(wrapper.buf);

    }

    constexpr int get_closest_cell(const int sampling, const float test_eta, const float test_phi) const
    {
      int cells[EtaPhiToCellMap::s_max_overlap_cells] = {};

      const int n_cells = etaPhiToCell.get_possible_cells_from_coords(sampling, test_eta, test_phi, cells);

      if (n_cells < 1)
        {
          return -1;
        }
      else if (n_cells == 1)
        {
          return cells[0];
        }
      else
        {
          float distance = 1e38f;
          int ret = -1;

          for (int i = 0; i < n_cells; ++i)
            {
              const int this_cell = cells[i];

              const float delta_eta = eta[this_cell] - test_eta;
              const float delta_phi = Helpers::angular_difference(phi[this_cell], test_phi);

              const float this_dist = delta_eta * delta_eta + delta_phi * delta_phi;
              if (this_dist < distance || (this_dist == distance && this_cell > ret))
                {
                  distance = this_dist;
                  ret = this_cell;
                }
            }
          return ret;
        }
    }
  };

  struct CellNoiseArr
  {
    float noise[NumGainStates][NCaloCells];
  };

}

#endif
