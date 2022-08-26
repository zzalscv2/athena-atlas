// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
//

#define TTAC_CALCULATE_PHI_BY_SIMPLE_AVERAGE 1

#include "CaloRecGPU/Helpers.h"
#include "TestDefinitions.h"

#include "DefaultImplementation.h"
#include "HalvedPairs.h"
#include "FixedPairs.h"

using namespace CaloRecGPU;

int main(int argc, char ** argv)
{
  std::string folder = ".";
  int max_events = 32,
      num_reps = test_function::s_num_reps,
      compress_loops_after = test_function::s_compress_loops_after,
      keep_best = test_function::s_keep_best;
  std::string prefix = test_function::s_prefix;

  if (argc > 1)
    {
      folder = argv[1];
    }
  if (argc > 2)
    {
      max_events = std::atoi(argv[2]);
    }
  if (argc > 3)
    {
      prefix = argv[3];
    }
  if (argc > 4)
    {
      num_reps = std::atoi(argv[4]);
    }
  if (argc > 5)
    {
      compress_loops_after = std::atoi(argv[5]);
    }
  if (argc > 6)
    {
      keep_best = std::atoi(argv[6]);
    }

  TestHolder tests(folder, max_events);

  test_function::s_num_reps = num_reps;
  test_function::s_compress_loops_after = compress_loops_after;
  test_function::s_keep_best = keep_best;
  test_function::s_prefix = prefix;

  loop_range threads{1, 128, 1};
  //loop_range blocks{256, 256, 256};
  loop_range blocks{32, 1024, 32};

  tests.add_snr_func("default_snr", DefaultImplementation::signal_to_noise, threads, blocks);
  tests.add_pair_func("default_pairs", DefaultImplementation::cell_pairs, threads, blocks);
  //tests.add_pair_func("half_pairs", HalvedPairs::cell_pairs, threads, blocks);
  tests.add_grow_func("default_growing", DefaultImplementation::cluster_growing, threads, blocks, blocks, blocks, blocks);
  //tests.add_grow_func("half_pair_growing", HalvedPairs::cluster_growing, threads, blocks, blocks, blocks);
  //tests.add_grow_func("fixed_pair_growing", FixedPairs::cluster_growing, threads, blocks, blocks, blocks);
  tests.add_finalize_func("default_finalize", DefaultImplementation::finalize_clusters, threads, blocks, blocks);

  tests.do_tests(false);
  //Change this to `true` to recover the old, "pretty" file format.

  return 0;
}
