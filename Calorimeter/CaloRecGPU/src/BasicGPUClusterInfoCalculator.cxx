/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "BasicGPUClusterInfoCalculator.h"
#include "BasicGPUClusterInfoCalculatorImpl.h"

#include "boost/chrono/chrono.hpp"
#include "boost/chrono/thread_clock.hpp"

using namespace CaloRecGPU;

BasicGPUClusterInfoCalculator::BasicGPUClusterInfoCalculator(const std::string & type, const std::string & name, const IInterface * parent):
  AthAlgTool(type, name, parent),
  CaloGPUTimed(this)
{
  declareInterface<CaloClusterGPUProcessor> (this);
}

StatusCode BasicGPUClusterInfoCalculator::initialize()
{

  if (m_numPreAllocatedGPUData > 0)
    {
      ATH_MSG_DEBUG("Pre-allocating temporaries for " << m_numPreAllocatedGPUData << " events.");

      m_temporariesHolder.resize(m_numPreAllocatedGPUData);
      //This will allocate the object holders.

      m_temporariesHolder.operate_on_all( [&](BasicGPUClusterInfoCalculatorTemporariesHolder & tth)
      {
        tth.allocate();
      }
                                        );
      //This will allocate all the memory at this point.
      //Also useful to prevent/debug potential allocation issues?
      //But the main point is really reducing the execute times...
    }

  return StatusCode::SUCCESS;
}

StatusCode BasicGPUClusterInfoCalculator::execute(const EventContext & ctx, const ConstantDataHolder & constant_data, EventDataHolder & event_data) const
{

  using clock_type = boost::chrono::thread_clock;
  auto time_cast = [](const auto & before, const auto & after)
  {
    return boost::chrono::duration_cast<boost::chrono::microseconds>(after - before).count();
  };

  const auto start = clock_type::now();

  BasicGPUClusterInfoCalculatorTemporariesHolder * temp_data_ptr = nullptr;

  Helpers::separate_thread_accessor<BasicGPUClusterInfoCalculatorTemporariesHolder> sep_th_acc(m_temporariesHolder, temp_data_ptr);
  //This is a RAII wrapper to access an object held by Helpers::separate_thread_holder,
  //to ensure the event data is appropriately released when we are done processing.

  temp_data_ptr->allocate();
  //Does nothing if it is already allocated.

  const auto before_seed_properties = clock_type::now();

  updateSeedCellProperties(event_data, *temp_data_ptr, constant_data, m_measureTimes);

  const auto before_calculating = clock_type::now();

  calculateClusterProperties(event_data, *temp_data_ptr, constant_data, m_measureTimes);

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