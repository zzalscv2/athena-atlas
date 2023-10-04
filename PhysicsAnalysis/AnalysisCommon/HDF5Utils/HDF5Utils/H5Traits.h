// this is -*- C++ -*-
/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef H5_TRAITS_H
#define H5_TRAITS_H

/**
 * HDF5 Traits
 *
 * This is a collection of tools to make HDF5 strongly typed
 *
 **/


namespace H5 {
  class DataType;
}

namespace H5Utils {

  /// @brief clssses to add type traits for H5
  /// @{

  namespace internal {

    /** @brief data_buffer_t
     *
     * This buffer element is used by the HDF5 library to store data
     * which is about to be written to disk.
     **/
    union data_buffer_t
    {
      int u_int;
      long u_long;
      long long u_llong;
      unsigned long u_ulong;
      unsigned long long u_ullong;
      unsigned int u_uint;
      unsigned char u_uchar;
      char u_char;
      float u_float;
      double u_double;
      bool u_bool;
      short u_short;
      unsigned short u_ushort;
    };

    /**
     * We have lots of code to get around HDF5's rather weak typing. Some
     * of this is filled out more in the cxx file.
     **/
    template <typename T> struct H5Traits;
    template <> struct H5Traits<int> {
      static const H5::DataType type;
      static int& ref(data_buffer_t& buf) { return buf.u_int; }
    };
    template <> struct H5Traits<long> {
      static const H5::DataType type;
      static long& ref(data_buffer_t& buf) { return buf.u_long; }
    };
    template <> struct H5Traits<long long> {
      static const H5::DataType type;
      static long long& ref(data_buffer_t& buf) { return buf.u_llong; }
    };
    template <> struct H5Traits<unsigned long> {
      static const H5::DataType type;
      static unsigned long& ref(data_buffer_t& buf) { return buf.u_ulong; }
    };
    template <> struct H5Traits<unsigned long long> {
      static const H5::DataType type;
      static unsigned long long& ref(data_buffer_t& buf) { return buf.u_ullong; }
    };
    template <> struct H5Traits<unsigned int> {
      static const H5::DataType type;
      static unsigned int& ref(data_buffer_t& buf) { return buf.u_uint; }
    };
    template <> struct H5Traits<unsigned char> {
      static const H5::DataType type;
      static unsigned char& ref(data_buffer_t& buf) { return buf.u_uchar; }
    };
    template <> struct H5Traits<char> {
      static const H5::DataType type;
      static char& ref(data_buffer_t& buf) { return buf.u_char; }
    };
    template <> struct H5Traits<float> {
      static const H5::DataType type;
      static float& ref(data_buffer_t& buf) { return buf.u_float; }
    };
    template <> struct H5Traits<double> {
      static const H5::DataType type;
      static double& ref(data_buffer_t& buf) { return buf.u_double; }
    };
    template <> struct H5Traits<bool> {
      static const H5::DataType type;
      static bool& ref(data_buffer_t& buf) { return buf.u_bool; }
    };
    template <> struct H5Traits<short> {
      static const H5::DataType type;
      static short& ref(data_buffer_t& buf) { return buf.u_short; }
    };
    template <> struct H5Traits<unsigned short> {
      static const H5::DataType type;
      static unsigned short& ref(data_buffer_t& buf) { return buf.u_ushort; }
    };

  }
  /// @}
}

#endif
