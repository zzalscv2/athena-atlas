//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#include "CaloGPUHybridClusterProcessor.h"

#include "AthenaKernel/errorcheck.h"

#include "AthenaMonitoringKernel/Monitored.h"

#include <algorithm>

//We'll use a quick and dirty helper with placement new...
#include <new>

#include "xAODCaloEvent/CaloClusterContainer.h"
#include "CaloEvent/CaloClusterCellLinkContainer.h"
#include "CaloUtils/CaloClusterStoreHelper.h"

#include "boost/chrono/chrono.hpp"
#include "boost/chrono/thread_clock.hpp"

using namespace CaloRecGPU;

CaloGPUHybridClusterProcessor::CaloGPUHybridClusterProcessor(const std::string & name, ISvcLocator * pSvcLocator):
  AthReentrantAlgorithm(name, pSvcLocator),
  CaloGPUTimed(this),
  m_temporariesSize(0),
  m_constantDataSent(false),
  m_preConvert(true),
  m_postConvert(true)
{

}

CaloGPUHybridClusterProcessor::~CaloGPUHybridClusterProcessor()
{
  //Nothing!
}


StatusCode CaloGPUHybridClusterProcessor::initialize()
{

  ATH_CHECK( m_clusterOutput.initialize() );

  if (m_clusterCellLinkOutput.key().empty())
    {
      m_clusterCellLinkOutput = m_clusterOutput.key() + "_links";
    }
  ATH_CHECK( m_clusterCellLinkOutput.initialize() );


  bool any_failed = false;

  auto retrieve_and_report = [&](auto & var, const auto & type, bool & falsify_if_empty)
  {
    if (var.empty())
      {
        falsify_if_empty = false;
        ATH_MSG_DEBUG("There is nothing to retrieve for " << type << ".");
      }
    else if (var.retrieve().isFailure())
      {
        ATH_MSG_ERROR("Failed to retrieve " << type << ": " << var);
        any_failed = true;
      }
    else
      {
        ATH_MSG_DEBUG("Successfully retrieved " << type << ": " << var);
      }
  };
  //A generic lambda to prevent code repetition.


  bool checker = true;
  retrieve_and_report(m_transformConstantData, "constant data to GPU transformer", checker);
  if (!checker)
    {
      ATH_MSG_ERROR("No constant data transformer, error!");
      any_failed = true;
    }
  retrieve_and_report(m_preGPUoperations, "pre-GPU operations", checker);
  retrieve_and_report(m_transformForGPU, "event data to GPU transformer", m_preConvert);
  retrieve_and_report(m_GPUoperations, "GPU operations", checker);
  retrieve_and_report(m_transformBackToCPU, "GPU to Athena transformer", m_postConvert);
  retrieve_and_report(m_postGPUoperations, "post-GPU operations", checker);

  if (m_doPlots)
    {
      retrieve_and_report(m_plotterTool, "plotter tool", checker);
    }

  if (m_doMonitoring)
    {
      retrieve_and_report(m_moniTool, "monitoring tool", checker);
    }

  if (any_failed)
    {
      return StatusCode::FAILURE;
    }

  ATH_CHECK( m_avgMuKey.initialize() );

  if (!m_deferConstantDataToFirstEvent)
    {
      ATH_CHECK( m_transformConstantData->initialize() );
      //Not sure if this is needed or the tool will get initialized by this point.

      ATH_CHECK( m_transformConstantData->convert(m_constantData, m_doPlots) );
      m_constantDataSent = true;
    }

  m_temporariesSize = 0;

  for (const auto & tool : m_GPUoperations)
    {
      m_temporariesSize = std::max(m_temporariesSize, tool->size_of_temporaries());
    }

  if (size_t(m_numPreAllocatedGPUData) > 0)
    {
      ATH_MSG_INFO("Pre-allocating event data and temporary buffer for " << size_t(m_numPreAllocatedGPUData) << " parellel events.");

      m_eventDataThreadedHolder.resize(m_numPreAllocatedGPUData);
      m_temporariesThreadedHolder.resize(m_numPreAllocatedGPUData);
      //This will allocate the object holders.

      m_eventDataThreadedHolder.operate_on_all( [&](EventDataHolder & edh)
      {
        edh.allocate(true);
      }
                                              );
      m_temporariesThreadedHolder.operate_on_all( [&](simple_GPU_pointer_holder & ph)
      {
        ph.allocate(m_temporariesSize);
      }
                                                );
      //This will allocate all the memory at this point.
      //Also useful to prevent/debug potential allocation issues?
      //But the main point is really reducing the execute times...
    }

  return StatusCode::SUCCESS;
}


