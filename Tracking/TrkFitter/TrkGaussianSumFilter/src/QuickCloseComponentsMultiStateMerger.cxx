/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/*********************************************************************************
      QuickCloseComponentsMultiStateMerger.cxx  -  description
      ---------------------------------------------------
begin                : Wednesday 3rd September 2008
author               : amorley
email                : Anthony.Morley@cern.ch
decription           : Implementation code for QuickCloseComponentsMultiStateMerger
                       class
*********************************************************************************/

#include "TrkGaussianSumFilter/QuickCloseComponentsMultiStateMerger.h"
#include "TrkGaussianSumFilter/IMultiComponentStateCombiner.h"
#include "TrkGaussianSumFilter/KLGaussianMixtureReduction.h"
#include "TrkGaussianSumFilter/AllignedDynArray.h"
#include "TrkParameters/TrackParameters.h"
#include "GaudiKernel/Chrono.h"
#include <limits>

using namespace KLGaussianMixtureReduction; 
using namespace GSFUtils; 

Trk::QuickCloseComponentsMultiStateMerger::QuickCloseComponentsMultiStateMerger(const std::string& type,
                                                                                const std::string& name,
                                                                                const IInterface* parent)
  : AthAlgTool(type, name, parent)
  , m_chronoSvc("ChronoStatSvc", name)
{

  declareInterface<IMultiComponentStateMerger>(this);
}

Trk::QuickCloseComponentsMultiStateMerger::~QuickCloseComponentsMultiStateMerger() {}

