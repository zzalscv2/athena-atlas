/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "JetUncertainties/CombinedMassUncertaintyComponent.h"
#include "JetUncertainties/Helpers.h"
#include "JetUncertainties/UncertaintyGroup.h"

#include "TFile.h"

namespace jet
{

//////////////////////////////////////////////////
//                                              //
//  Constructor/destructor/initialization       //
//                                              //
//////////////////////////////////////////////////

CombinedMassUncertaintyComponent::CombinedMassUncertaintyComponent(const std::string& name)
    : UncertaintyComponent(ComponentHelper(name),0)
    , m_combMassType(CombMassComp::UNKNOWN)
    , m_setWeightMassDefs(false)
    , m_caloMassComp(nullptr)
    , m_TAMassComp(nullptr)
    , m_caloMassWeight(nullptr)
    , m_TAMassWeight(nullptr)
    , m_caloMassScale_weights("")
    , m_TAMassScale_weights("")
    , m_weightParam(CompParametrization::UNKNOWN)
    , m_truthLabelName("")
    , m_truthLabels()
{
    JESUNC_NO_DEFAULT_CONSTRUCTOR;
}

CombinedMassUncertaintyComponent::CombinedMassUncertaintyComponent(const ComponentHelper& component)
    : UncertaintyComponent(component,0)
    , m_combMassType(component.combMassType)
    , m_setWeightMassDefs(false)
    , m_caloMassComp(nullptr)
    , m_TAMassComp(nullptr)
    , m_caloMassWeight(nullptr)
    , m_TAMassWeight(nullptr)
    , m_caloMassScale_weights("")
    , m_TAMassScale_weights("")
    , m_weightParam(CompParametrization::UNKNOWN)
    , m_truthLabelName(component.LargeRJetTruthLabelName)
    , m_truthLabels(component.LargeRJetTruthLabels)
{
    ATH_MSG_DEBUG("Created CombinedMassUncertaintyComponent named " << getName().Data());
}

CombinedMassUncertaintyComponent::CombinedMassUncertaintyComponent(const CombinedMassUncertaintyComponent& toCopy)
    : UncertaintyComponent(toCopy)
    , m_combMassType(toCopy.m_combMassType)
    , m_setWeightMassDefs(toCopy.m_setWeightMassDefs)
    , m_caloMassComp(nullptr)
    , m_TAMassComp(nullptr)
    , m_caloMassWeight(toCopy.m_caloMassWeight)
    , m_TAMassWeight(toCopy.m_TAMassWeight)
    , m_caloMassScale_weights(toCopy.m_caloMassScale_weights)
    , m_TAMassScale_weights(toCopy.m_TAMassScale_weights)
    , m_weightParam(toCopy.m_weightParam)
    , m_truthLabelName(toCopy.m_truthLabelName)
    , m_truthLabels(toCopy.m_truthLabels)
{
    ATH_MSG_DEBUG("Creating copy of CombinedMassUncertaintyComponent named " << getName().Data());
    if (toCopy.m_caloMassComp)
        m_caloMassComp = new UncertaintyGroup(*toCopy.m_caloMassComp);
    if (toCopy.m_TAMassComp)
        m_TAMassComp = new UncertaintyGroup(*toCopy.m_TAMassComp);
}

CombinedMassUncertaintyComponent* CombinedMassUncertaintyComponent::clone() const
{
    return new CombinedMassUncertaintyComponent(*this);
}

CombinedMassUncertaintyComponent::~CombinedMassUncertaintyComponent()
{
    JESUNC_SAFE_DELETE(m_caloMassComp);
    JESUNC_SAFE_DELETE(m_TAMassComp);
}

StatusCode CombinedMassUncertaintyComponent::setCaloTerm(UncertaintyGroup* caloComp)
{
    if (m_isInit)
    {
        ATH_MSG_ERROR("Can only set the calo mass term before initialization: " << getName().Data());
        return StatusCode::FAILURE;
    }
    if (m_caloMassComp != nullptr)
    {
        ATH_MSG_ERROR("Calo mass term has already been set, blocking double-init: " << getName().Data());
        return StatusCode::FAILURE;
    }
    m_caloMassComp = caloComp;

    return StatusCode::SUCCESS;
}

StatusCode CombinedMassUncertaintyComponent::setTATerm(UncertaintyGroup* TAComp)
{
    if (m_isInit)
    {
        ATH_MSG_ERROR("Can only set the TA mass term before initialization: " << getName().Data());
        return StatusCode::FAILURE;
    }
    if (m_TAMassComp != nullptr)
    {
        ATH_MSG_ERROR("TA mass term has already been set, blocking double-init: " << getName().Data());
        return StatusCode::FAILURE;
    }
    m_TAMassComp = TAComp;

    return StatusCode::SUCCESS;
}

StatusCode CombinedMassUncertaintyComponent::setCaloWeights(const UncertaintyHistogram* caloWeights)
{
    if (m_isInit)
    {
        ATH_MSG_ERROR("Can only set the calo mass weights before initialization: " << getName().Data());
        return StatusCode::FAILURE;
    }
    if (m_caloMassWeight != nullptr)
    {
        ATH_MSG_ERROR("Calo mass weights has already been set, blocking double-init: " << getName().Data());
        return StatusCode::FAILURE;
    }
    m_caloMassWeight = caloWeights;

    return StatusCode::SUCCESS;
}

StatusCode CombinedMassUncertaintyComponent::setTAWeights(const UncertaintyHistogram* TAWeights)
{
    if (m_isInit)
    {
        ATH_MSG_ERROR("Can only set the TA mass weights before initialization: " << getName().Data());
        return StatusCode::FAILURE;
    }
    if (m_TAMassWeight != nullptr)
    {
        ATH_MSG_ERROR("TA mass weights has already been set, blocking double-init: " << getName().Data());
        return StatusCode::FAILURE;
    }
    m_TAMassWeight = TAWeights;

    return StatusCode::SUCCESS;
}

StatusCode CombinedMassUncertaintyComponent::setCombWeightMassDefs(const CompMassDef::TypeEnum caloMassDef, const CompMassDef::TypeEnum TAMassDef)
{
    if (m_isInit)
    {
        ATH_MSG_ERROR("Can only set the weight mass definitions before initialization: " << getName().Data());
        return StatusCode::FAILURE;
    }
    if (m_setWeightMassDefs)
    {
        ATH_MSG_ERROR("Already set the weights, blocking double-setting: " << getName().Data());
        return StatusCode::FAILURE;
    }
    switch (caloMassDef)
    {
        case CompMassDef::CaloMass:
        case CompMassDef::TAMass:
            m_caloMassScale_weights = JetFourMomAccessor(CompMassDef::getJetScaleString(caloMassDef).Data());
            break;
        default:
            ATH_MSG_ERROR("Unsupported mass parametrization for the combined mass calo weights: " << getName().Data());
            return StatusCode::FAILURE;
    }
    switch (TAMassDef)
    {
        case CompMassDef::CaloMass:
        case CompMassDef::TAMass:
            m_TAMassScale_weights = JetFourMomAccessor(CompMassDef::getJetScaleString(TAMassDef).Data());
            break;
        default:
            ATH_MSG_ERROR("Unsupported mass parametrization for the combined mass TA weights: " << getName().Data());
            return StatusCode::FAILURE;
    }

    m_setWeightMassDefs = true;
    return StatusCode::SUCCESS;
}

StatusCode CombinedMassUncertaintyComponent::setCombWeightParam(const CompParametrization::TypeEnum param)
{
    if (m_isInit)
    {
        ATH_MSG_ERROR("Can only set the weight parametrization before initialization: " << getName().Data());
        return StatusCode::FAILURE;
    }
    m_weightParam = param;

    return StatusCode::SUCCESS;
}

StatusCode CombinedMassUncertaintyComponent::initialize(TFile* histFile)
{
    // We are completely different here than the normal case
    // Ignore the base component initialization procedure
    
    // Prevent double-initialization
    if (m_isInit)
    {
        ATH_MSG_ERROR("Component is already initialized: " << getName().Data());
        return StatusCode::FAILURE;
    }
    
    // Ensure that the combination weights exist
    if (!m_caloMassWeight)
    {
        ATH_MSG_ERROR("Calorimeter mass weights were not defined: " << getName().Data());
        return StatusCode::FAILURE;
    }
    if (!m_TAMassWeight)
    {
        ATH_MSG_ERROR("Track-assisted mass weights were not defined: " << getName().Data());
        return StatusCode::FAILURE;
    }

    // Ensure that the weight mass definitions were set
    if (!m_setWeightMassDefs)
    {
        ATH_MSG_ERROR("The mass definitions for the combination weight factors were not set: " << getName().Data());
        return StatusCode::FAILURE;
    }

    // Understand what situation we are in
    if      (m_combMassType == CombMassComp::UNKNOWN)
    {
        ATH_MSG_ERROR("Unknown combined mass uncertainty type: " << getName().Data());
        return StatusCode::FAILURE;
    }
    else if (m_combMassType == CombMassComp::Calo)
    {
        // Only the calo mass component should be specified
        if (!m_caloMassComp)
        {
            ATH_MSG_ERROR("Calo mass term was not specified for a CombMass_calo component: " << getName().Data());
            return StatusCode::FAILURE;
        }
        if (m_TAMassComp)
        {
            ATH_MSG_ERROR("TA mass term was specified for a CombMass_calo component: " << getName().Data());
            return StatusCode::FAILURE;
        }
    }
    else if (m_combMassType == CombMassComp::TA)
    {
        // Only the TA mass component should be specified
        if (m_caloMassComp)
        {
            ATH_MSG_ERROR("Calo mass term was specified for a CombMass_TA component: " << getName().Data());
            return StatusCode::FAILURE;
        }
        if (!m_TAMassComp)
        {
            ATH_MSG_ERROR("TA mass term was not specified for a CombMass_TA component: " << getName().Data());
            return StatusCode::FAILURE;
        }
    }
    else if (m_combMassType == CombMassComp::Both)
    {
        // Both components should be specified
        if (!m_caloMassComp)
        {
            ATH_MSG_ERROR("Calo mass term was not specified for a CombMass_both component: " << getName().Data());
            return StatusCode::FAILURE;
        }
        if (!m_TAMassComp)
        {
            ATH_MSG_ERROR("TA mass term was not specified for a CombMass_both component: " << getName().Data());
            return StatusCode::FAILURE;
        }
    }

    // Initialize the component(s)
    if (m_caloMassComp && m_caloMassComp->initialize(histFile).isFailure())
    {
        ATH_MSG_ERROR("Failed to initialize calo mass portion of a comb mass uncertainty: " << getName().Data());
        return StatusCode::FAILURE;
    }
    if (m_TAMassComp && m_TAMassComp->initialize(histFile).isFailure())
    {
        ATH_MSG_ERROR("Failed to initialize TA mass portion of a comb mass uncertainty: " << getName().Data());
        return StatusCode::FAILURE;
    }


    // Done!
    m_isInit = true;
    return StatusCode::SUCCESS;
}


//////////////////////////////////////////////////
//                                              //
//  Validity and uncertainty retrieval          //
//                                              //
//////////////////////////////////////////////////

bool CombinedMassUncertaintyComponent::getValidityImpl(const xAOD::Jet& jet, const xAOD::EventInfo& eInfo) const
{
    return getValidityCalo(jet,eInfo) && getValidityTA(jet,eInfo);
}

StatusCode CombinedMassUncertaintyComponent::calculateCombinedMass(const xAOD::Jet& jet, const double shiftFactorCalo, const double shiftFactorTA, double& combMass) const
{
    // Accessors for the scales we need
    static const JetFourMomAccessor caloMassScale(CompMassDef::getJetScaleString(CompMassDef::CaloMass).Data());
    static const JetFourMomAccessor TAMassScale(CompMassDef::getJetScaleString(CompMassDef::TAMass).Data());
    static const JetFourMomAccessor combMassScale(CompMassDef::getJetScaleString(CompMassDef::CombMassQCD).Data());

    // Get the weight factors
    const double factorCalo = getWeightFactorCalo(jet,shiftFactorCalo);
    const double factorTA   = getWeightFactorTA(jet,shiftFactorTA);

    // Watch for division by zero
    if (factorCalo+factorTA == 0)
    {
        if (combMassScale.m(jet) == 0)
        {
            // JetCalibTools sets the mass to zero when this situation occurs
            // This is therefore not an error state, rather we want to be consistent
            combMass = 0;
            return StatusCode::SUCCESS;
        }
        else
        {
            // We somehow have invalid calo and TA masses, but a "valid" combined mass
            // This is an error state that should result in a message to the user
            // If this occurs, most likely there is something wrong in the inputs
            ATH_MSG_ERROR("Encountered division by zero when calculating weights: mCalo = " << caloMassScale.m(jet) << ", mTA = " << TAMassScale.m(jet) << ", mComb = " << combMassScale.m(jet));
            return StatusCode::FAILURE;
        }
    }

    // Calculate the weights
    const double caloWeight = factorCalo/(factorCalo+factorTA);
    const double TAWeight   = factorTA/(factorCalo+factorTA);

    // Watch for zero masses
    // If one mass is zero, use the other without a weight
    // If both are zero, it doesn't matter, the combined mass is zero
    if (caloMassScale.m(jet) == 0)
        combMass = TAMassScale.m(jet)*shiftFactorTA;
    else if (TAMassScale.m(jet) == 0)
        combMass = caloMassScale.m(jet)*shiftFactorCalo;
    else
        combMass = (caloMassScale.m(jet)*shiftFactorCalo*caloWeight) + (TAMassScale.m(jet)*shiftFactorTA*TAWeight);

    return StatusCode::SUCCESS;
}

double CombinedMassUncertaintyComponent::getUncertaintyImpl(const xAOD::Jet& jet, const xAOD::EventInfo& eInfo) const
{
    // Check if we need to do anything at all - this uncertainty may be zero based on the jet truth label
    // Truth labels are usually not expected for this component, so check if we need to do this
    if (!m_truthLabels.empty())
    {
        // Truth labels are specified, so we need to check if this jet is labelled appropriately or not
        const SG::AuxElement::ConstAccessor<int> accTruthLabel(m_truthLabelName);
        if (!accTruthLabel.isAvailable(jet) || accTruthLabel(jet) == LargeRJetTruthLabel::UNKNOWN)
        {
            ATH_MSG_ERROR("Unable to retrieve the LargeRJetTruthLabel: " << m_truthLabelName << " from the jet.  Please use JetTruthLabelingTool before calling this function.");
            return JESUNC_ERROR_CODE;
        }
        const LargeRJetTruthLabel::TypeEnum jetTruthLabel = LargeRJetTruthLabel::intToEnum(accTruthLabel(jet));
    
        // We now have the truth jet label, check if it matches one of the label(s) assigned to this component
        // The uncertainty is only applied to same-labelled jets (zero otherwise)
        bool labelApplies = false;
        for (const LargeRJetTruthLabel::TypeEnum aLabel : m_truthLabels)
        {
            if (aLabel == jetTruthLabel)
            {
                labelApplies = true;
                break;
            }
        }
        // If the jet is not of the relevant label, then there is no uncertainty for this jet
        if (!labelApplies)
            return 0;
    }

    // Get the per-part uncertainties
    double uncCalo = 0;
    if (m_caloMassComp && !m_caloMassComp->getValidUncertainty(uncCalo,jet,eInfo,CompScaleVar::Mass))
    {
        // Not valid
        ATH_MSG_ERROR("Combined mass is outside of the validity range (calo part)");
        return JESUNC_ERROR_CODE;
    }
    double uncTA = 0;
    if (m_TAMassComp && !m_TAMassComp->getValidUncertainty(uncTA,jet,eInfo,CompScaleVar::Mass))
    {
        // Not valid
        ATH_MSG_ERROR("Combined mass is outside of the validity range (TA part)");
        return JESUNC_ERROR_CODE;
    }

    // Get the mass values (both up and down shifts)
    // The resolution functions evaluated for masses shifted up and down may not be symmetric
    // For now, take the average of the up and down shifts
    // Note that we also only consider correlated shifts here
    // Uncorrelated shifts can be handled as two separate components
    // Anticorelated shifts are currently not supported (no need so far)
    double massDefault = 0;
    if (!calculateCombinedMass(jet,1,1,massDefault))            return JESUNC_ERROR_CODE;
    if (massDefault == 0) return 0; // This is a relative uncertainty
    double massUp = 0;
    if (!calculateCombinedMass(jet,1+uncCalo,1+uncTA,massUp))   return JESUNC_ERROR_CODE;
    double massDown = 0;
    if (!calculateCombinedMass(jet,1-uncCalo,1-uncTA,massDown)) return JESUNC_ERROR_CODE;
    
    const double massUncUp   = fabs((massUp-massDefault)/massDefault);
    const double massUncDown = fabs((massDown-massDefault)/massDefault);

    return (massUncUp+massUncDown)/2.;
}



bool CombinedMassUncertaintyComponent::getValidityCalo(const xAOD::Jet& jet, const xAOD::EventInfo& eInfo) const
{
    return !m_caloMassComp ? true : m_caloMassComp->getValidity(jet,eInfo,CompScaleVar::Mass);
}

bool CombinedMassUncertaintyComponent::getValidityTA(const xAOD::Jet& jet, const xAOD::EventInfo& eInfo) const
{
    return !m_TAMassComp ? true : m_TAMassComp->getValidity(jet,eInfo,CompScaleVar::Mass);
}

double CombinedMassUncertaintyComponent::getUncertaintyCalo(const xAOD::Jet& jet, const xAOD::EventInfo& eInfo) const
{
    return !m_caloMassComp ? 0 : m_caloMassComp->getUncertainty(jet,eInfo,CompScaleVar::Mass);
}

double CombinedMassUncertaintyComponent::getUncertaintyTA(const xAOD::Jet& jet, const xAOD::EventInfo& eInfo) const
{
    return !m_TAMassComp ? 0 : m_TAMassComp->getUncertainty(jet,eInfo,CompScaleVar::Mass);
}

bool CombinedMassUncertaintyComponent::getValidUncertaintyCalo(double& unc, const xAOD::Jet& jet, const xAOD::EventInfo& eInfo) const
{
    return !m_caloMassComp ? true : m_caloMassComp->getValidUncertainty(unc,jet,eInfo,CompScaleVar::Mass);
}

bool CombinedMassUncertaintyComponent::getValidUncertaintyTA(double& unc, const xAOD::Jet& jet, const xAOD::EventInfo& eInfo) const
{
    return !m_TAMassComp ? true : m_TAMassComp->getValidUncertainty(unc,jet,eInfo,CompScaleVar::Mass);
}

double CombinedMassUncertaintyComponent::readHistoFromParam(const xAOD::JetFourMom_t& jet4vec, const UncertaintyHistogram& histo, const CompParametrization::TypeEnum param, const double massShiftFactor) const
{
    double resolution = 0;
    switch (param)
    {
        case CompParametrization::Pt:
            resolution = histo.getValue(jet4vec.Pt()*m_energyScale);
            break;
        case CompParametrization::PtEta:
            resolution = histo.getValue(jet4vec.Pt()*m_energyScale,jet4vec.Eta());
            break;
        case CompParametrization::PtAbsMass:
            resolution = histo.getValue(jet4vec.Pt()*m_energyScale,jet4vec.M()*m_energyScale*massShiftFactor);
            break;
        case CompParametrization::PtAbsEta:
            resolution = histo.getValue(jet4vec.Pt()*m_energyScale,fabs(jet4vec.Eta()));
            break;
        case CompParametrization::PtMass:
            resolution = histo.getValue(jet4vec.Pt()*m_energyScale,jet4vec.M()*massShiftFactor/jet4vec.Pt());
            break;
        case CompParametrization::PtMassEta:
            resolution = histo.getValue(jet4vec.Pt()*m_energyScale,jet4vec.M()*massShiftFactor/jet4vec.Pt(),jet4vec.Eta());
            break;
        case CompParametrization::PtMassAbsEta:
            resolution = histo.getValue(jet4vec.Pt()*m_energyScale,jet4vec.M()*massShiftFactor/jet4vec.Pt(),fabs(jet4vec.Eta()));
            break;
        case CompParametrization::PtAbsMassEta:
            resolution = histo.getValue(jet4vec.Pt()*m_energyScale,jet4vec.M()*m_energyScale*massShiftFactor,jet4vec.Eta());
            break;
        case CompParametrization::PtAbsMassAbsEta:
            resolution = histo.getValue(jet4vec.Pt()*m_energyScale,jet4vec.M()*m_energyScale*massShiftFactor,fabs(jet4vec.Eta()));
            break;
        case CompParametrization::eLOGmOe:
            resolution = histo.getValue(jet4vec.E()*m_energyScale,log(jet4vec.M()*massShiftFactor/jet4vec.E()));
            break;
        case CompParametrization::eLOGmOeEta:
            resolution = histo.getValue(jet4vec.E()*m_energyScale,log(jet4vec.M()*massShiftFactor/jet4vec.E()),jet4vec.Eta());
            break;
        case CompParametrization::eLOGmOeAbsEta:
            resolution = histo.getValue(jet4vec.E()*m_energyScale,log(jet4vec.M()*massShiftFactor/jet4vec.E()),fabs(jet4vec.Eta()));
            break;
        default:
            ATH_MSG_ERROR("Failed to read histogram due to unknown parametrization type in " << getName());
            break;
    }
    return resolution == 0 ? 0 : 1./(resolution*resolution);
}

double CombinedMassUncertaintyComponent::getWeightFactorCalo(const xAOD::Jet& jet, const double massShiftFactor) const
{
    if(m_caloMassScale_weights(jet).M() < 0.0) return 0;
    if(!m_caloMassWeight) return 0;
    return readHistoFromParam(m_caloMassScale_weights(jet),*m_caloMassWeight,m_weightParam,massShiftFactor);
}

double CombinedMassUncertaintyComponent::getWeightFactorTA(const xAOD::Jet& jet, const double massShiftFactor) const
{
    if(m_TAMassScale_weights(jet).M() < 0.0) return 0;
    if(!m_TAMassWeight) return 0;
    return readHistoFromParam(m_TAMassScale_weights(jet),*m_TAMassWeight,m_weightParam,massShiftFactor);
}



//////////////////////////////////////////////////
//                                              //
//  Base class method overrides                 //
//                                              //
//////////////////////////////////////////////////

bool CombinedMassUncertaintyComponent::isAlwaysZero() const
{
    if (!m_isInit)
    {
        ATH_MSG_ERROR("Cannot call method before initialization, component: " << getName().Data());
        return false;
    }

    return (!m_caloMassComp || m_caloMassComp->isAlwaysZero()) && (!m_TAMassComp || m_TAMassComp->isAlwaysZero());
}




} // end jet namespace
