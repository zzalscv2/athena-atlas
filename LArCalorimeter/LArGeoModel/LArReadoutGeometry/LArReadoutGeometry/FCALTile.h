/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARREADOUTGEOMETRY_FCALTILE_H
#define LARREADOUTGEOMETRY_FCALTILE_H

#include "LArReadoutGeometry/FCAL_ChannelMap.h"
#include "LArHV/FCALHVModule.h"
#include "LArReadoutGeometry/FCALTubeConstLink.h"
#include "CxxUtils/CachedPointer.h"
#include "CxxUtils/CachedValue.h"

class FCALModule;

/**
 * @class FCALTile
 *
 * @brief A tile of the forward calorimeter readout geometry
 *
 * This class represents the size, shape, position, and
 * indices of a single tile within the FCAL.  For access to
 * the FCAL Tiles, you should use the FCAL Module class.
 */

class FCALTile 
{
  typedef FCAL_ChannelMap::tileMap_const_iterator TileConstIterator;
  
 public:

  /** 
   * @brief Copy constructor
   */
  FCALTile(const FCALTile &right);

  /** 
   * @brief Constructor
   */
  FCALTile (const FCALModule *module, FCALTile::TileConstIterator tIterator);

  /** 
   * @brief Destructor
   */
  ~FCALTile();

  /** 
   * @brief Assignment
   */
  FCALTile & operator=(const FCALTile &right);


  /** 
   * @brief Gets the X Position of the Tile
   */
  double getX () const;

  /** 
   * @brief Gets the Y position of the Tile.
   */
  double getY () const;

  /** 
   * @brief Gets the number of tubes in the tile.
   */
  unsigned int getNumTubes () const;

  /** 
   * @brief Returns the index "i" of the tile.
   */
  int getIndexI () const;

  /** 
   * @brief Returns the index "J" of the tile.
   */
  int getIndexJ () const;

  /** 
   * @brief Returns the identifier of this tile (convention:  comes from the FCAL Channel Map).
   */
  unsigned int identify () const;

  /** 
   * @brief For sorting operations:
   */
  bool operator < (const FCALTile & t) const { return ((*m_tile).first) < (*t.m_tile).first; }

  /**
   * @brief Get the Module
   **/
  const FCALModule *getModule() const;

  /**
   * @brief Get a tube
   */
  FCALTubeConstLink getTube (unsigned int i) const;
  
  /**
   * @brief Get num hvlines
   */
  static unsigned int getNumHVLines() ;
  
  /**
   * @brief Get hvline
   */
  const FCALHVLine* getHVLine (unsigned int i) const;
          
 private:

  // Iterator to a tile within the FCAL Channel Map.
  const FCALModule *m_module;
  TileConstIterator m_tile;
      
  /**
   * @brief	Cache of subgaps.
   */
  CxxUtils::CachedPointer<const FCALHVLine> m_line[4];

  /**
   * @brief	Cache of tubes.
   */
  CxxUtils::CachedValue<std::vector<FCALTubeConstLink> > m_tube;

  
      
};



#endif
