/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <iostream>
#include <algorithm>
#include "FPGATrackSimObjects/FPGATrackSimMultiTruth.h"

ClassImp(FPGATrackSimMultiTruth)

// FPGATrackSimMultiTruth
// ================================================================
// code to match clusters and tracks to GEANT charge deposition
// information
// ================================================================
//   20-04-2009 Antonio Boveia (boveia@hep.uchicago.edu)

long FPGATrackSimMultiTruth::best_barcode() const {
  Barcode code;
  Weight weight;
  return !best(code, weight) ? -999 : code.second;
}

void FPGATrackSimMultiTruth::add(const FPGATrackSimMultiTruth::Barcode& code, const FPGATrackSimMultiTruth::Weight& weight) {
  assert(weight >= 0.);
  m_truth[code] += weight;
}

void FPGATrackSimMultiTruth::add(const FPGATrackSimMultiTruth& rval) {
  for (auto& rval_truth : rval.m_truth) {
    m_truth[rval_truth.first] += rval_truth.second;
  }
}

// if this doesn't have any truth info, assign maxweight_barcode to be the truth.
// otherwise, this should only have the barcode of the largest weight seen so far;
// compare with the input barcode and keep the best.
void FPGATrackSimMultiTruth::maximize(const FPGATrackSimMultiTruth::Barcode& code, const FPGATrackSimMultiTruth::Weight& weight)
{
  if (m_truth.empty()) m_truth[code] = weight;
  else
  {
    assert(m_truth.size() == 1);
    if (m_truth.begin()->second < weight)
    {
      m_truth.clear();
      m_truth[code] = weight;
    }
  }
}

void FPGATrackSimMultiTruth::maximize(const FPGATrackSimMultiTruth& rval)
{
  if (rval.m_truth.empty()) return; // if rval has no truth info, do nothing.

  // find input barcode with maximum weight;
  Weight maxweight = 0.;
  Barcode maxweight_barcode(-1l, -1l);
  for (const auto & rval_truth : rval.m_truth)
  {
    if (rval_truth.second > maxweight)
    {
      maxweight = rval_truth.second;
      maxweight_barcode = rval_truth.first;
    }
  }

  maximize(maxweight_barcode, maxweight);
}

void FPGATrackSimMultiTruth::assign_equal_normalization()
{
  for ( auto& truth : m_truth) truth.second = 1. / m_truth.size();
}

std::ostream& operator<<(std::ostream& o, const FPGATrackSimMultiTruth& mt)
{
  size_t count = std::count_if(mt.begin(), mt.end(), [](auto){return true;});
  o << "FPGATrackSimMultiTruth (event index,barcode) <=> weight, entries: " << count  << "\n";

  if (count == 0) return o;

  for ( auto& truth : mt)
    o << "   " << (truth.first).first << "," << (truth.first).second << " <=> " << truth.second << "\n";

  o << "      best: \n";

  FPGATrackSimMultiTruth::Barcode code;
  FPGATrackSimMultiTruth::Weight weight(0);

  if (!mt.best(code, weight)) o << "N/A\n";
  else o << code.first << "," << code.second << " " << weight << "\n";
  return o;

}

const FPGATrackSimMultiTruth FPGATrackSimMultiTruth::add(const FPGATrackSimMultiTruth& rval) const
{
  FPGATrackSimMultiTruth result(*this);
  result.add(rval);
  return result;
}

const FPGATrackSimMultiTruth FPGATrackSimMultiTruth::maximize(const FPGATrackSimMultiTruth& rval) const
{
  FPGATrackSimMultiTruth result(*this);
  result.maximize(rval);
  return result;
}