StatusCode
Trk::QuickCloseComponentsMultiStateMerger::initialize()
{
  ATH_MSG_VERBOSE("Initialising Close Components Merger");

  // Request the Chrono Service
  if (m_chronoSvc.retrieve().isFailure()) {
    ATH_MSG_FATAL("Failed to retrieve service " << m_chronoSvc);
    return StatusCode::FAILURE;
  } else
    ATH_MSG_INFO("Retrieved service " << m_chronoSvc);

  // Request an instance of the MultiComponentStateCombiner
  if (m_stateCombiner.retrieve().isFailure()) {
    ATH_MSG_FATAL("Could not retrieve an instance of the multi-component state combiner... Exiting!");
    return StatusCode::FAILURE;
  }

  // Request an instance of the MultiComponentStateAssembler
  if (m_stateAssembler.retrieve().isFailure()) {
    ATH_MSG_FATAL("Could not retrieve an instance of the mutli-component state assembler... Exiting!");
    return StatusCode::FAILURE;
  }

  if (m_maximumNumberOfComponents <= 0) {
    ATH_MSG_FATAL("Attempting to merge multi-state into zero components... stop being silly!");
    return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("Initialisation of " << type() << " under instance " << name() << " was successful");
  return StatusCode::SUCCESS;
}

StatusCode
Trk::QuickCloseComponentsMultiStateMerger::finalize()
{

  ATH_MSG_INFO("Finalisation of " << type() << " under instance " << name() << " was successful");

  return StatusCode::SUCCESS;
}

std::unique_ptr<Trk::MultiComponentState>
Trk::QuickCloseComponentsMultiStateMerger::merge(const Trk::MultiComponentState& unmergedState) const
{

  ATH_MSG_VERBOSE("Merging state with " << unmergedState.size() << " components");
  // Assembler Cache
  IMultiComponentStateAssembler::Cache cache;
  // MAke sure  the assembler is reset
  m_stateAssembler->reset(cache);

  if (unmergedState.size() <= m_maximumNumberOfComponents) {
    ATH_MSG_VERBOSE("State is already sufficiently small... no component reduction required");
    return std::unique_ptr<Trk::MultiComponentState> (unmergedState.clone());
  }


  if (unmergedState.empty()) {
    ATH_MSG_ERROR("Attempting to merge multi-state with zero components");
    return nullptr;
  }

  bool componentWithoutMeasurement = false;

  Trk::MultiComponentState::const_iterator component = unmergedState.begin();

  // Scan all components for covariance matrices. If one or more component
  // is missing an error matrix, component reduction is impossible.
  for (; component != unmergedState.end(); ++component) {
    const AmgSymMatrix(5)* measuredCov = component->first->covariance();
    if (!measuredCov) {
      componentWithoutMeasurement = true;
      break;
    }
  }

  if (componentWithoutMeasurement) {
    ATH_MSG_DEBUG("A track parameters object is without measurement... reducing state to single component");
    const Trk::ComponentParameters reducedState(unmergedState.begin()->first->clone(), 1.);
    return std::make_unique<Trk::MultiComponentState>(reducedState);
  }
  
  const size_t n = unmergedState.size();
// Create an array of all components to be merged
  SimpleMultiComponentState statesToMerge(n);
  component = unmergedState.begin();

  size_t ii = 0;
  for (; component != unmergedState.end(); ++component) {
    // Fill in infomation
    statesToMerge[ii].first = std::unique_ptr<Trk::TrackParameters>(component->first->clone());
    statesToMerge[ii].second = component->second;
    ++ii;
  }

  if(ii==0)
    return 0;

  return mergeFullDistArray(cache, statesToMerge);
}



std::unique_ptr<Trk::MultiComponentState>
Trk::QuickCloseComponentsMultiStateMerger::merge( Trk::SimpleMultiComponentState&& statesToMerge ) const
{

  ATH_MSG_VERBOSE("Merging state with " << statesToMerge.size() << " components");
  // Assembler Cache
  IMultiComponentStateAssembler::Cache cache;
  // MAke sure  the assembler is reset
  m_stateAssembler->reset(cache);

  if (statesToMerge.size() <= m_maximumNumberOfComponents) {
    ATH_MSG_VERBOSE("State is already sufficiently small... no component reduction required");
    m_stateAssembler->addMultiState( cache, std::move(statesToMerge) );
    return m_stateAssembler->assembledState(cache);
  }


  if (statesToMerge.empty()) {
    ATH_MSG_ERROR("Attempting to merge multi-state with zero components");
    return nullptr;
  }

  bool componentWithoutMeasurement = false;

  Trk::SimpleMultiComponentState::const_iterator component = statesToMerge.cbegin();

  // Scan all components for covariance matrices. If one or more component
  // is missing an error matrix, component reduction is impossible.
  for (; component != statesToMerge.cend(); ++component) {
    const AmgSymMatrix(5)* measuredCov = component->first->covariance();
    if (!measuredCov) {
      componentWithoutMeasurement = true;
      break;
    }
  }

  if (componentWithoutMeasurement) {
    ATH_MSG_DEBUG("A track parameters object is without measurement... reducing state to single component");
    // Sort to select the one with the largest weight
    std::sort(statesToMerge.begin(),statesToMerge.end(), [](const SimpleComponentParameters& x, const SimpleComponentParameters& y){return x.second > y.second;});
    const Trk::ComponentParameters reducedState(statesToMerge.begin()->first->clone(), 1.);
    return std::make_unique<Trk::MultiComponentState>(reducedState);
  }
  

  return mergeFullDistArray(cache, statesToMerge);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<Trk::MultiComponentState>
Trk::QuickCloseComponentsMultiStateMerger::mergeFullDistArray(IMultiComponentStateAssembler::Cache& cache,
                                                              SimpleMultiComponentState& statesToMerge ) const 
{

  const int n = statesToMerge.size();
  const int nn2 = (n + 1) * n / 2;

  AlignedDynArray<float,alignment> distances(nn2); // Array to store all of the distances between components
  AlignedDynArray<int,alignment> indexToI(nn2);    // The i  & J of each distances so that i don't have to calculate them
  AlignedDynArray<int,alignment> indexToJ(nn2);
  AlignedDynArray<float,alignment> qonp(n);    // Array of qonp for each component
  AlignedDynArray<float,alignment> qonpCov(n); // Array of Cov(qonp,qonp) for each component
  AlignedDynArray<float,alignment> qonpG(n);   // Array of 1/Cov(qonp,qonp) for each component

  // Initlise all values
  for (int i = 0; i < n; ++i) {
    qonp[i] = 0;
    qonpCov[i] = 0;
    qonpG[i] = 1e10;
  }

  for (int i = 0; i < nn2; ++i) {
    distances[i] = std::numeric_limits<float>::max();
    indexToI[i] = -1;
    indexToJ[i] = -1;
  }

  // Calculate indicies
  for (int i = 0; i < n; ++i) {
    int indexConst = (i + 1) * i / 2;
    for (int j = 0; j <= i; ++j) {
      int index = indexConst + j;
      indexToI[index] = i;
      indexToJ[index] = j;
    }
  }

  // Create an array of all components to be merged
  for (int ii(0); ii < n; ++ii) {
    const AmgSymMatrix(5)* measuredCov = statesToMerge[ii].first->covariance();
    const Amg::VectorX& parameters = statesToMerge[ii].first->parameters();

    // Fill in infomation
    qonp[ii] = parameters[Trk::qOverP];
    qonpCov[ii] = measuredCov ? (*measuredCov)(Trk::qOverP, Trk::qOverP): -1.;
    qonpG[ii] = qonpCov[ii] > 0 ? 1. / qonpCov[ii] : 1e10;
  }

 
  // Calculate distances for all pairs
  // This loop can be vectorised
  calculateAllDistances(qonp, qonpCov, qonpG, distances, n);

  // Loop over all components until you reach the target amount
  unsigned int numberOfComponents = n;
  int minIndex = -1;
  int nextMinIndex = -1;

  while (numberOfComponents > m_maximumNumberOfComponents) {

    // Searching for the minimum distances is slowest part of the loop
    // lets try and speed it up by reducing the number searches by 2
    if (nextMinIndex < 1) {
      // Run search again
      std::pair<int, int> min = findMinimumIndex(distances, nn2);
      minIndex = min.first;
      nextMinIndex = min.second;
    } else {
      // Use new min index
      minIndex = nextMinIndex;
      nextMinIndex = -1;
    }

    int mini = indexToI[minIndex];
    int minj = indexToJ[minIndex];

    if (mini == minj) {
      ATH_MSG_ERROR("Err keys are equal key1 " << mini << " key2 " << minj);
      break;
    }

    // merge components
    ATH_MSG_VERBOSE("key1 " << mini << " key2 " << minj);
    /*
     * Combine the components to be merged
     * statesToMerge[mini] becomes the merged
     * statesToMerge[minj] is set to dummy values
     */
    m_stateCombiner->combineWithWeight(statesToMerge[mini], statesToMerge[minj]);
    ATH_MSG_VERBOSE("Weight of new component " << statesToMerge[mini].second);
    statesToMerge[minj].first.reset();
    statesToMerge[minj].second = 0.;

    /*
     * set relevant distances
     */
    const AmgSymMatrix(5)* measuredCov = statesToMerge[mini].first->covariance();
    const AmgVector(5)& parameters = statesToMerge[mini].first->parameters();
    qonp[mini] = parameters[Trk::qOverP];
    qonpCov[mini] = (*measuredCov)(Trk::qOverP, Trk::qOverP);
    qonpG[mini] = qonpCov[mini] > 0 ? 1. / qonpCov[mini] : 1e10;
    qonp[minj] = 0.;
    qonpCov[minj] = 0.;
    qonpG[minj] = 1e10;

    // Reset old weights;
    resetDistances(distances, mini, n);

    // If that element has been removed already the next min pair is invalid and can't be used
    if (nextMinIndex > 0 && (mini == indexToI[nextMinIndex] || minj == indexToJ[nextMinIndex] ||
                             minj == indexToI[nextMinIndex] || mini == indexToJ[nextMinIndex])) {
      nextMinIndex = -1;
    }

    // Calculate weights for new component
    int possibleNextMin = recalculateDistances(qonp, qonpCov, qonpG, distances, mini, n);
    if (nextMinIndex > 0 && possibleNextMin > 0 && distances[possibleNextMin] < distances[nextMinIndex]) {
      nextMinIndex = possibleNextMin;
    }
    // Reset old weights;
    resetDistances(distances, minj, n);
    // Decrement the number of components
    --numberOfComponents;
  } // end of merge while

  // Build final state containing both merged and unmerged components
  for (auto& state : statesToMerge) {
    // Avoid merge ones
    if (!state.first) {
      continue;
    }
    /*
     * Add componets to the state be prepared for assembly
     * and update the relevant weight
     */
    cache.multiComponentState.push_back(SimpleComponentParameters(state.first.release(), state.second));
    cache.validWeightSum += state.second;
  }
  std::unique_ptr<Trk::MultiComponentState> mergedState = m_stateAssembler->assembledState(cache);
  ATH_MSG_DEBUG("Number of components in merged state: " << mergedState->size());

  // Clear the state vector
  statesToMerge.clear();

  return mergedState;
}


