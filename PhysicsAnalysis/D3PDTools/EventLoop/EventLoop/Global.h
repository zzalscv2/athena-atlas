/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef EVENT_LOOP_GLOBAL_HH
#define EVENT_LOOP_GLOBAL_HH

namespace EL
{
  class Algorithm;
  class AnaAlgorithmWrapper;
  class BatchDriver;
  struct BatchJob;
  struct BatchSample;
  struct BatchSegment;
  class CondorDriver;
  class DirectDriver;
  class Driver;
  struct EventRange;
  class GEDriver;
  class Job;
  class JobConfig;
  class LLDriver;
  class LSFDriver;
  class LocalDriver;
  class OutputStream;
  class SlurmDriver;
  class StatusCode;
  class TorqueDriver;
  class IWorker;
  class Worker;

  namespace Detail
  {
    struct AlgorithmData;
    struct JobSubmitInfo;
    enum class JobSubmitStep;
    class Module;
    struct ModuleData;
    class OutputStreamData;

    class AlgorithmStateModule;
  }
}

#endif
