/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGEOUTILS_BITUTILS_H
#define ACTSGEOUTILS_BITUTILS_H

/// Returns the position of the most left bit which is set to 1
template <typename T> constexpr int maxBit(const T &number) {
    for (int bit = sizeof(number) * 8 - 1; bit >= 0; --bit) {
        if (number & (1 << bit)) return bit;
    }
    return -1;
}
/// Returns the position of the most right bit which is set to 1
template <typename T> constexpr int minBit(const T &number) {
    for (unsigned int bit = 0; bit <= sizeof(number) * 8 - 1; ++bit) {
        if (number & (1 << bit)) return bit;
    }
    return -1;
}
#endif
