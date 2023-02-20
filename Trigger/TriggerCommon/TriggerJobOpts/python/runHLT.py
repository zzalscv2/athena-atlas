# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration


def lock_and_restrict(flags):
   """Deny access to a few flags and lock"""

   def bomb(x):
      raise RuntimeError("Concurrency flags cannot be used in the HLT to ensure "
                         "that the configuration is portable across different CPUs")

   flags.Concurrency.NumProcs = bomb
   flags.Concurrency.NumThreads = bomb
   flags.Concurrency.NumConcurrentEvents = bomb
   flags.lock()
