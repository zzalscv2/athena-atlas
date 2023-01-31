#!/usr/bin/env python

# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
# @file:    trigDumpTimers.py
# @purpose: Script to dump the timing histograms from expert-monitoring.root/TIMERS
# @author:  Stewart Martin-Haugh

import ROOT
from optparse import OptionParser
import re
from TrigValTools.TrigRootUtils import lsroot

def timing(hist):
  mean = hist.GetMean()
  error = hist.GetMeanError()
  if hist.GetEntries() == 0:
    return "0"
  mean_plus_err = str(round(mean,3)) + " +/- " + str(round(error,3))
  overflow =  hist.GetBinContent(hist.GetNbinsX()+1)
  if overflow == 0.0:
    return mean_plus_err
  else:
    return mean_plus_err + "\tOverflow: " + str(overflow)

def get_matches(pattern, exclude, noSkip, myFile):
  names=lsroot(myFile)
  regex         = re.compile(".*" + pattern + ".*")
  if exclude:
    regex_exclude = re.compile(".*" + exclude + ".*")
  for name in names:
    if not noSkip:
      #Run-3 timers always contain TIME or TotalTime
      if "TIME" not in name:
        if "TotalTime" not in name:
          continue
    if not regex.match(name):
      continue
    if exclude:
      if regex_exclude.match(name):
        continue
    hist = myFile.Get(name)
    if not hist.IsA().InheritsFrom( "TH1" ):
      continue
    print(name + " " + timing(hist))

def main():
  parser = OptionParser()
  parser.add_option("-p", "--pattern", dest="pattern", type = "string", default = None,
                    help="Pattern to match histogram to")
  parser.add_option("-x", "--exclude", dest="exclude", type = "string", default = None,
                    help="Pattern to exclude histogram from matching")
  parser.add_option("-n", "--noSkip", dest="noSkip", action = "store_true",
                    help="Match all histograms (not just trigger timers)")
  (options, args) = parser.parse_args()        

  for arg in args:
    print(arg)
    myFile = ROOT.TFile(arg)
    pattern = ".*"
    if (options.pattern):
      pattern = options.pattern
    get_matches(pattern, options.exclude, options.noSkip, myFile)

    
if __name__ == "__main__":
  main()

