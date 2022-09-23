/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "BasicGPUToAthenaImporter.h"

#include "CaloRecGPU/CUDAFriendlyClasses.h"

#include "AthenaKernel/errorcheck.h"

#include <vector>
#include <algorithm>
#include <memory>

#include "xAODCaloEvent/CaloClusterKineHelper.h"
#include "CaloDetDescr/CaloDetDescrManager.h"

#include "boost/chrono/chrono.hpp"
#include "boost/chrono/thread_clock.hpp"

using namespace CaloRecGPU;

BasicGPUToAthenaImporter::BasicGPUToAthenaImporter(const std::string & type, const std::string & name, const IInterface * parent):
  AthAlgTool(type, name, parent),
  CaloGPUTimed(this)
{
  declareInterface<ICaloClusterGPUOutputTransformer> (this);

}

#include "MacroHelpers.h"

StatusCode BasicGPUToAthenaImporter::initialize()
{
  ATH_CHECK( m_cellsKey.value().initialize() );
  ATH_CHECK( detStore()->retrieve(m_calo_id, "CaloCell_ID") );

  auto get_option_from_string = [](const std::string & str, bool & failed)
  {
    failed = false;
    CRGPU_RECURSIVE_MACRO(
            CRGPU_CHEAP_STRING_TO_ENUM( str, xAOD::CaloCluster,
                                        SW_55ele,
                                        SW_35ele,
                                        SW_37ele,
                                        SW_55gam,
                                        SW_35gam,
                                        SW_37gam,
                                        SW_55Econv,
                                        SW_35Econv,
                                        SW_37Econv,
                                        SW_softe,
                                        Topo_420,
                                        Topo_633,
                                        SW_7_11,
                                        SuperCluster,
                                        Tower_01_01,
                                        Tower_005_005,
                                        Tower_fixed_area
                                      )
    )
    //I know Topological Clustering only supports a subset of those,
    //but this is supposed to be a general data exporting tool...
    else
      {
        //failed = true;
        return xAOD::CaloCluster::CSize_Unknown;
      }
  };

  bool size_failed = false;
  m_clusterSize = get_option_from_string(m_clusterSizeString, size_failed);

  if (m_clusterSize == xAOD::CaloCluster::CSize_Unknown)
    {
      ATH_MSG_ERROR("Invalid Cluster Size: " << m_clusterSizeString);
    }

  if (size_failed)
    {
      return StatusCode::FAILURE;
    }

  return StatusCode::SUCCESS;
}



StatusCode BasicGPUToAthenaImporter::convert (const EventContext & ctx,
                                              const ConstantDataHolder &,
                                              EventDataHolder & ed,
                                              xAOD::CaloClusterContainer * cluster_container) const
{


  using clock_type = boost::chrono::thread_clock;
  auto time_cast = [](const auto & before, const auto & after)
  {
    return boost::chrono::duration_cast<boost::chrono::microseconds>(after - before).count();
  };

  const auto start = clock_type::now();

  SG::ReadHandle<CaloCellContainer> cell_collection(m_cellsKey, ctx);
  if ( !cell_collection.isValid() )
    {
      ATH_MSG_ERROR( " Cannot retrieve CaloCellContainer: " << cell_collection.name()  );
      return StatusCode::RECOVERABLE;
    }
  const DataLink<CaloCellContainer> cell_collection_link (cell_collection.name(), ctx);

  ed.returnToCPU(!m_keepGPUData, true);

  const auto after_send = clock_type::now();

  std::vector<std::unique_ptr<CaloClusterCellLink>> cell_links;

  cell_links.reserve(ed.m_clusters->number);

  for (int i = 0; i < ed.m_clusters->number; ++i)
    {
      if (ed.m_clusters->seedCellID[i] >= 0)
        {
          cell_links.emplace_back(std::make_unique<CaloClusterCellLink>(cell_collection_link));
          cell_links.back()->reserve(256);
          //To be adjusted.
        }
      else
        {
          cell_links.emplace_back(nullptr);
          //The excluded clusters don't have any cells.
        }
    }

  const auto after_creation = clock_type::now();

  CaloCellContainer::const_iterator iCells = cell_collection->begin();

  for (int cell_count = 0; iCells != cell_collection->end(); ++iCells, ++cell_count)
    {
      const CaloCell * cell = (*iCells);

      const int index = m_calo_id->calo_cell_hash(cell->ID());

      const tag_type this_tag = ed.m_cell_state->clusterTag[index];

      if (Tags::is_part_of_cluster(this_tag))
        {
          const int this_index = Tags::get_index_from_tag(this_tag);
          cell_links[this_index]->addCell(cell_count, 1.);
          //So we put this in the right cell link.
        }
    }

  const auto after_cells = clock_type::now();

  std::vector<int> cluster_order(ed.m_clusters->number);

  std::iota(cluster_order.begin(), cluster_order.end(), 0);

  std::sort(cluster_order.begin(), cluster_order.end(), [&](const int a, const int b)
  {
    return ed.m_clusters->clusterEt[a] > ed.m_clusters->clusterEt[b];
  } );

  //Ordered by Et as in the default algorithm...
  //The fact that some invalid clusters
  //(with possibly trash values for Et)
  //can crop up is irrelevant since
  //we don't add those anyway:
  //the rest is still ordered like we want it to be.

  const auto after_sort = clock_type::now();

  cluster_container->clear();
  cluster_container->reserve(cell_links.size());

  for (size_t i = 0; i < cluster_order.size(); ++i)
    {
      const int cluster_index = cluster_order[i];

      const float cluster_Et = ed.m_clusters->clusterEt[cluster_index];

      if ( cell_links[cluster_index] != nullptr && cell_links[cluster_index]->size() > 0 &&
           (m_cutClustersInAbsE ? std::abs(cluster_Et) : cluster_Et) > m_clusterETThreshold )
        {
          xAOD::CaloCluster * cluster = new xAOD::CaloCluster();
          cluster_container->push_back(cluster);

          cluster->addCellLink(cell_links[cluster_index].release());
          cluster->setClusterSize(m_clusterSize);
          if (m_useCPUPropertiesCalculation)
            {
              CaloClusterKineHelper::calculateKine(cluster, false, true);
            }
          else
            {
              cluster->setE(ed.m_clusters->clusterEnergy[cluster_index]);
              cluster->setEta(ed.m_clusters->clusterEta[cluster_index]);
              cluster->setPhi(ed.m_clusters->clusterPhi[cluster_index]);
            }
        }

    }

  const auto after_fill = clock_type::now();

  if (m_measureTimes)
    {
      record_times(ctx.evt(), time_cast(start, after_send),
                   time_cast(after_send, after_creation),
                   time_cast(after_creation, after_cells),
                   time_cast(after_cells, after_sort),
                   time_cast(after_sort, after_fill)
                  );
    }

  return StatusCode::SUCCESS;

}


StatusCode BasicGPUToAthenaImporter::finalize()
{

  if (m_measureTimes)
    {
      print_times("Transfer_from_GPU Cluster_Creation Cell_Adding Sorting Collection_Filling", 5);
    }
  return StatusCode::SUCCESS;
}


BasicGPUToAthenaImporter::~BasicGPUToAthenaImporter()
{
  //Nothing!
}
