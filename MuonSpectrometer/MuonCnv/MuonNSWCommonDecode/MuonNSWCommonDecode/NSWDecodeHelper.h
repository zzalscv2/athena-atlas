/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/
#ifndef _MUON_NSW_DECODE_HELPER_H_
#define _MUON_NSW_DECODE_HELPER_H_

#include <stdint.h>
#include <stdexcept>

#include <sstream>
#include <CxxUtils/span.h>

namespace Muon
{
  namespace nsw
  {
    enum channel_type
    {
      OFFLINE_CHANNEL_TYPE_PAD = 0,
      OFFLINE_CHANNEL_TYPE_STRIP = 1,
      OFFLINE_CHANNEL_TYPE_WIRE = 2
    };

    enum vmm_channels
    {
      VMM_per_MMFE8 = 8,
      VMM_per_sFEB  = 3,
      VMM_per_pFEB  = 3,
      VMM_channels  = 64
    };

    namespace helper
    {
      uint32_t get_bits (uint32_t word, uint32_t mask, uint8_t position);
      //uint32_t set_bits (uint32_t word, uint32_t mask, uint8_t position);
    }
  }
  // Helper function to replace placeholders in a string                       
  template<typename T>
    std::string format(const std::string& str, const T& arg) {
    std::stringstream ss;
    std::size_t pos = str.find("{}");
    if (pos == std::string::npos) {
      ss << str;
    } else {
      ss << str.substr(0, pos) << arg << str.substr(pos + 2);
    }
    return ss.str();
  }
  
  // Variadic template                                                                                                                                          
  template<typename T, typename... Args>
    std::string format(const std::string& str, const T& arg, const Args&... args) {
    std::stringstream ss;
    std::size_t pos = str.find("{}");
    if (pos == std::string::npos) {
      ss << str;
    } else {
      ss << str.substr(0, pos) << arg << format(str.substr(pos + 2), args...);
    }
    return ss.str();
  }
  template <typename T, typename X>
  T bit_slice(const X words[], int start, int end){
    //start and end are the positions in the entire stream
    //start and end included;
    int wordSize = sizeof(X)*8;
    T s = 0;
    int n = end / wordSize;
    for(int i= 0; i <= n; ++i){
      s = (s << wordSize) + words[i]; //if T is too small, does not care, it's user fault
      //when a fragment is splitted between N words, T should be at least of the size of N*words (in order to accomodate it)
    }
    s >>= (n+1) * wordSize - (end+1);
    T mask = (((T)1) << (end - start + 1))- 1; //len = end - start + 1
    s &= mask;
    return s;
  }
  /**
   * @brief Decode bits from data of words
   *
   * Start and end included
   *
   * @tparam Target Type of the value that is decoded
   * @tparam Source Type of the words in the data
   * @param words Data
   * @param start Start of value in bits
   * @param end Stop of value in bits
   * @return Target Decoded value
   */
  template <typename Target, typename Source>
  Target safe_bit_slice(const CxxUtils::span<const Source> words,
                          const std::size_t start, const std::size_t end) {
    // start and end are the positions in the entire stream
    // start and end included;
    const auto wordSize = sizeof(Source) * 8;
    auto s = Target{};
    const auto n = end / wordSize;
    const auto m = start / wordSize;
    if (end < start) {
      throw std::runtime_error(
          format("End must be larger than start ({} vs {})", start, end));
    }
    if (n >= std::size(words)) {
      throw std::runtime_error(
          format("End index ({}) out of range (number of words: {}, maximum allowed index: {})", n,
                      std::size(words), std::size(words) - 1));
    }
    if (sizeof(Target) < sizeof(Source) * (n - m + 1)) {
      throw std::runtime_error(
          format("Target type (size {}) too small to fit result of "
                      "bit_slice given start {} and end {} and source size {}",
                      sizeof(Target), start, end, sizeof(Source)));
    }
    for (auto i = m; i <= n; ++i) {
      s = (s << wordSize) + words[i];
    }
    s >>= (n + 1) * wordSize - (end + 1);
    // len = end - start + 1
    const Target mask = ((Target{1}) << (end - start + 1)) - 1;
    s &= mask;
    return s;
  }
  /**
   * @brief Decode bits from data of words and advance the read pointer
   *
   * @tparam Target Type of the value that is decoded
   * @tparam Source Type of the words in the data
   * @param words Data
   * @param start Read pointer (start of value in bits), increased by function
   * @param size Number of bits to be decoded
   * @return Target Decoded value
   */
  template <typename Target, typename Source>
  constexpr Target decode_and_advance(const CxxUtils::span<const Source> words,
                            std::size_t& start, const std::size_t size) {
    const auto res =
        safe_bit_slice<Target, Source>(words, start, start + size - 1);
    start += size;
    return res;
  }
  /// @brief Returns the most left hand bit which is set in a number
  /// @tparam T Any built-in data type
  /// @param number value
  /// @return  Set bit. -1 if no bit is set
  template <class T>
    constexpr int8_t max_bit(const T &number) {
        constexpr int8_t num_bits = sizeof(number) * 8 - 1;
        for (int8_t bit = num_bits; bit >= 0; --bit) {
            if (number & (1 << bit))
                return bit;
        }
        return -1;
  }
  /// @brief Returns the most right hand bit which is set in a number
  /// @tparam T  Any built-in data type
  /// @param number value
  /// @return Position of the bit. -1 if no bit is set
  template <class T>
    constexpr int8_t min_bit(const T &number) {
    constexpr int8_t num_bits = sizeof(number) * 8 - 1;  
    for (size_t bit = 0; bit <= num_bits; ++bit) {
      if (number & (1 << bit))
	return bit;
    }
    return -1;
  }
  template <class Out>
    constexpr Out fill_bitmask(const uint8_t first_bit,  const uint8_t num_bits) {
    Out mask {0};
    for (uint8_t bit = first_bit; bit <= first_bit + num_bits ; ++bit){
      mask |= (1<<bit);
    }
    return mask;
  }

}

inline uint32_t Muon::nsw::helper::get_bits (uint32_t word, uint32_t mask, uint8_t position)
{
  return (word >> position) & mask;
}

//inline uint32_t Muon::nsw::helper::set_bits (uint32_t word, uint32_t setbits, uint32_t mask)
//{
//  return word; //TODO 
//}

#endif // _MUON_NSW_DECODE_HELPER_H_


