/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
//  PixelMapping.h
//  PixelMapping
//
//  Created by sroe on 06/12/2022.
//

#ifndef PixelMapping_h
#define PixelMapping_h

#include <string>
#include <unordered_map>
#include <array>

namespace pix{
  class PixelMapping{
  public:
    PixelMapping(const std::string & csvFilename);
    //rewrite the function but dont change the signature too much
    void 
    mapping(const std::string & geographicalID, int *hashID, int *bec, int *layer, int *phimod, int *etamod) const;
    int nModules() const;
    
  private:
    typedef std::array<int,5> Coordinates;
    std::unordered_map<std::string,Coordinates > m_internalMap;
    bool m_filled{};
  };

}//end of ibl namespace


#endif /* pixelMapping_h */