StatusCode CaloGPUHybridClusterProcessor::execute(const EventContext & ctx) const
{
  SG::WriteHandle<xAOD::CaloClusterContainer> cluster_collection (m_clusterOutput, ctx);

  ATH_CHECK(CaloClusterStoreHelper::AddContainerWriteHandle(cluster_collection));
  //ATH_CHECK(CaloClusterStoreHelper::AddContainerWriteHandle(&(*evtStore()), cluster_collection, msg()));

  xAOD::CaloClusterContainer * cluster_collection_ptr = cluster_collection.ptr();

  if (m_deferConstantDataToFirstEvent && !m_constantDataSent.load())
    {
      std::lock_guard<std::mutex> lock_guard(m_mutex);
      if (!m_constantDataSent.load())
        {
          ConstantDataHolder * cdh_ptr ATLAS_THREAD_SAFE = &m_constantData;
          ATH_CHECK( m_transformConstantData->convert(ctx, *cdh_ptr, m_doPlots) );
          m_constantDataSent.store(true);
        }
    }

  EventDataHolder * event_data_ptr = nullptr;

  Helpers::separate_thread_accessor<EventDataHolder> sep_th_acc_1(m_eventDataThreadedHolder, event_data_ptr);
  //This is a RAII wrapper to access an object held by Helpers::separate_thread_holder,
  //to ensure the event data is appropriately released when we are done processing.

  if (event_data_ptr == nullptr && (m_preConvert || m_postConvert || m_GPUoperations.size()))
    {
      ATH_MSG_ERROR("Could not get valid Event Data Holder! Event: " << ctx.evt() );
      return StatusCode::FAILURE;
    }


  simple_GPU_pointer_holder * temporaries_data_ptr_holder = nullptr;

  Helpers::separate_thread_accessor<simple_GPU_pointer_holder> sep_th_acc_2(m_temporariesThreadedHolder, temporaries_data_ptr_holder);
  if (!temporaries_data_ptr_holder)
  {
    ATH_MSG_ERROR("temporaries_data_ptr_holder is null in CaloGPUHybridClusterProcessor::execute" );
    return StatusCode::FAILURE;
  }
  temporaries_data_ptr_holder->allocate(m_temporariesSize);
  //This will not perform any allocations if they've already been done.

  if ( (temporaries_data_ptr_holder->get_pointer() == nullptr)   &&
       (m_preConvert || m_postConvert || m_GPUoperations.size()) &&
       m_temporariesSize > 0                                         )
    {
      ATH_MSG_ERROR("Could not get valid temporary buffer holder! Event: " << ctx.evt() );
      return StatusCode::FAILURE;
    }

  const ConstantDataHolder & constant_data_holder ATLAS_THREAD_SAFE = m_constantData;
  //Just to shut up the checker. We know what we are doing...

  using clock_type = boost::chrono::thread_clock;
  auto time_cast = [](const auto & before, const auto & after)
  {
    return boost::chrono::duration_cast<boost::chrono::microseconds>(after - before).count();
  };

  std::vector<size_t> times;

  size_t plot_time = 0;

  if (m_measureTimes)
    {
      const size_t time_size = m_preGPUoperations.size() + m_GPUoperations.size() + m_postGPUoperations.size() + m_doPlots + m_preConvert + m_postConvert;
      //+2 for the conversions
      //+1 for the plotter (only added at the end)
      times.reserve(time_size);
    }

  if (m_doPlots)
    {
      auto t1 = clock_type::now();
      ATH_CHECK( m_plotterTool->update_plots_start(ctx, constant_data_holder, cluster_collection_ptr) );
      auto t2 = clock_type::now();
      if (m_measureTimes)
        {
          plot_time += time_cast(t1, t2);
        }
    }


  for (const auto & pre_GPU_tool : m_preGPUoperations)
    {
      auto t1 = clock_type::now();
      ATH_CHECK( pre_GPU_tool->execute(ctx, cluster_collection_ptr) );
      auto t2 = clock_type::now();
      if (m_measureTimes)
        {
          times.push_back(time_cast(t1, t2));
        }
      if (m_doPlots)
        {
          auto t3 = clock_type::now();
          ATH_CHECK( m_plotterTool->update_plots(ctx, constant_data_holder, cluster_collection_ptr, pre_GPU_tool.get()) );
          auto t4 = clock_type::now();
          if (m_measureTimes)
            {
              plot_time += time_cast(t3, t4);
            }
        }
    }

  if (m_preConvert)
    {
      auto t3 = clock_type::now();
      ATH_CHECK( m_transformForGPU->convert(ctx, constant_data_holder, cluster_collection_ptr, *event_data_ptr) );
      auto t4 = clock_type::now();
      if (m_measureTimes)
        {
          times.push_back(time_cast(t3, t4));
        }
    }

  if (m_doPlots)
    {
      auto t1 = clock_type::now();
      ATH_CHECK( m_plotterTool->update_plots(ctx, constant_data_holder, cluster_collection_ptr, *event_data_ptr, m_transformForGPU.get()) );
      auto t2 = clock_type::now();
      if (m_measureTimes)
        {
          plot_time += time_cast(t1, t2);
        }
    }

  for (const auto & GPU_tool : m_GPUoperations)
    {
      auto t5 = clock_type::now();
      ATH_CHECK( GPU_tool->execute(ctx, constant_data_holder, *event_data_ptr, temporaries_data_ptr_holder->get_pointer()) );
      auto t6 = clock_type::now();
      if (m_measureTimes)
        {
          times.push_back(time_cast(t5, t6));
        }
      if (m_doPlots)
        {
          auto t3 = clock_type::now();
          ATH_CHECK( m_plotterTool->update_plots(ctx, constant_data_holder, cluster_collection_ptr, *event_data_ptr, GPU_tool.get()) );
          auto t4 = clock_type::now();
          if (m_measureTimes)
            {
              plot_time += time_cast(t3, t4);
            }
        }
    }

  if (m_postConvert)
    {
      auto t7 = clock_type::now();
      ATH_CHECK( m_transformBackToCPU->convert(ctx, constant_data_holder, *event_data_ptr, cluster_collection_ptr) );
      auto t8 = clock_type::now();
      if (m_measureTimes)
        {
          times.push_back(time_cast(t7, t8));
        }
    }

  if (m_doPlots)
    {
      auto t1 = clock_type::now();
      ATH_CHECK( m_plotterTool->update_plots(ctx, constant_data_holder, cluster_collection_ptr, *event_data_ptr, m_transformBackToCPU.get()) );
      auto t2 = clock_type::now();
      if (m_measureTimes)
        {
          plot_time += time_cast(t1, t2);
        }
    }

  for (const auto & post_GPU_tool : m_postGPUoperations)
    {
      auto t9 = clock_type::now();
      ATH_CHECK( post_GPU_tool->execute(ctx, cluster_collection_ptr) );
      auto t10 = clock_type::now();
      if (m_measureTimes)
        {
          times.push_back(time_cast(t9, t10));
        }
      if (m_doPlots)
        {
          auto t3 = clock_type::now();
          ATH_CHECK( m_plotterTool->update_plots(ctx, constant_data_holder, cluster_collection_ptr, post_GPU_tool.get()) );
          auto t4 = clock_type::now();
          if (m_measureTimes)
            {
              plot_time += time_cast(t3, t4);
            }
        }
    }

  if (m_doPlots)
    {
      auto t1 = clock_type::now();
      ATH_CHECK( m_plotterTool->update_plots_end(ctx, constant_data_holder, cluster_collection_ptr) );
      auto t2 = clock_type::now();
      if (m_measureTimes)
        {
          plot_time += time_cast(t1, t2);
        }
    }



  if (m_doMonitoring)
    //For monitoring.
    //Taken from TrigCaloClusterMaker from TrigCaloRec.
    {
      auto mon_container_size = Monitored::Scalar( "container_size", 0. );
      auto mon_clusEt = Monitored::Collection( "Et", *cluster_collection_ptr, &xAOD::CaloCluster::et );
      auto mon_clusSignalState = Monitored::Collection( "signalState", *cluster_collection_ptr, &xAOD::CaloCluster::signalState );
      auto mon_clusSize = Monitored::Collection( "clusterSize", *cluster_collection_ptr, &xAOD::CaloCluster::clusterSize );
      std::vector<double>       clus_phi;
      std::vector<double>       clus_eta;
      std::vector<double>       N_BAD_CELLS;
      std::vector<double>       ENG_FRAC_MAX;
      std::vector<unsigned int> sizeVec;
      auto mon_clusPhi = Monitored::Collection( "Phi", clus_phi );
      auto mon_clusEta = Monitored::Collection( "Eta", clus_eta );
      auto mon_badCells = Monitored::Collection( "N_BAD_CELLS", N_BAD_CELLS );
      auto mon_engFrac = Monitored::Collection( "ENG_FRAC_MAX", N_BAD_CELLS );
      auto mon_size = Monitored::Collection( "size", sizeVec );
      auto monmu = Monitored::Scalar( "mu", -999.0 );
      auto monitorIt = Monitored::Group( m_moniTool, mon_container_size, mon_clusEt,
                                         mon_clusPhi, mon_clusEta, mon_clusSignalState, mon_clusSize,
                                         mon_badCells, mon_engFrac, mon_size, monmu );
      // fill monitored variables
      for (xAOD::CaloCluster * cl : *cluster_collection_ptr)
        {
          const CaloClusterCellLink * num_cell_links = cl->getCellLinks();
          if (! num_cell_links)
            {
              sizeVec.push_back(0);
            }
          else
            {
              sizeVec.push_back(num_cell_links->size());
            }
          clus_phi.push_back(cl->phi());
          clus_eta.push_back(cl->eta());
          N_BAD_CELLS.push_back(cl->getMomentValue(xAOD::CaloCluster::N_BAD_CELLS));
          ENG_FRAC_MAX.push_back(cl->getMomentValue(xAOD::CaloCluster::ENG_FRAC_MAX));
        }
      SG::ReadDecorHandle<xAOD::EventInfo, float> eventInfoDecor(m_avgMuKey, ctx);
      if (eventInfoDecor.isPresent())
        {
          monmu = eventInfoDecor(0);
        }
      mon_container_size = cluster_collection_ptr->size();
    }

  if (m_measureTimes)
    {
      if (m_doPlots)
        {
          times.push_back(plot_time);
        }
      record_times(ctx.evt(), times);
    }

  ATH_MSG_DEBUG("Created cluster container with " << cluster_collection->size() << " clusters");

  SG::WriteHandle<CaloClusterCellLinkContainer> cell_links(m_clusterCellLinkOutput, ctx);

  ATH_CHECK( CaloClusterStoreHelper::finalizeClusters(cell_links, cluster_collection.ptr()) );

  return StatusCode::SUCCESS;
}

