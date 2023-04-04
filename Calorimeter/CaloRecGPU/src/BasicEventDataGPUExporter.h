//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_BASICEVENTDATAGPUEXPORTER_H
#define CALORECGPU_BASICEVENTDATAGPUEXPORTER_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "CaloRecGPU/CaloClusterGPUTransformers.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "CaloRecGPU/CaloGPUTimed.h"

class CaloCell_ID;

/**
 * @class BasicEventDataGPUExporter
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 29 May 2022
 * @brief Standard tool to export cell energy and gain to the GPU.
 *
 * @warning The two gaussian noise for evaluating the seed cell cuts
 * based on out of time seeds is not implemented! We currently assume
 * noise as estimated without that flag.
 *
 */

class BasicEventDataGPUExporter :
  public AthAlgTool, virtual public ICaloClusterGPUInputTransformer, public CaloGPUTimed
{
 public:

  BasicEventDataGPUExporter(const std::string & type, const std::string & name, const IInterface * parent);

  virtual StatusCode initialize() override;

  virtual StatusCode convert (const EventContext & ctx, const CaloRecGPU::ConstantDataHolder & constant_data,
                              const xAOD::CaloClusterContainer * cluster_collection, CaloRecGPU::EventDataHolder & event_data) const override;

  virtual StatusCode finalize() override;

  virtual ~BasicEventDataGPUExporter();

 private:

  /** @brief If @p true, do not delete the CPU version of the GPU-friendly data representation.
   *  Defaults to @p true.
   *
   */
  Gaudi::Property<bool> m_keepCPUData {this, "KeepCPUData", true, "Keep CPU version of GPU data format"};

  /**
   * @brief vector of names of the cell containers to use as input.
   */
  Gaudi::Property<SG::ReadHandleKey<CaloCellContainer>> m_cellsKey {this, "CellsName", "", "Name(s) of Cell Containers"};

  /** @brief If @p true, into account the possibility of a cell being shared between clusters.
   *  Hurts performance when not needed.
   *  Defaults to @p false.
   *
   */
  Gaudi::Property<bool> m_considerSharedcells {this, "ConsiderSharedCells", false, "Take into account the possibility of a cell being shared between clusters."};


  /**
  * @brief Pointer to Calo ID Helper
  */
  const CaloCell_ID * m_calo_id {nullptr};

};

#endif //CALORECGPU_BASICEVENTDATAGPUEXPORTER_H
