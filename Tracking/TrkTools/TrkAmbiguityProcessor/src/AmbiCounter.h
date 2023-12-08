/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrkAmbiguityProcessor_AmbiCounter_icc
#define TrkAmbiguityProcessor_AmbiCounter_icc
#include <array>
#include <vector>
#include <string>
#include "TrkTrack/Track.h"
#include <stdexcept>
#include <algorithm>
#include <optional>

template<class EnumType>
class AmbiCounter {
public:
  using Categories = EnumType;
  enum RegionIndex {iBarrel , iTransi , iEndcap , iFWD = 3, nRegions=4, iForwrd = 3};
  enum GlobalCounterIndices {
    nEvents,
    nInvalidTracks,
    nTracksWithoutParam,
    nGlobalCounters,
  };
  //
  AmbiCounter(const std::vector<float> &eta_bounds): m_etaBounds(eta_bounds){
    const std::string errMsgPrefix = "In AmbiCounter.icc, eta_bounds size must be ";
    if (m_etaBounds.size()!=nRegions) throw std::runtime_error(errMsgPrefix + std::to_string(nRegions) + " elements long.");
    if (not std::is_sorted(m_etaBounds.begin(), m_etaBounds.end())){
      throw std::runtime_error(errMsgPrefix + "in ascending order.");
    }
  }

  //convert Category to array index
  size_t
  idx(const Categories & categoryIndex) const{
    return static_cast<size_t>(categoryIndex);
  }

  void
  resetGlobalCounters(){
    m_globalCounter.fill(0);
  }

  //increment event count
  void
  newEvent(){
    ++m_globalCounter[nEvents];
  }

  //return number of events
  int
  numberOfEvents() const{
    return m_globalCounter[nEvents];
  }
  // increment one bin
  void
  increment(Categories category, unsigned int etaBinIdx) {
    if ((category>= Categories::kNCounter) or (etaBinIdx >=nRegions)){
      throw std::out_of_range("in AmbiCounter.icc::increment()");
    }
    ++m_counter[idx(category)][etaBinIdx];
  }
  //
  AmbiCounter<EnumType> & operator +=(const AmbiCounter<EnumType> &a) {
  for (unsigned int i=0; i<nGlobalCounters; ++i) {
       m_globalCounter[i]+= a.m_globalCounter[i];
  }
  for (size_t categoryIdx=0; categoryIdx < idx(Categories::kNCounter); ++categoryIdx) {
     for (unsigned int etaBinIdx=0; etaBinIdx < a.m_counter[categoryIdx].size(); ++etaBinIdx) {
        m_counter[categoryIdx][etaBinIdx] += a.m_counter[categoryIdx][etaBinIdx];
     }
  }
  return *this;
}
  //
  void incrementCounterByRegion(Categories categoryIdx,const Trk::Track* track){
   // test
   if (!track) {
      ++m_globalCounter[nInvalidTracks];
      return;
   }
   // use first parameter
   if (!track->trackParameters()) {
      ++m_globalCounter[nTracksWithoutParam];
   } else {
      std::array<int, nRegions> &nTracks = m_counter.at(idx(categoryIdx));
      // @TODO make sure that list of track parameters is not empty
      const double absEta = std::abs(track->trackParameters()->front()->eta());
      if (const auto &possibleIdx{etaBin(absEta)}){//i.e. if it's within bounds
        ++nTracks[possibleIdx.value()];
      }
    }
  }
  //
  std::string
  dumpRegions(const std::string & head,Categories categoryIdx, const int iw =9) const {
    std::stringstream out;
    out << head;
    if (categoryIdx >= Categories::kNCounter) throw std::out_of_range("Array index out of range in AmbiCounter::inc by region");
    const auto & displayedArray = m_counter[idx(categoryIdx)];
    const auto allRegionCounts  = std::accumulate(displayedArray.begin(), displayedArray.end(),0);
    out << std::setiosflags(std::ios::dec) << std::setw(iw) << allRegionCounts;
    for (unsigned int etaBinIdx=0; etaBinIdx < nRegions; ++etaBinIdx) {
       out << std::setiosflags(std::ios::dec) << std::setw(iw) << m_counter[idx(categoryIdx)][etaBinIdx];
    }
    out << "\n";
    return out.str();
  }
  //
  int
  globalCount(GlobalCounterIndices i) const{
    return m_globalCounter[i];
  }

private:
  std::array<std::array<int, nRegions>,static_cast<size_t>(Categories::kNCounter)> m_counter{};
  std::array<int,nGlobalCounters>                      m_globalCounter{};
  const std::vector<float>  &m_etaBounds;           //!< eta intervals for internal monitoring
  std::optional<size_t>
  etaBin(const double val){
    auto pVal =  std::lower_bound(m_etaBounds.begin(), m_etaBounds.end(), val);
    //if it's in bounds, return the value, otherwise return a nullopt
    return (pVal!=m_etaBounds.end()) ? std::optional<size_t>(std::distance(m_etaBounds.begin(), pVal)):std::nullopt;
  }
};

#endif
