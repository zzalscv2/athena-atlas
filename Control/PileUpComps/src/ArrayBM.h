/*  -*- C++ -*- */

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PILEUPCOMPS_ARRAYBM
#define PILEUPCOMPS_ARRAYBM 1
/** @file ArrayBM.h
 * @brief A IBeamIntensity service configured with an intensity array
 * The Gaudi::Property<std::vector<float>> describes the intensity pattern that is
 * repeated for the entire beam xing range.
 *
 * $Id: BkgStreamsCache.h,v 1.10 2008-08-28 01:11:06 calaf Exp $
 * @author Paolo Calafiura - ATLAS Collaboration
 */
#include "GaudiKernel/ContextSpecificPtr.h"
#include "GaudiKernel/ServiceHandle.h"
#include "Gaudi/Property.h"

#include "PileUpTools/IBeamIntensity.h"
#include "CxxUtils/checker_macros.h"
#include "AthenaBaseComps/AthService.h"
#include "AthenaKernel/IAthRNGSvc.h"

#include <boost/random.hpp>

namespace CLHEP
{
  class RandGeneral;
}

class ArrayBM : public extends<AthService, IBeamIntensity>
{
public:
  /// \name Constructor and Destructor
  //@{
  ArrayBM(const std::string& name,ISvcLocator* svc);
  virtual ~ArrayBM();
  //@}
  /// \name AthService methods
  //@{
  virtual StatusCode initialize() override final;
  //@}
  /// \name IBeamIntensity methods
  //@{
  virtual float normFactor(int iXing) const override final;
  virtual float largestElementInPattern() const override final { return m_largestElementInPattern; }
  virtual void selectT0(unsigned int run, unsigned long long event) override final;
  virtual unsigned int getCurrentT0BunchCrossing() const override final { return m_t0Offset; }
  virtual unsigned int getBeamPatternLength() const override final { return m_ipLength; }
  //@}
private:
  /// max bunch crossings per orbit
  unsigned int m_maxBunchCrossingPerOrbit;
  /// offset of the t0 wrto our intensity pattern
  Gaudi::Hive::ContextSpecificData<unsigned int> m_t0Offset;
  /// seed for FastReseededPRNG. Non-zero switches to using FastReseededPRNG.
  Gaudi::Property<std::uint64_t> m_seed{this, "Seed", 0, "Seed for FastReseededPRNG. Zero seed switches to AthRNGSvc mode."};
  /// user-defined intensity pattern
  Gaudi::Property<std::vector<float>> m_intensityPatternProp;
  /// length of the intensity pattern
  unsigned int m_ipLength;
  /// normalized intensity pattern. C array to make clhep RandGeneral happy
  double* m_intensityPattern;
  /// shoot random number proportionally to m_intensityPattern
  CLHEP::RandGeneral* m_biRandom;
  /// as with m_biRandom, but for FastReseededPRNG
  std::unique_ptr<const boost::random::discrete_distribution<unsigned int>> m_t0Dist{nullptr};
  /// the service managing our random seeds/sequences
  ServiceHandle<IAthRNGSvc> m_randomSvc{this, "RandomSvc", "AthRNGSvc","The random number service that will be used."};
  ATHRNG::RNGWrapper*           m_rngWrapper ATLAS_THREAD_SAFE{};
  /// The largest value in the pattern assuming that the pattern has
  /// mean value 1.0. Multiplying by this converts values in the
  /// m_intensityPattern from having max value 1.0 to having mean
  /// value 1.0.
  float m_largestElementInPattern;
  /// Empty bunch option.  Default (0) means no special treatment of
  /// empty bunches, signal goes in filled bunches.  Negative number
  /// means put signal in any empty bunch.  Positive number means put
  /// signal in one of the first N bunches after a filled bunch.
  int m_emptyBunches;
  /// Additional array for keeping the locations we want signal in
  /// By default, will match the intensity pattern
  double* m_signalPattern;
};
#endif
