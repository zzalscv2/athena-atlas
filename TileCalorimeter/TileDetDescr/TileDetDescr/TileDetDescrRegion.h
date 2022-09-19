/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 Tile Calorimeter detector description package
 -----------------------------------------
***************************************************************************/

#ifndef TILEDETDESCR_TILEDETDESCRREGION_H
#define TILEDETDESCR_TILEDETDESCRREGION_H

#include "Identifier/Identifiable.h"

#include "TileDetDescr/TileDetDescriptor.h"

/**
   @section s1 Class TileDetDescrRegion

   This class provides an interface to detector description
   information for the regions (sub-detectors) of the  Tile calorimeter.
   Each instance of an TileDetDescrRegion corresponds to the description for a SINGLE region.

   @section s2 Information available:

   Identifier: Each TileDetDescrRegion has an Identifier which
   uniquely identifies to which region it corresponds, i.e. which
   sub-detector and positive/negative half, (See classes Tile_ID, ... in TileDetDescr/TileID.h for
   more info.).

   Print: A general print method is available.

   @section s3 Object ownership:

   Clients are NOT responsible for deleting the objects connected
   to the pointers received for DetectorPosition or TileDetDescriptor
   objects.
*/
class TileDetDescrRegion : public Identifiable
{
public:

  typedef TileDetDescriptor descriptor_type;

  TileDetDescrRegion(void) : m_descriptor(0) {}

  TileDetDescrRegion(const Identifier& id, const descriptor_type* descriptor)
          : m_id(id) , m_descriptor(descriptor) {}

  virtual Identifier          identify() const { return m_id; }
  const descriptor_type*      descriptor() const { return m_descriptor; }
  void                        print() const;

private:

  Identifier                  m_id;
  const descriptor_type*      m_descriptor;
};

#endif // TILEDETDESCR_TILEDETDESCRREGION_H
