/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//Dear emacs, this is -*-c++-*-

#ifndef CALORECGPU_BASICCONSTANTGPUDATAEXPORTER_H
#define CALORECGPU_BASICCONSTANTGPUDATAEXPORTER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "CaloRecGPU/CaloClusterGPUTransformers.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "CaloRecGPU/CaloGPUTimed.h"
#include "CaloConditions/CaloNoise.h"
#include "CaloDetDescr/CaloDetDescrManager.h"

/**
 * @class BasicConstantGPUDataExporter
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 29 May 2022
 * @brief Standard tool to export calorimeter geometry and cell noise to GPU.
 *
 * For the time being, this must be run on first event, so that the noise tool exists and so on.
 * Hopefully we can find a way around this in the future.
 */

class BasicConstantGPUDataExporter :
  public AthAlgTool, virtual public ICaloClusterGPUConstantTransformer, public CaloGPUTimed
{
 public:

  BasicConstantGPUDataExporter(const std::string & type, const std::string & name, const IInterface * parent);

  virtual StatusCode initialize() override;

  virtual StatusCode convert (ConstantDataHolder & constant_data) const override;

  virtual StatusCode convert (const EventContext & ctx, ConstantDataHolder & constant_data) const override;

  virtual StatusCode finalize() override;
  
  virtual ~BasicConstantGPUDataExporter();

 private:

  /** @brief If @p true, do not delete the CPU version of the GPU-friendly data representation.
   *  Defaults to @p true.
   *
   */

  Gaudi::Property<bool> m_keepCPUData {this, "KeepCPUData", true, "Keep CPU version of GPU data format"};

  /** @brief If @p true, check for and correct for cases where the neighbourhood relations between the cells
      are not symmetric, that is, cell A is given as neighbour to cell B, but cell B is not given as neighbour to cell A
      (which is a basic assumption necessary for the GPU automaton-based algorithm to work properly)
  */
  //Gaudi::Property<bool> m_correctNonSymmetricNeighs {this, "CorrectNonSymmetricNeighbors", false, "Detect and correct non-symmetric neighbours"};
  //No longer necessary.

  /** @brief Key of the CaloNoise Conditions data object. Typical values
      are '"electronicNoise', 'pileupNoise', or '"totalNoise' (default) */

  SG::ReadCondHandleKey<CaloNoise> m_noiseCDOKey{this, "CaloNoiseKey", "totalNoise", "SG Key of CaloNoise data object"};

  /**
   * @brief Key for the CaloDetDescrManager in the Condition Store
   */
  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey { this
      , "CaloDetDescrManager"
      , "CaloDetDescrManager"
      , "SG Key for CaloDetDescrManager in the Condition Store" };

  /**
   * @brief type of neighbor relations to use.
   *
   * The CaloIdentifier package defines different types of neighbors
   * for the calorimeter cells. Currently supported neighbor relations
   * for topological clustering are:
   *
   * @li "all2D" for all cells in the same layer (sampling or module)
   *      of one calorimeter subsystem. Note that endcap and barrel
   *      will be unconnected in this case even for the LAREM.
   *
   * @li "all3D" for all cells in the same calorimeter. This means all
   *      the "all2D" neighbors for each cell plus the cells in
   *      adjacent samplings overlapping at least partially in
   *      \f$\eta\f$ and \f$\phi\f$ with the cell. Note that endcap
   *      and barrel will be connected in this case for the LAREM.
   *
   * @li "super3D" for all cells. This means all the "all3D" neighbors
   *      for each cell plus the cells in adjacent samplings from
   *      other subsystems overlapping at least partially in
   *      \f$\eta\f$ and \f$\phi\f$ with the cell. All calorimeters
   *      are connected in this case.
   *
   * The default setting is "super3D".  */
  Gaudi::Property<std::string> m_neighborOptionString {this, "NeighborOption", "super3D",
                                                       "Neighbor option to be used for cell neighborhood relations"};
  LArNeighbours::neighbourOption m_neighborOption;


  /**
   * @brief if set to true limit the neighbors in HEC IW and FCal2&3.
   *
   * The cells in HEC IW and FCal2&3 get very large in terms of eta
   * and phi.  Since this might pose problems on certain jet
   * algorithms one might need to avoid expansion in eta and phi for
   * those cells. If this property is set to true the 2d neighbors of
   * these cells are not used - only the next sampling neighbors are
   * probed. */
  Gaudi::Property<bool> m_restrictHECIWandFCalNeighbors {this, "RestrictHECIWandFCalNeighbors",
                                                         false, "Limit the neighbors in HEC IW and FCal2&3"};

  /**
   * @brief if set to true limit the neighbors in presampler Barrel and Endcap.
   *
   * The presampler cells add a lot of PileUp in the Hilum
   * samples. With this option set to true the presampler cells do not
   * expand the cluster in the presampler layer.  Only the next
   * sampling is used as valid neighbor source. */
  Gaudi::Property<bool> m_restrictPSNeighbors {this, "RestrictPSNeighbors",
                                                         false, "Limit the neighbors in presampler Barrel and Endcap"};



  bool m_hasBeenInitialized;

};

#endif //CALORECGPU_BASICCONSTANTGPUDATAEXPORTER_H
