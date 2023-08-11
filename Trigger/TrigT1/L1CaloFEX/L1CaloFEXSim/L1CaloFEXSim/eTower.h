/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           eTower.h  -  description
//                              -------------------
//     begin                : 19 02 2019
//     email                : Alan.Watson@cern.ch, jacob.julian.kempster@cern.ch
//  ***************************************************************************/


#ifndef eTower_H
#define eTower_H

#include <vector>
#include "AthenaKernel/CLASS_DEF.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "CaloIdentifier/CaloCell_SuperCell_ID.h"

#include "xAODCore/AuxStoreAccessorMacros.h"

namespace LVL1 {
  
  //Doxygen class description below:
  /** The eTower class is an interface object for eFEX trigger algorithms
      The purposes are twofold:
      - to provide inputs to the algorithms in a way that reflects the cell
      structure within a tower (the basic element of an eFEX algorithm window)
      - to hide the details of the individual data sources, e.g. which hadronic
      cells are LAr and which are HEC, from the algorithms
      It needs to contain supercell ET values and a tower coordinate or identifier
      (not yet defined). ET values should be on the eFEX's internal ET scale, not MeV.
      This should be a purely transient object, but will need to enter the transient
      event store (so class_def etc will be needed before it can be used for real)
  */
  
  class eTower {
    
  public:
    
    /** Constructors */
    eTower();
    eTower(float eta, float phi, int id_modifier, int posneg);
    
    /** Destructor */
    virtual ~eTower() = default;
    
    /** Clear supercell ET values */
    void clearET();
    
    /** Clear and resize Identifier value vector */
    void clear_scIDs();

    /** Add to ET of a specified cell in MeV */
    void addET(float et, int cell);

    /** Add to ET of a specified cell */
    void recordMD_ET(float et, int cell);

    /** Get coordinates of tower */
    int iEta() const;
    int iPhi() const;
    float eta() const {return m_eta;};
    float phi() const {return m_phi;};
    
    void setEta(const float thiseta){ m_eta = thiseta; }

    int id() const {return m_tower_id;}

    float constid() const {return m_tower_id;};

    /** Get ET of a specified cell in MeV */
    unsigned int getET(unsigned int layer, int cell = 0) const;
    
    /** Get ET sum of all cells in the eTower in MeV */
    unsigned int getTotalET() const;

    /** Get total ET sum of all cells in a given layer in MeV */
    unsigned int getLayerTotalET(unsigned int layer) const;

    /** Get vector of ET values for a given layer in MeV */
    std::vector<unsigned int> getLayerETvec(unsigned int layer) const;
    
    /** Get vector of all ET values in MeV */
    std::vector<unsigned int> getETs() const {return m_et;};

    /** Get vector of INT which describe whether a slot shared split ET from two different supercells - required information for production of CSV input files */
    std::vector<unsigned int> getETSplits() const {return m_etSplits;};

    /** Get ET of a specified cell in MeV FLOAT VERSION */
    float getET_float(unsigned int layer, int cell = 0) const;

    /** Get ET sum of all cells in the eTower in MeV FLOAT VERSION */
    float getTotalET_float() const;

    /** Get total ET sum of all cells in a given layer in MeV FLOAT VERSION */
    float getLayerTotalET_float(unsigned int layer) const;

    /** Get vector of ET values for a given layer in MeV FLOAT VERSION */
    std::vector<float> getLayerETvec_float(unsigned int layer) const;

    /** Get vector of all ET values in MeV FLOAT VERSION */
    std::vector<float> getETs_float() const {return m_et_float;};

    void setET(int cell, float et, int layer);

    /** Set supercell position ID **/
    void setSCID(Identifier ID, int cell, float et, int layer, bool doenergysplit);

    std::vector<Identifier> getSCIDs() const { return m_scID; }
    
    std::vector<Identifier> getSCIDs_split() const { return m_scID_split; }

    Identifier getSCID(int cell) const { return m_scID[cell]; }

    std::vector<Identifier> getLayerSCIDs(unsigned int layer) const;

    void setPosNeg(int posneg);

    inline int getPosNeg() const {return m_posneg;}

    /** Internal data */
  private:
    float m_eta;
    float m_phi;
    std::vector<Identifier> m_scID;
    std::vector<Identifier> m_scID_split;
    std::vector<unsigned int> m_et;    
    std::vector<float> m_et_float;
    std::vector<unsigned int> m_etSplits;
    int m_tower_id;
    int m_posneg = 0;

  };
  
} // end of namespace

CLASS_DEF( LVL1::eTower , 32201259 , 1 )


#endif
