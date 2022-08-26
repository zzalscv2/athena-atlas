// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
//

#ifndef TOPOAUTOMATONCLUSTEROPTIMIZER_EXTRA_DEFS_H
#define TOPOAUTOMATONCLUSTEROPTIMIZER_EXTRA_DEFS_H

using namespace CaloRecGPU;

class ExtraTags : public Tags
{
  public:
  inline constexpr static tag_type set_for_terminal_propagation2(const tag_type tag)
  {
    return (tag - tag_propagation_delta) & (~ValidPropagationMark);
  }
  inline constexpr static tag_type update_non_terminal_tag(const tag_type old_tag, const tag_type new_tag)
  {
    return clear_everything_but_counter(old_tag) | new_tag;
  }

  inline constexpr static tag_type update_terminal_tag(const tag_type old_tag, const tag_type new_tag)
  {
    return clear_everything_but_counter(old_tag) | new_tag; 
  }
  
  inline constexpr static tag_type update_terminal_tag2(const tag_type old_tag, const tag_type new_tag)
  {
    return clear_everything_but_counter(old_tag) | (new_tag & (~ValidPropagationMark)); 
  }

  inline constexpr static tag_type terminal_to_seed_tag(const tag_type old_tag)
  {
    return old_tag | ValidPropagationMark;
  }

  /*
  inline constexpr static tag_type set_for_neighbour_propagation(const tag_type tag)
  {
    const uint32_t count = get_counter_from_tag(tag) - 1;
    const uint32_t index = get_index_from_tag(tag);
    const uint32_t snr = get_snr_from_tag(tag);
    tag_type ret = count;
    ret = (ret << 31) | snr; //MSB of snr is 0
    ret = (ret << 16) | index;

    //So: 0 ... 16 bit counter ... 31 bit SNR ... 16 bit index

    return ret;
  }


  inline constexpr static int32_t get_index_from_terminal_tag(const tag_type tag)
  {
    return tag & 0x0000FFFFU;
  }

  inline constexpr static int32_t get_counter_from_terminal_tag(const tag_type tag)
  {
    uint32_t ret = (tag & 0x7FFF800000000000ULL) >> 47;
    //0x 7FFF 8000 0000 0000
    return ret;
  }

  inline constexpr static int32_t clear_everything_but_counter_from_terminal_tag(const tag_type tag)
  {
    return tag & 0x7FFF800000000000ULL;
  }

  inline constexpr static int32_t get_snr_from_terminal_tag(const tag_type tag)
  {
    uint32_t ret = (tag & 0x00007FFFFFFF0000ULL) >> 16;
    //0x 0000 7FFF FFFF 0000
    return ret;
  }

  inline constexpr static tag_type update_non_terminal_tag(const tag_type old_tag, const tag_type new_tag)
  {
    return clear_everything_but_counter(old_tag) | new_tag;
  }

  inline constexpr static tag_type update_terminal_tag(const tag_type old_tag, const tag_type new_tag)
  {
    const uint32_t index = get_index_from_tag(new_tag);
    const uint32_t snr = get_snr_from_tag(new_tag);
    tag_type ret = snr;
    ret = (ret << 16) | index;
    return clear_everything_but_counter_from_terminal_tag(old_tag) | ret;
  }

  inline constexpr static tag_type terminal_to_seed_tag(const tag_type old_tag)
  {
    tag_type ret = get_snr_from_terminal_tag(old_tag);


    const uint32_t index = get_index_from_terminal_tag(old_tag);

    ret = (ret << 16) | index;

    ret = (ret << 16);

    return ret | ValidPropagationMark;

  }
  */
  
};

#endif