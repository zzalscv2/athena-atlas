//
// Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
//


#ifndef CALORECGPU_TOPOAUTOMATONCLUSTERING_H
#define CALORECGPU_TOPOAUTOMATONCLUSTERING_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "CxxUtils/checker_macros.h"

#include "CaloRecGPU/CaloClusterGPUProcessor.h"
#include "CaloRecGPU/CaloGPUTimed.h"
#include "TopoAutomatonClusteringGPU.h"
#include <string>
#include <mutex>

/**
 * @class TopoAutomatonClustering
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 31 May 2022
 * @brief Topological cluster maker algorithm to be run on GPUs.
 */
 

class TopoAutomatonClustering :
  public AthAlgTool, virtual public CaloClusterGPUProcessor, public CaloGPUTimed
{
 public:

  TopoAutomatonClustering(const std::string & type, const std::string & name, const IInterface * parent);

  virtual StatusCode initialize() override;
  
  virtual StatusCode execute (const EventContext & ctx, const ConstantDataHolder & constant_data, EventDataHolder & event_data) const override;

  virtual StatusCode finalize() override;
  
  virtual ~TopoAutomatonClustering();

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
  Gaudi::Property<std::vector<std::string>>  m_samplingNames {this, "SeedSamplingNames", {}, "Name(s) of Calorimeter Samplings to consider for seeds"};

  /**
   * @brief all cells have to satisfy \f$|E| > N_{\rm cell}\,\sigma\f$
   *
   * This cut determines how much the cluster will extend beyond the
   * last cell passing the neighbor threshold. The smaller this cut is
   * the more cells will be allowed in the tail of the cluster. It
   * should be smaller or equal to the neighbor threshold. If a cell
   * passing this cut is neighbor of two or more cells passing the
   * neighbor cut it will be inserted in the cluster which has the
   * neighbor cell that was asked first for its neighbors. Since the
   * original list of seed cells is ordered in descending order of
   * \f$E/\sigma\f$ (or \f$|E|/\sigma\f$) the distance of the cell (in
   * number of cell generations passing the neighbor cut until this
   * cell will be reached) usually determines in which cluster the
   * cell will end up in. The cell cut should be lower or equal to the
   * neighbor cut.  */
  Gaudi::Property<float> m_cellThresholdOnEorAbsEinSigma {this, "CellThresholdOnEorAbsEinSigma", 0., "Cell (terminal) threshold (in units of noise Sigma)"};

  /**
   * @brief cells with \f$|E| > N_{\rm neighbor}\,\sigma\f$ extend the cluster
   *
   * This cut determines how many cells are asked for their neighbors
   * to expand the cluster. The smaller this cut is the more cells
   * will be asked for their neighbors. If a cell passing this cut is
   * neighbor of two other cells passing this cut from different
   * clusters, the two clusters are merged. The neighbor cut should be
   * lower or equal to the seed cut.  */
  Gaudi::Property<float> m_neighborThresholdOnEorAbsEinSigma {this, "NeighborThresholdOnEorAbsEinSigma", 2., "Neighbor (grow) threshold (in units of noise Sigma)"};

  /**
   * @brief cells with \f$|E| > N_{\rm seed}\,\sigma\f$ start a cluster
   *
   * This cut determines how many clusters are formed initially.  The
   * smaller this cut is the more clusters will be created.  During
   * the accumulation of cells inside the clusters it can happen that
   * clusters are merged if a cell passing the neighbor threshold
   * would be included in both clusters.  */
  Gaudi::Property<float> m_seedThresholdOnEorAbsEinSigma {this, "SeedThresholdOnEorAbsEinSigma", 4., "Seed threshold (in units of noise Sigma)"};


  /**
   * @brief if set to true seed cuts are on \f$|E|\f$ and \f$|E|_\perp\f$.
   *
   * The seed cuts and the \f$E_\perp\f$ cut on the final clusters
   * before insertion to the CaloClusterContainer will be on absolute
   * energy and absolute transverse energy if this is set to true. If
   * set to false the cuts will be on energy and transverse energy
   * instead.  */
  Gaudi::Property<bool> m_seedCutsInAbsE {this, "SeedCutsInAbsE", true, "Seed cuts in Abs E instead of E"};

  /**
   * @brief if set to true neighbor cuts are on \f$|E|\f$ and \f$|E|_\perp\f$.
   *
   * The neighbor cuts will be on absolute energy and absolute
   * transverse energy if this is set to true. If set to false the
   * cuts will be on energy and transverse energy instead.  */
  Gaudi::Property<bool> m_neighborCutsInAbsE {this, "NeighborCutsInAbsE", true, "Neighbor (grow) cuts in Abs E instead of E"};

  /**
   * @brief if set to true cell cuts are on \f$|E|\f$ and \f$|E|_\perp\f$.
   *
   * The cell cuts will be on absolute energy and absolute transverse
   * energy if this is set to true. If set to false the cuts will be
   * on energy and transverse energy instead.  */
  Gaudi::Property<bool> m_cellCutsInAbsE {this, "CellCutsInAbsE", true, "Cell (terminal) cuts in Abs E instead of E"};


  /**
   * @brief if set to true use 2-gaussian noise description for
   * TileCal
   *
   * @warning Currently unsupported on the GPU side!
   */
  Gaudi::Property<bool> m_twoGaussianNoise{this, "TwoGaussianNoise", false, "Use 2-gaussian noise description for TileCal"};


  /**
   * @brief Number of events for which to pre-allocate space on GPU memory
   * (should ideally be set to the expected number of threads to be run with).
   *
   */
  Gaudi::Property<size_t> m_numPreAllocatedGPUData {this, "NumPreAllocatedDataHolders", 0, "Number of temporary data holders to pre-allocate on GPU memory"};

  /** @brief A way to reduce allocations over multiple threads by keeping a cache
  *   of previously allocated objects that get assigned to the threads as they need them.
  *   It's all thread-safe due to an internal mutex ensuring no objects get assigned to different threads.
  */
  mutable CaloRecGPU::Helpers::separate_thread_holder<TACTemporariesHolder> m_temporariesHolder ATLAS_THREAD_SAFE;


  /** @brief Options for the algorithm, held in a GPU-friendly way.
  */
  TACOptionsHolder m_options;

};

#endif //CALORECGPU_TOPOAUTOMATONCLUSTERING_H
