/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file  AthAllocators/ArenaBlockAlignDetail.h
 * @author C. anastopoulos with help from scott snyder
 * @date July 2023
 * @brief A dummy pad struct to put at the end of the
 * ArenaBlock header to ensure the alignment of the elements.
 * Plus some constexpr that we can static assert on.
 * The block is page aligned ~ 4096 or so, what we
 * want is to control the alignment of the elements,
 * at least up to a reason ...
 */

#ifndef ATLALLOCATORS__ARENABLOCKALIGNDETAIL
#define ATLALLOCATORS__ARENABLOCKALIGNDETAIL

#include <cstddef>
namespace SG {
namespace ArenaBlockAlignDetail {

struct padForAlign {
  // gcc and clang seem to define this
  // But not the same way ...
  // gcc provides 16,32,64 as we change arch
  // clang seem to provide 16 always...
  // From the nature of the Block Header (other elements total size 24)
  // 16 vs 32 costs the same in space overhead.
  // So minimum 32
#if defined(__BIGGEST_ALIGNMENT__) && (__BIGGEST_ALIGNMENT__ > 32)
  alignas(__BIGGEST_ALIGNMENT__) double dummy;
#else
  alignas(32) double dummy;
#endif
};

constexpr size_t alignment = std::alignment_of<padForAlign>::value;

}  // namespace ArenaBlockAlignDetail
}  // namespace SG
#endif
