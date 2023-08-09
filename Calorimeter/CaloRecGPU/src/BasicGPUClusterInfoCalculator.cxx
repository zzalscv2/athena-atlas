//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#include "BasicGPUClusterInfoCalculator.h"
#include "BasicGPUClusterInfoCalculatorImpl.h"

#include "boost/chrono/chrono.hpp"
#include "boost/chrono/thread_clock.hpp"

using namespace CaloRecGPU;
using namespace BasicClusterInfoCalculator;

BasicGPUClusterInfoCalculator::BasicGPUClusterInfoCalculator(const std::string & type, const std::string & name, const IInterface * parent):
  AthAlgTool(type, name, parent),
  CaloGPUTimed(this)
{
  declareInterface<CaloClusterGPUProcessor> (this);
}

StatusCode BasicGPUClusterInfoCalculator::initialize()
{ 
  ATH_CHECK( m_kernelSizeOptimizer.retrieve() );
  register_kernels( *(m_kernelSizeOptimizer.get()) );
  return StatusCode::SUCCESS;
}

StatusCode BasicGPUClusterInfoCalculator::execute(const EventContext & ctx, const ConstantDataHolder & constant_data,
                                                  EventDataHolder & event_data, void * temporary_buffer) const
{
  using clock_type = boost::chrono::thread_clock;
  auto time_cast = [](const auto & before, const auto & after)
  {
    return boost::chrono::duration_cast<boost::chrono::microseconds>(after - before).count();
  };

  const auto start = clock_type::now();

  void * temp_store = (m_preserveClusterMoments ? temporary_buffer : (ClusterMomentsArr *) event_data.m_moments_dev);

  Helpers::CUDA_kernel_object<ClusterInfoCalculatorTemporaries> temporaries((ClusterInfoCalculatorTemporaries *) temp_store);

  const auto before_seed_properties = clock_type::now();

  updateSeedCellProperties(event_data, temporaries, constant_data, *(m_kernelSizeOptimizer.get()), m_measureTimes);

  const auto before_calculating = clock_type::now();

  calculateClusterProperties(event_data, temporaries, constant_data, *(m_kernelSizeOptimizer.get()), m_measureTimes, m_cutClustersInAbsE, m_clusterETThreshold);

  const auto end = clock_type::now();


  if (m_measureTimes)
    {
      record_times(ctx.evt(),
                   time_cast(start, before_seed_properties),
                   time_cast(before_seed_properties, before_calculating),
                   time_cast(before_calculating, end)
                  );
    }


  return StatusCode::SUCCESS;

}


StatusCode BasicGPUClusterInfoCalculator::finalize()
{
  if (m_measureTimes)
    {
      print_times("Preprocessing Seed_Cell_Properties_Updating Cluster_Properties_Calculation", 3);
    }
  return StatusCode::SUCCESS;
}


BasicGPUClusterInfoCalculator::~BasicGPUClusterInfoCalculator()
{
  //Nothing!
}
