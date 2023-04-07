//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

//Dear emacs, this is -*-c++-*-

#ifndef CALORECGPU_GPUTOATHENAIMPORTERWITHMOMENTS_H
#define CALORECGPU_GPUTOATHENAIMPORTERWITHMOMENTS_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "CaloRecGPU/CaloClusterGPUTransformers.h"
#include "CaloRecGPU/CaloGPUTimed.h"
#include "StoreGate/ReadHandleKey.h"

#include "CaloDetDescr/CaloDepthTool.h"
#include "CaloInterface/ILArHVFraction.h"
#include <stdexcept>

class CaloCell_ID;

/**
 * @class GPUToAthenaImporterWithMoments
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 30 May 2022
 * @brief Tool to convert the GPU data representation back to CPU, with selected moments too.
 *
 */

class GPUToAthenaImporterWithMoments :
  public AthAlgTool, virtual public ICaloClusterGPUOutputTransformer, public CaloGPUTimed
{
 public:

  GPUToAthenaImporterWithMoments(const std::string & type, const std::string & name, const IInterface * parent);

  virtual StatusCode initialize() override;

  virtual StatusCode convert (const EventContext & ctx, const CaloRecGPU::ConstantDataHolder & constant_data,
                              CaloRecGPU::EventDataHolder & event_data, xAOD::CaloClusterContainer * cluster_collection) const override;

  virtual StatusCode finalize() override;

  virtual ~GPUToAthenaImporterWithMoments();

 private:

  /** @brief If @p true, do not delete the GPU data representation.
   *  Defaults to @p true.
   *
   */
  Gaudi::Property<bool> m_keepGPUData {this, "KeepGPUData", true, "Keep GPU allocated data"};

  /**
   * @brief vector of names of the cell containers to use as input.
   */
  Gaudi::Property<SG::ReadHandleKey<CaloCellContainer>> m_cellsKey {this, "CellsName", "", "Name(s) of Cell Containers"};

  /// Cluster size. Should be set accordingly to the threshold.
  Gaudi::Property<std::string> m_clusterSizeString {this, "ClusterSize", "Topo_420", "The size/type of the clusters"};

  xAOD::CaloCluster::ClusterSize m_clusterSize;
  
  /**
   * @brief Pointer to Calo ID Helper
   */
  const CaloCell_ID* m_calo_id {nullptr};

  /**
   * @brief Key for the CaloDetDescrManager in the Condition Store
   */
  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey{this, "CaloDetDescrManager", "CaloDetDescrManager",
                                                          "SG Key for CaloDetDescrManager in the Condition Store"};
  
  //Tool handles for things we can't (yet) do on the GPU.
  
  ToolHandle<CaloDepthTool> m_caloDepthTool{"CaloDepthTool", this};
  ToolHandle<ILArHVFraction> m_larHVFraction{this, "LArHVFraction", {}, "Tool Handle for LArHVFraction"};

  
  
  /** 
   * @brief vector holding the input list of names of moments to
   * calculate.
   *
   * This is the list of desired names of moments given in the
   * jobOptions.*/
  Gaudi::Property<std::vector<std::string>>  m_momentsNames{this, "MomentsNames", {}, "List of names of moments to calculate"}; 
  
    
  struct MomentsOptionsArray
  {
    static constexpr int num_moments = 72;
    bool array[num_moments]{};
    //Initialize to false.
    
    static constexpr int moment_to_linear(const xAOD::CaloCluster::MomentType moment)
    {
      switch (moment)
        {
          default:
            return num_moments;
          case xAOD::CaloCluster::FIRST_PHI:
            return 0;
          case xAOD::CaloCluster::FIRST_ETA:
            return 1;
          case xAOD::CaloCluster::SECOND_R:
            return 2;
          case xAOD::CaloCluster::SECOND_LAMBDA:
            return 3;
          case xAOD::CaloCluster::DELTA_PHI:
            return 4;
          case xAOD::CaloCluster::DELTA_THETA:
            return 5;
          case xAOD::CaloCluster::DELTA_ALPHA:
            return 6;
          case xAOD::CaloCluster::CENTER_X:
            return 7;
          case xAOD::CaloCluster::CENTER_Y:
            return 8;
          case xAOD::CaloCluster::CENTER_Z:
            return 9;
          case xAOD::CaloCluster::CENTER_MAG:
            return 10;
          case xAOD::CaloCluster::CENTER_LAMBDA:
            return 11;
          case xAOD::CaloCluster::LATERAL:
            return 12;
          case xAOD::CaloCluster::LONGITUDINAL:
            return 13;
          case xAOD::CaloCluster::ENG_FRAC_EM:
            return 14;
          case xAOD::CaloCluster::ENG_FRAC_MAX:
            return 15;
          case xAOD::CaloCluster::ENG_FRAC_CORE:
            return 16;
          case xAOD::CaloCluster::FIRST_ENG_DENS:
            return 17;
          case xAOD::CaloCluster::SECOND_ENG_DENS:
            return 18;
          case xAOD::CaloCluster::ISOLATION:
            return 19;
          case xAOD::CaloCluster::ENG_BAD_CELLS:
            return 20;
          case xAOD::CaloCluster::N_BAD_CELLS:
            return 21;
          case xAOD::CaloCluster::N_BAD_CELLS_CORR:
            return 22;
          case xAOD::CaloCluster::BAD_CELLS_CORR_E:
            return 23;
          case xAOD::CaloCluster::BADLARQ_FRAC:
            return 24;
          case xAOD::CaloCluster::ENG_POS:
            return 25;
          case xAOD::CaloCluster::SIGNIFICANCE:
            return 26;
          case xAOD::CaloCluster::CELL_SIGNIFICANCE:
            return 27;
          case xAOD::CaloCluster::CELL_SIG_SAMPLING:
            return 28;
          case xAOD::CaloCluster::AVG_LAR_Q:
            return 29;
          case xAOD::CaloCluster::AVG_TILE_Q:
            return 30;
          case xAOD::CaloCluster::ENG_BAD_HV_CELLS:
            return 31;
          case xAOD::CaloCluster::N_BAD_HV_CELLS:
            return 32;
          case xAOD::CaloCluster::PTD:
            return 33;
          case xAOD::CaloCluster::MASS:
            return 34;
          case xAOD::CaloCluster::EM_PROBABILITY:
            return 35;
          case xAOD::CaloCluster::HAD_WEIGHT:
            return 36;
          case xAOD::CaloCluster::OOC_WEIGHT:
            return 37;
          case xAOD::CaloCluster::DM_WEIGHT:
            return 38;
          case xAOD::CaloCluster::TILE_CONFIDENCE_LEVEL:
            return 39;
          case xAOD::CaloCluster::SECOND_TIME:
            return 40;
          case xAOD::CaloCluster::NCELL_SAMPLING:
            return 41;
          case xAOD::CaloCluster::VERTEX_FRACTION:
            return 42;
          case xAOD::CaloCluster::NVERTEX_FRACTION:
            return 43;
          case xAOD::CaloCluster::ETACALOFRAME:
            return 44;
          case xAOD::CaloCluster::PHICALOFRAME:
            return 45;
          case xAOD::CaloCluster::ETA1CALOFRAME:
            return 46;
          case xAOD::CaloCluster::PHI1CALOFRAME:
            return 47;
          case xAOD::CaloCluster::ETA2CALOFRAME:
            return 48;
          case xAOD::CaloCluster::PHI2CALOFRAME:
            return 49;
          case xAOD::CaloCluster::ENG_CALIB_TOT:
            return 50;
          case xAOD::CaloCluster::ENG_CALIB_OUT_L:
            return 51;
          case xAOD::CaloCluster::ENG_CALIB_OUT_M:
            return 52;
          case xAOD::CaloCluster::ENG_CALIB_OUT_T:
            return 53;
          case xAOD::CaloCluster::ENG_CALIB_DEAD_L:
            return 54;
          case xAOD::CaloCluster::ENG_CALIB_DEAD_M:
            return 55;
          case xAOD::CaloCluster::ENG_CALIB_DEAD_T:
            return 56;
          case xAOD::CaloCluster::ENG_CALIB_EMB0:
            return 57;
          case xAOD::CaloCluster::ENG_CALIB_EME0:
            return 58;
          case xAOD::CaloCluster::ENG_CALIB_TILEG3:
            return 59;
          case xAOD::CaloCluster::ENG_CALIB_DEAD_TOT:
            return 60;
          case xAOD::CaloCluster::ENG_CALIB_DEAD_EMB0:
            return 61;
          case xAOD::CaloCluster::ENG_CALIB_DEAD_TILE0:
            return 62;
          case xAOD::CaloCluster::ENG_CALIB_DEAD_TILEG3:
            return 63;
          case xAOD::CaloCluster::ENG_CALIB_DEAD_EME0:
            return 64;
          case xAOD::CaloCluster::ENG_CALIB_DEAD_HEC0:
            return 65;
          case xAOD::CaloCluster::ENG_CALIB_DEAD_FCAL:
            return 66;
          case xAOD::CaloCluster::ENG_CALIB_DEAD_LEAKAGE:
            return 67;
          case xAOD::CaloCluster::ENG_CALIB_DEAD_UNCLASS:
            return 68;
          case xAOD::CaloCluster::ENG_CALIB_FRAC_EM:
            return 69;
          case xAOD::CaloCluster::ENG_CALIB_FRAC_HAD:
            return 70;
          case xAOD::CaloCluster::ENG_CALIB_FRAC_REST:
            return 71;
        }
    }
    
    bool & operator[] (const xAOD::CaloCluster::MomentType moment)
    {
      const int idx=moment_to_linear(moment); //this can return 72
      if (idx == num_moments) {
        throw std::out_of_range("index out of range in bool & MomentsOptionsArray[]");
      }
      return array[moment_to_linear(moment)];
    }
    
    bool operator[] (const xAOD::CaloCluster::MomentType moment) const
    { 
      const int idx=moment_to_linear(moment); //this can return 72
      if (idx == num_moments) {
        throw std::out_of_range("index out of range in bool MomentsOptionsArray[] const");
      }
      return array[moment_to_linear(moment)];
    }
  };
  
  /** @brief Holds (in a linearized way) the moments and whether to add them to the clusters.
             (on the GPU side, they are unconditionally calculated).
  */
  MomentsOptionsArray m_momentsToDo;
  

};

#endif //CALORECGPU_GPUTOATHENAIMPORTERWITHMOMENTS_H
