/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloGPUHybridClusterProcessor.h"

#include "AthenaKernel/errorcheck.h"

#include <algorithm>

#include "xAODCaloEvent/CaloClusterContainer.h"
#include "CaloEvent/CaloClusterCellLinkContainer.h"
#include "CaloUtils/CaloClusterStoreHelper.h"

#include "boost/chrono/chrono.hpp"
#include "boost/chrono/thread_clock.hpp"

using namespace CaloRecGPU;

CaloGPUHybridClusterProcessor::CaloGPUHybridClusterProcessor(const std::string & name, ISvcLocator * pSvcLocator):
  AthReentrantAlgorithm(name, pSvcLocator),
  CaloGPUTimed(this),
  m_transformConstantData(this),
  m_preGPUoperations(this),
  m_transformForGPU(this),
  m_GPUoperations(this),
  m_transformBackToCPU(this),
  m_postGPUoperations(this),
  m_clusterOutput(""),
  m_clusterCellLinkOutput(""),
  m_constantDataSent(false)
{
  //Tool for transforming the constant data and sending it to the GPU (before the run starts)
  //(a CaloClusterGPUConstantTransformer tool)
  declareProperty("ConstantDataToGPUTool", m_transformConstantData, "Tool for transforming the constant data and sending it to the GPU");

  //Operations to be done on CPU before processing stuff on the GPU
  //(array of CaloClusterCollectionProcessor tools)
  declareProperty("BeforeGPUTools", m_preGPUoperations, "Tools to be applied to the clusters on the CPU before processing them on the GPU");

  //Tool for transforming the event data and sending it to the GPU
  //(a CaloClusterGPUInputTransformer tool)
  declareProperty("EventDataToGPUTool", m_transformForGPU, "Tool for transforming the event data and sending it to the GPU");

  //Operations to be done on the GPU
  //(array of CaloClusterGPUProcessor tools)
  declareProperty("GPUTools", m_GPUoperations, "Tools to be applied to the clusters on the GPU");

  //Tool for getting the data from the GPU back to the CPU
  //(a CaloClusterGPUOutputTransformer tool)
  declareProperty("GPUToEventDataTool", m_transformBackToCPU, "Tool for getting the data from the GPU back to the CPU Athena datastructures");

  //Operations to be done on CPU after having processed stuff on the GPU
  //(array of CaloClusterCollectionProcessor tools)
  declareProperty("AfterGPUTools", m_postGPUoperations, "Tools to be applied to the clusters on the CPU after returning from the GPU");

  // Name of Cluster Container to be registered in TDS
  declareProperty("ClustersOutputName", m_clusterOutput, "The name of the key in StoreGate for the output CaloClusterContainer");
  declareProperty("ClusterCellLinkOutputName", m_clusterCellLinkOutput, "The name of the key in StoreGate for the output CaloClusterCellLinkContainer");
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

  auto retrieve_and_report = [&](auto & var, const auto & type)
  {
    if (var.retrieve().isFailure())
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
  
  retrieve_and_report(m_transformConstantData, "constant data to GPU transformer");
  retrieve_and_report(m_preGPUoperations, "pre-GPU operations");
  retrieve_and_report(m_transformForGPU, "event data to GPU transformer");
  retrieve_and_report(m_GPUoperations, "GPU operations");
  retrieve_and_report(m_transformBackToCPU, "GPU to Athena transformer");
  retrieve_and_report(m_postGPUoperations, "post-GPU operations");

  if (any_failed)
    {
      return StatusCode::FAILURE;
    }

  if (!m_deferConstantDataToFirstEvent)
    {
      ATH_CHECK( m_transformConstantData->initialize() );
      //Not sure if this is needed or the tool will get initialized by this point.

      ATH_CHECK( m_transformConstantData->convert(m_constantData) );
      m_constantDataSent = true;
    }


  if (size_t(m_numPreAllocatedGPUData) > 0)
    {
      ATH_MSG_DEBUG("Pre-allocating event data for " << size_t(m_numPreAllocatedGPUData) << " events.");

      m_eventDataThreadedHolder.resize(m_numPreAllocatedGPUData);
      //This will allocate the object holders.

      m_eventDataThreadedHolder.operate_on_all( [&](EventDataHolder & edh)
      {
        edh.allocate(true);
      }
                                              );
      //This will allocate all the memory at this point.
      //Also useful to prevent/debug potential allocation issues?
      //But the main point is really reducing the execute times...
    }

  return StatusCode::SUCCESS;
}


StatusCode CaloGPUHybridClusterProcessor::execute(const EventContext & ctx, xAOD::CaloClusterContainer * cluster_collection_ptr) const
{
  if (m_deferConstantDataToFirstEvent && !m_constantDataSent)
    {
      std::lock_guard<std::mutex> lock_guard(m_mutex);
      if (!m_constantDataSent)
        {
          ConstantDataHolder * cdh_ptr ATLAS_THREAD_SAFE = &m_constantData;
          ATH_CHECK( m_transformConstantData->convert(ctx, *cdh_ptr) );
          m_constantDataSent = true;
        }
    }

  EventDataHolder * event_data_ptr = nullptr;

  Helpers::separate_thread_accessor<EventDataHolder> sep_th_acc(m_eventDataThreadedHolder, event_data_ptr);
  //This is a RAII wrapper to access an object held by Helpers::separate_thread_holder,
  //to ensure the event data is appropriately released when we are done processing.

  if (event_data_ptr == nullptr)
    {
      ATH_MSG_ERROR("Could not get valid Event Data Holder! Event: " << ctx.evt() );
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

  if (m_measureTimes)
    {
      const size_t time_size = m_preGPUoperations.size() + m_GPUoperations.size() + m_postGPUoperations.size() + 2;
      times.reserve(time_size);
    }

  for (auto & pre_GPU_tool : m_preGPUoperations)
    {
      auto t1 = clock_type::now();
      ATH_CHECK( pre_GPU_tool->execute(ctx, cluster_collection_ptr) );
      auto t2 = clock_type::now();
      if (m_measureTimes)
        {
          times.push_back(time_cast(t1, t2));
        }
    }
    
  auto t3 = clock_type::now();
  ATH_CHECK( m_transformForGPU->convert(ctx, constant_data_holder, cluster_collection_ptr, *event_data_ptr) );
  auto t4 = clock_type::now();
  if (m_measureTimes)
    {
      times.push_back(time_cast(t3, t4));
    }

  for (auto & GPU_tool : m_GPUoperations)
    {
      auto t5 = clock_type::now();
      ATH_CHECK( GPU_tool->execute(ctx, constant_data_holder, *event_data_ptr) );
      auto t6 = clock_type::now();
      if (m_measureTimes)
        {
          times.push_back(time_cast(t5, t6));
        }
    }

  auto t7 = clock_type::now();
  ATH_CHECK( m_transformBackToCPU->convert(ctx, constant_data_holder, *event_data_ptr, cluster_collection_ptr) );
  auto t8 = clock_type::now();
  if (m_measureTimes)
    {
      times.push_back(time_cast(t7, t8));
    }


  for (auto & post_GPU_tool : m_postGPUoperations)
    {
      auto t9 = clock_type::now();
      ATH_CHECK( post_GPU_tool->execute(ctx, cluster_collection_ptr) );
      auto t10 = clock_type::now();
      if (m_measureTimes)
        {
          times.push_back(time_cast(t9, t10));
        }
    }

  if (m_measureTimes)
    {
      record_times(ctx.evt(), times);
    }

  return StatusCode::SUCCESS;
}

StatusCode CaloGPUHybridClusterProcessor::execute(const EventContext & ctx) const
{
  SG::WriteHandle<xAOD::CaloClusterContainer> cluster_collection (m_clusterOutput, ctx);

  ATH_CHECK(CaloClusterStoreHelper::AddContainerWriteHandle(cluster_collection));
  //ATH_CHECK(CaloClusterStoreHelper::AddContainerWriteHandle(&(*evtStore()), cluster_collection, msg()));

  const StatusCode exec_return = this->execute(ctx, cluster_collection.ptr());

  if (exec_return != StatusCode::SUCCESS)
    {
      return exec_return;
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

      for (auto & pre_GPU_tool : m_preGPUoperations)
        {
          add_name_to_string(pre_GPU_tool);
        }

      add_name_to_string(m_transformForGPU);

      for (auto & GPU_tool : m_GPUoperations)
        {
          add_name_to_string(GPU_tool);
        }

      add_name_to_string(m_transformBackToCPU);

      for (auto & post_GPU_tool : m_postGPUoperations)
        {
          add_name_to_string(post_GPU_tool);
        }

      print_times(header_string, m_preGPUoperations.size() + m_GPUoperations.size() + m_postGPUoperations.size() + 2);
    }
  return StatusCode::SUCCESS;
}