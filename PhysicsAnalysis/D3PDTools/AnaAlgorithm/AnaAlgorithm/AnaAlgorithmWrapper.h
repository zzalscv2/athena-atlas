/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef ANA_ALGORITHM__ANA_ALGORITHM_WRAPPER_H
#define ANA_ALGORITHM__ANA_ALGORITHM_WRAPPER_H

#include <AnaAlgorithm/Global.h>

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AnaAlgorithm/AnaAlgorithmConfig.h>
#include <AnaAlgorithm/IAlgorithmWrapper.h>

namespace EL
{
  /// \brief an \ref IAlgorithmWrapper for \ref AnaAlgorithm

  class AnaAlgorithmWrapper final : public IAlgorithmWrapper
  {
  public:

    /// Public Members
    /// ==============

    /// \brief test the invariant of this object
    void testInvariant () const;

    /// \brief standard default constructor for serialization
    AnaAlgorithmWrapper () {};

    /// \brief standard constructor
    AnaAlgorithmWrapper (AnaAlgorithmConfig val_config);



    /// Inherited Members
    /// =================

    virtual std::string_view getName () const override;

    virtual bool hasName (const std::string& name) const override;

    virtual std::unique_ptr<IAlgorithmWrapper> makeClone() const override;

    virtual StatusCode initialize (const AlgorithmWorkerData& workerData) override;

    virtual StatusCode execute () override;

    virtual StatusCode finalize () override;

    virtual ::StatusCode fileExecute () override;

    virtual ::StatusCode beginInputFile () override;

    virtual ::StatusCode endInputFile () override;



    /// Private Members
    /// ===============

  private:

    /// \brief the algorithm configuration
    AnaAlgorithmConfig m_config;

    /// \brief the actual algorithm (once instantiated)
    ///
    /// Please note (and do not remove) the comment at the end of the
    /// line.  This specifically excludes this member from
    /// serialization.
    std::unique_ptr<AnaAlgorithm> m_algorithm; //!
  };
}

#endif
