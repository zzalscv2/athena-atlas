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
 #include <iostream>
 class PixelModuleData;
 //stream insertion to output all values from PixelModuleData
 std::ostream & operator << (std::ostream &out, const PixelModuleData &c);
 //stream extraction to get all values into PixelModuleData from string or file
 std::istream & operator >> (std::istream &in, PixelModuleData &c);
 //input pseudo-manipulator to read in soshi's file format
 struct SoshiFormat{
   std::istream * m_is{};
   bool run1=false;
   SoshiFormat() = default;
   SoshiFormat(bool run1arg):m_is{}, run1(run1arg){};
   friend SoshiFormat & operator>>(std::istream & i, SoshiFormat & f){
     f.m_is = &i;
     return f;
   }
 };
 std::istream & operator >>(SoshiFormat & f, PixelModuleData & md);
 #endif
 