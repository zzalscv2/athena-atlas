// -*- c++ -*-
/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKSYSTEMATICSTOOLS_INDETTRACKBIASINGTOOL_H
#define INDETTRACKSYSTEMATICSTOOLS_INDETTRACKBIASINGTOOL_H

#include "InDetTrackSystematicsTools/IInDetTrackBiasingTool.h"
#include "AsgTools/AsgTool.h"
#include "PATInterfaces/CorrectionTool.h"
#include "InDetTrackSystematicsTools/InDetTrackSystematicsTool.h"
#include "xAODTracking/TrackParticleContainer.h"
#include <string>
#include <vector>

#include <TH2.h>

#include "TRandom3.h"

class TFile;

namespace InDet {

  /// @class InDetTrackBiasingTool
  /// This class biases tracks to emulate systematic distortions of the tracking geometry.
  /// In data, it corrects the biases.
  /// In simulation, it applys the biases in the same direction they are observed in the data.
  /// @author Pawel Bruckman (pawel.bruckman.de.renstrom@cern.ch)
  /// @author Felix Clark (michael.ryan.clark@cern.ch)

  class InDetTrackBiasingTool
    : public virtual IInDetTrackBiasingTool
    , public virtual InDetTrackSystematicsTool
    , public virtual CP::CorrectionTool< xAOD::TrackParticleContainer >
  {

    ASG_TOOL_CLASS( InDetTrackBiasingTool,
        InDet::IInDetTrackBiasingTool )

    public:

    InDetTrackBiasingTool (const std::string& name);
    virtual ~InDetTrackBiasingTool();

    virtual StatusCode initialize() override;
    virtual void prepare() override {};

    /// Computes the tracks origin
    virtual CP::CorrectionCode applyCorrection(xAOD::TrackParticle& track) override;
    virtual CP::CorrectionCode correctedCopy( const xAOD::TrackParticle& in,
                xAOD::TrackParticle*& out ) override;
    virtual CP::CorrectionCode applyContainerCorrection( xAOD::TrackParticleContainer& cont ) override;


    /// returns: whether the tool is affected by the systematic
    virtual bool isAffectedBySystematic( const CP::SystematicVariation& ) const override;
    /// returns: list of systematics this tool can be affected by
    virtual CP::SystematicSet affectingSystematics() const override;
    /// returns: list of recommended systematics to use with this tool
    virtual CP::SystematicSet recommendedSystematics() const override;
    /// configure the tool to apply a given list of systematic variations
    virtual StatusCode applySystematicVariation( const CP::SystematicSet& ) override;

  protected:

    StatusCode initHistograms();
    StatusCode firstCall();

    float readHistogram(float fDefault, TH2* histogram, float phi, float eta) const;

    float m_biasD0 = 0.f;
    float m_biasZ0 = 0.f;
    float m_biasQoverPsagitta = 0.f;

    std::unique_ptr<TH2> m_data15_biasD0Histogram = nullptr; //!
    std::unique_ptr<TH2> m_data15_biasZ0Histogram = nullptr; //!
    std::unique_ptr<TH2> m_data15_biasQoverPsagittaHistogram = nullptr; //!
    std::unique_ptr<TH2> m_data15_biasD0HistError = nullptr; //!
    std::unique_ptr<TH2> m_data15_biasZ0HistError = nullptr; //!
    std::unique_ptr<TH2> m_data15_biasQoverPsagittaHistError = nullptr; //!

    std::unique_ptr<TH2> m_data16_1stPart_biasD0Histogram = nullptr; //!
    std::unique_ptr<TH2> m_data16_1stPart_biasZ0Histogram = nullptr; //!
    std::unique_ptr<TH2> m_data16_1stPart_biasQoverPsagittaHistogram = nullptr; //!
    std::unique_ptr<TH2> m_data16_1stPart_biasD0HistError = nullptr; //!
    std::unique_ptr<TH2> m_data16_1stPart_biasZ0HistError = nullptr; //!
    std::unique_ptr<TH2> m_data16_1stPart_biasQoverPsagittaHistError = nullptr; //!

    std::unique_ptr<TH2> m_data16_2ndPart_biasD0Histogram = nullptr; //!
    std::unique_ptr<TH2> m_data16_2ndPart_biasZ0Histogram = nullptr; //!
    std::unique_ptr<TH2> m_data16_2ndPart_biasQoverPsagittaHistogram = nullptr; //!
    std::unique_ptr<TH2> m_data16_2ndPart_biasD0HistError = nullptr; //!
    std::unique_ptr<TH2> m_data16_2ndPart_biasZ0HistError = nullptr; //!
    std::unique_ptr<TH2> m_data16_2ndPart_biasQoverPsagittaHistError = nullptr; //!

    std::unique_ptr<TH2> m_data17_1stPart_biasD0Histogram = nullptr; //!
    std::unique_ptr<TH2> m_data17_1stPart_biasZ0Histogram = nullptr; //!
    std::unique_ptr<TH2> m_data17_1stPart_biasQoverPsagittaHistogram = nullptr; //!
    std::unique_ptr<TH2> m_data17_1stPart_biasD0HistError = nullptr; //!
    std::unique_ptr<TH2> m_data17_1stPart_biasZ0HistError = nullptr; //!
    std::unique_ptr<TH2> m_data17_1stPart_biasQoverPsagittaHistError = nullptr; //!

    std::unique_ptr<TH2> m_data17_2ndPart_biasD0Histogram = nullptr; //!
    std::unique_ptr<TH2> m_data17_2ndPart_biasZ0Histogram = nullptr; //!
    std::unique_ptr<TH2> m_data17_2ndPart_biasQoverPsagittaHistogram = nullptr; //!
    std::unique_ptr<TH2> m_data17_2ndPart_biasD0HistError = nullptr; //!
    std::unique_ptr<TH2> m_data17_2ndPart_biasZ0HistError = nullptr; //!
    std::unique_ptr<TH2> m_data17_2ndPart_biasQoverPsagittaHistError = nullptr; //!

    std::unique_ptr<TH2> m_data18_1stPart_biasD0Histogram = nullptr; //!
    std::unique_ptr<TH2> m_data18_1stPart_biasZ0Histogram = nullptr; //!
    std::unique_ptr<TH2> m_data18_1stPart_biasQoverPsagittaHistogram = nullptr; //!
    std::unique_ptr<TH2> m_data18_1stPart_biasD0HistError = nullptr; //!
    std::unique_ptr<TH2> m_data18_1stPart_biasZ0HistError = nullptr; //!
    std::unique_ptr<TH2> m_data18_1stPart_biasQoverPsagittaHistError = nullptr; //!

    std::unique_ptr<TH2> m_data18_2ndPart_biasD0Histogram = nullptr; //!
    std::unique_ptr<TH2> m_data18_2ndPart_biasZ0Histogram = nullptr; //!
    std::unique_ptr<TH2> m_data18_2ndPart_biasQoverPsagittaHistogram = nullptr; //!
    std::unique_ptr<TH2> m_data18_2ndPart_biasD0HistError = nullptr; //!
    std::unique_ptr<TH2> m_data18_2ndPart_biasZ0HistError = nullptr; //!
    std::unique_ptr<TH2> m_data18_2ndPart_biasQoverPsagittaHistError = nullptr; //!

    // if neither of these is set manually, then the tool will try to get this from EventInfo in the event store.
    // these options exist only as a manual override.
    bool m_isData = false;
    bool m_isSimulation = false;
    uint32_t m_runNumber = 0;

    bool m_doD0Bias = true;
    bool m_doZ0Bias = true;
    bool m_doQoverPBias = true;

    // allow the user to configure which calibration files to use if desired
    std::string m_calibFileData15;
    std::string m_calibFileData16_1stPart;
    std::string m_calibFileData16_2ndPart;
    std::string m_calibFileData17_1stPart;
    std::string m_calibFileData17_2ndPart;
    std::string m_calibFileData18_1stPart;
    std::string m_calibFileData18_2ndPart;

    // paths and histogram names in the calibration files
    std::string m_d0_nominal_histName = "d0/d0_theNominal";
    std::string m_z0_nominal_histName = "z0/z0_theNominal";
    std::string m_sagitta_nominal_histName = "sagitta/sagitta_theNominal";
    std::string m_d0_uncertainty_histName = "d0/d0_theUncertainty";
    std::string m_z0_uncertainty_histName = "z0/z0_theUncertainty";
    std::string m_sagitta_uncertainty_histName = "sagitta/sagitta_theUncertainty";

  }; // class InDetTrackBiasingTool

} // namespace InDet

#endif