StatusCode CaloGPUHybridClusterProcessor::finalize()
{
  if (m_measureTimes)
    {
      std::string header_string;

      auto add_name_to_string = [&](const auto & obj)
      {
        std::string rep = obj->name();
        std::replace(rep.begin(), rep.end(), ' ', '_');
        header_string += rep + " ";
      };

      for (const auto & pre_GPU_tool : m_preGPUoperations)
        {
          add_name_to_string(pre_GPU_tool);
        }

      if (m_preConvert)
        {
          add_name_to_string(m_transformForGPU);
        }

      for (const auto & GPU_tool : m_GPUoperations)
        {
          add_name_to_string(GPU_tool);
        }

      if (m_postConvert)
        {
          add_name_to_string(m_transformBackToCPU);
        }

      for (const auto & post_GPU_tool : m_postGPUoperations)
        {
          add_name_to_string(post_GPU_tool);
        }

      if (m_doPlots)
        {
          add_name_to_string(m_plotterTool);
        }

      print_times(header_string, m_preGPUoperations.size() + m_GPUoperations.size() + m_postGPUoperations.size() + m_preConvert + m_postConvert + m_doPlots);
    }

  if (m_doPlots)
    {
      ATH_CHECK(m_plotterTool->finalize_plots());
    }

  return StatusCode::SUCCESS;
}
