/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// SCT_ReadoutScheme.cxx
//   Implementation file for class SCT_ReadoutScheme
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Version 1.0 23/5/2001 Alessandro Fornaini
// Modified: Grant Gorfine
///////////////////////////////////////////////////////////////////

#include "SCT_ReadoutGeometry/SCT_ReadoutScheme.h"

namespace InDetDD {

// Copy constructor:
SCT_ReadoutScheme::SCT_ReadoutScheme(const SCT_ReadoutScheme &scheme) 
  
= default;

// Constructor with parameters:
SCT_ReadoutScheme::SCT_ReadoutScheme(const int crystals,const int diodes,
				     const int cells,const int shift) :
  m_crystals(crystals),
  m_diodes(diodes),
  m_cells(cells),
  m_shift(shift)
{ 
  //check if parameters are consistent
  if (m_crystals<1) m_crystals=1;
  if (m_diodes<0) m_diodes=0;
  if (m_cells<0) m_cells=0;
}

// Assignment operator:
SCT_ReadoutScheme &SCT_ReadoutScheme::operator=(const SCT_ReadoutScheme &scheme)
{
  if (this!=&scheme) {
    m_crystals=scheme.m_crystals;
    m_diodes=scheme.m_diodes;
    m_cells=scheme.m_cells;
    m_shift=scheme.m_shift;
  } else {}
  return *this;
}


// diode id -> readout id
SiReadoutCellId 
SCT_ReadoutScheme::readoutIdOfCell(const SiCellId & cellId) const
{
  // readout cell and diode numbering are the same.
  // The range can be different due to the unconnected edge strips (if m_shift != 0).
  if (!cellId.isValid()) return {}; // return an invalid id. 
  int strip = cellId.strip();
  // I think this should be >= m_cells. But need to check if this has any implications.
  if (strip < 0 || strip > m_cells) return {}; // return an invalid id.
  return {strip};
}

} // namespace InDetDD
