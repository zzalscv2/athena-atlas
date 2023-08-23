/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file PixelModuleDataStream.h
 * @author Shaun Roe
 * @date August, 2023
 * @brief Stream insertion operator for PixelModuleData
 */
 #ifndef PixelModuleDataStream_h
 #define PixelModuleDataStream_h
 #include <iosfwd>
 class PixelModuleData;
 //stream insertion to output all values from PixelModuleData
 std::ostream & operator << (std::ostream &out, const PixelModuleData &c);
 //stream extraction to get all values into PixelModuleData from string or file
 std::istream & operator >> (std::istream &in, PixelModuleData &c);
 #endif
 