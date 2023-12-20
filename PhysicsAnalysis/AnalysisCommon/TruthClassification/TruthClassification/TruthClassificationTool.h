/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRUTH_CLASSIFICATION__TRUTH_CLASSIFICATION_TOOL_H_
#define TRUTH_CLASSIFICATION__TRUTH_CLASSIFICATION_TOOL_H_

#include <string>

#include <AsgAnalysisInterfaces/ITruthClassificationTool.h>
#include <AsgTools/AsgTool.h>
#include <xAODEgamma/Electron.h>
#include <xAODMuon/Muon.h>
#include <xAODTruth/TruthParticle.h>


/// \brief a tool to classify particles based on their type and origin
class TruthClassificationTool : virtual public ITruthClassificationTool, public asg::AsgTool
{
  ASG_TOOL_CLASS2(TruthClassificationTool, IClassificationTool, ITruthClassificationTool)

public:
  explicit TruthClassificationTool(const std::string &type);

  /// \brief classify and return unsigned int
  virtual StatusCode classify(const xAOD::IParticle &particle,
                              unsigned int &classification) const override;

  /// \brief classify and return Truth::Type
  virtual StatusCode classify(const xAOD::IParticle &particle,
                              Truth::Type &classification) const override;

private:
  /// \brief electron classification helper
  StatusCode classifyElectron(const xAOD::IParticle &electron,
                              Truth::Type &classification) const;

  /// \brief muon classification helper
  StatusCode classifyMuon(const xAOD::IParticle &muon,
                          Truth::Type &classification) const;

  /// \brief separately store charge-flip electrons/muons
  bool m_separateChargeFlipElectrons = false;
  bool m_separateChargeFlipMuons = false;

  /// \brief use truth particle decorations
  bool m_useTruthParticleDecorations{false};

  // accessors
  const SG::AuxElement::ConstAccessor<int> m_truthType{"truthType"};
  const SG::AuxElement::ConstAccessor<int> m_truthOrigin{"truthOrigin"};
  const SG::AuxElement::ConstAccessor<int> m_truthPdgId{"truthPdgId"};
  const SG::AuxElement::ConstAccessor<unsigned int> m_classifierParticleType{"classifierParticleType"};
  const SG::AuxElement::ConstAccessor<unsigned int> m_classifierParticleOrigin{"classifierParticleOrigin"};
  const SG::AuxElement::ConstAccessor<int> m_firstMotherTruthType{"firstEgMotherTruthType"};
  const SG::AuxElement::ConstAccessor<int> m_firstMotherTruthOrigin{"firstEgMotherTruthOrigin"};
  const SG::AuxElement::ConstAccessor<int> m_firstMotherPdgId{"firstEgMotherPdgId"};
  const SG::AuxElement::ConstAccessor<int> m_lastMotherTruthType{"lastEgMotherTruthType"};
  const SG::AuxElement::ConstAccessor<int> m_lastMotherTruthOrigin{"lastEgMotherTruthOrigin"};
  const SG::AuxElement::ConstAccessor<int> m_lastMotherPdgId{"lastEgMotherPdgId"};
  const SG::AuxElement::ConstAccessor<int> m_fallbackTruthType{"TruthClassifierFallback_truthType"};
  const SG::AuxElement::ConstAccessor<int> m_fallbackTruthOrigin{"TruthClassifierFallback_truthOrigin"};
  const SG::AuxElement::ConstAccessor<float> m_fallbackDR{"TruthClassifierFallback_dR"};
  
  /// \brief a helper to check if an electron is prompt
  bool isPromptElectron(const xAOD::IParticle &electron,
                        bool isTruthParticle,
                        const xAOD::TruthParticle *truthParticle) const;
  /// \brief a helper to check if an electron has an incorrectly reconstructed charge
  bool isChargeFlipElectron(const xAOD::IParticle &electron,
                            bool isTruthParticle,
                            const xAOD::TruthParticle *truthParticle) const;
  /// \brief a helper to check if a muon has an incorrectly reconstructed charge
  bool isChargeFlipMuon(const xAOD::IParticle &muon,
                        bool isTruthParticle,
                        const xAOD::TruthParticle *truthParticle) const;

  /// \brief a helper to check if the origin is a b-hadron
  bool hasBHadronOrigin(int origin) const;
  /// \brief a helper to check if the origin is a c-hadron
  bool hasCHadronOrigin(int origin) const;
  /// \brief a helper to check if the origin is a light hadron
  bool hasLightHadronOrigin(int origin) const;
};

#endif  // TRUTH_CLASSIFICATION__TRUTH_CLASSIFICATION_TOOL_H_
