//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//


#ifndef CALORECGPU_CALOTOPOCLUSTERSPLITTERGPU_H
#define CALORECGPU_CALOTOPOCLUSTERSPLITTERGPU_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "CLHEP/Units/SystemOfUnits.h"

#include "CxxUtils/checker_macros.h"

#include "CaloRecGPU/CaloClusterGPUProcessor.h"
#include "CaloRecGPU/CaloGPUTimed.h"
#include "CaloTopoClusterSplitterGPUImpl.h"
#include <string>
#include <mutex>

/**
 * @class CaloTopoClusterSplitterGPU
 * @author Cosmin-Gabriel Samoila <cosmin.samoila@cern.ch> (Algorithm & Implementation)
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch> (Interfacing with the rest of the GPU code)
 * @date 08 August 2022
 * @brief Cluster splitter algorithm to be run on GPUs.
 */


class CaloTopoClusterSplitterGPU :
  public AthAlgTool, virtual public CaloClusterGPUProcessor, public CaloGPUTimed
{
 public:

  CaloTopoClusterSplitterGPU(const std::string & type, const std::string & name, const IInterface * parent);

  virtual StatusCode initialize() override;

  virtual StatusCode execute (const EventContext & ctx,
                              const CaloRecGPU::ConstantDataHolder & constant_data,
                              CaloRecGPU::EventDataHolder & event_data,
                              void * temporary_buffer) const override;

  virtual StatusCode finalize() override;

  virtual ~CaloTopoClusterSplitterGPU();

  virtual size_t size_of_temporaries() const
  {
    return sizeof(GPUSplitterTemporaries);
  };

 private:


  /**
   * @brief vector of names of the calorimeter samplings to consider
   * for seeds.
   *
   * The default is to use all calorimeter samplings. Excluding a
   * sampling from this vector prevents the definition of a seed cell
   * in this sampling. Cells in those samplings are still used and
   * incorporated in the topo clusters (both on the neighbor and the
   * cell level) they can therefore even expand a cluster but not seed
   * one ...*/
  Gaudi::Property<std::vector<std::string>>  m_samplingNames {this, "SamplingNames", {}, "Name(s) of Calorimeter Samplings to consider for local maxima"};

  /**
   * @brief vector of names of the secondary calorimeter samplings to
   * consider.
   *
   * Samplings in this list will be considered for local maxima only
   * if no local max in the primary list is overlapping. By default this
   * list is empty  */
  Gaudi::Property<std::vector<std::string>> m_secondarySamplingNames {this, "SecondarySamplingNames", {}, "Name(s) of secondary Calorimeter Samplings to consider for local maxima"};

  /**
   * @brief local maxima need at least this number of neighbors to
   * become seeds
   *
   * each cell above the energy cut having at least this many
   * neighbors in the parent cluster and only neighbors with smaller
   * energy seed a split cluster. */
  Gaudi::Property<int> m_nCells {this, "NumberOfCellsCut", 4, "Local maxima need at least this number of neighbors to become seeds"};

  /**
   * @brief local maxima need at least this energy content
   *
   * potential seed cells have to pass this cut on the energy
   * content. */
  Gaudi::Property<float> m_minEnergy {this, "EnergyCut", 500 * CLHEP::MeV, "Minimal energy for a local max"};


  /**
   * @brief share cells at the border between two local maxima
   *
   * this property needs to be set to true in order to treat cells
   * which would be included in 2 clusters (for more then 2 the 2 with
   * the largest E for the current seed cells are used) as shared
   * cells. Shared cells are first excluded from the clustering and
   * then clustered after all normal cells are clustered. The shared
   * clusters are added to the 2 clusters they neighbor with the
   * weights \f$w_1 = E_1/(E_1+r E_2)\f$ and \f$w_2 = 1-w_1\f$, where
   * \f$E_{1,2}\f$ are the current energies of the 2 neighboring
   * clusters without the shared cells and \f$r=\exp(d_1-d_2)\f$ is
   * the ratio of the expected dependencies on the distances \f$d_i\f$
   * (in units of a typical em shower scale) of each shared cell to
   * the cluster centers. If the property is set to false the border
   * cells are included in the normal clustering and the cluster with
   * the largest E for the current seed cells gets the current border
   * cell. */
  Gaudi::Property<bool> m_shareBorderCells {this, "ShareBorderCells", false, "Whether or not to share cells at the boundary between two clusters"};


  /**
   * @brief typical EM shower scale to use for distance criteria in
   * shared cells
   *
   * a shared cell is included in both clusters neighboring the cell
   * with weights depending on the cluster energies and the distance
   * of the shared cell to the cluster centroids. The distance is
   * measured in units of this property to roughly describe the
   * exponential slope of the energy density distribution for em
   * showers. The exact choice of this property is not critical but
   * should roughly match the Moliere radius in the LArEM since here
   * the sharing of cells has the biggest use case. */
  Gaudi::Property<float> m_emShowerScale {this, "EMShowerScale", 5 * CLHEP::cm, "Typical EM shower distance for which the energy density should drop to 1/e"};

  /**
   * @brief if set to true, splitter only looks at absolute
   * value of Energy in order to identify potential seed cells */
  Gaudi::Property<bool> m_absOpt {this, "WeightingOfNegClusters", false, "Should absolute value be used to identify potential seed cells"};

  /**
   * @brief if set to true treat cells with a dead OTX which can be
   * predicted by L1 trigger info as good instead of bad cells */
  Gaudi::Property<bool> m_treatL1PredictedCellsAsGood {this, "TreatL1PredictedCellsAsGood", true, "Treat bad cells with dead OTX if predicted from L1 as good"};

  /** @brief Options for the algorithm, held in a GPU-friendly way.
  */
  GPUSplitterOptionsHolder m_options;

};

#endif //CALORECGPU_CALOTOPOCLUSTERSPLITTERGPU_H