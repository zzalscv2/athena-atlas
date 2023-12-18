/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file ElectronCombinedMaterialEffects.cxx
 * @begin         Friday 11th January 2005
 * @author        Anthony Morley, Tom  Atkinson, Christos Anastopoulos
 * @brief         Implementation code for ElectronCombinedMaterialEffects class
 */

#include "TrkGaussianSumFilterUtils/ElectronCombinedMaterialEffects.h"
//
#include "PathResolver/PathResolver.h"
#include "TrkEventPrimitives/ParamDefs.h"
#include "TrkExUtils/MaterialInteraction.h"
#include "TrkMaterialOnTrack/EnergyLoss.h"
#include "TrkParameters/TrackParameters.h"
//
#include <cmath>
#include <exception>
#include <fstream>
#include <sstream>

namespace {
constexpr double s_singleGaussianRange = 0.0001;
constexpr double s_lowerRange = 0.002;
constexpr double s_xOverRange = 0.10;
constexpr double s_upperRange = 0.20;
constexpr double s_componentMeanCut = 0.0;

/**
 * use Horner's method to evaluate Polynomials
 * We unroll up to order 5
 */
template<size_t N>
inline double
hornerEvaluate(const std::array<double, N>& a, const double& x)
{
  constexpr size_t order = N - 1;
  if constexpr (order == 0) {
    return a[0];
  } else if constexpr (order == 1) {
    return a[0] * x + a[1];
  } else if constexpr (order == 2) {
    return (a[0] * x + a[1]) * x + a[2];
  } else if constexpr (order == 3) {
    return ((a[0] * x + a[1]) * x + a[2]) * x + a[3];
  } else if constexpr (order == 4) {
    return (((a[0] * x + a[1]) * x + a[2]) * x + a[3]) * x + a[4];
  } else if constexpr (order == 5) {
    return ((((a[0] * x + a[1]) * x + a[2]) * x + a[3]) * x + a[4]) * x + a[5];
  } else {  // start from the last one we have
    double result =
        ((((a[0] * x + a[1]) * x + a[2]) * x + a[3]) * x + a[4]) * x + a[5];
    for (size_t i = 6; i < N; ++i) {
      result = result * x + a[i];
    }
    return result;
  }
}
inline bool
inRange(int var, int lo, int hi)
{
  return ((var <= hi) and (var >= lo));
}

// Logistic function - needed for transformation of weight and mean
inline double
logisticFunction(const double x)
{
  return 1. / (1. + std::exp(-x));
}

using BH = Trk::ElectronCombinedMaterialEffects;
// Correct weights of components
void
correctWeights(BH::MixtureParameters& mixture, const int numberOfComponents)
{
  if (numberOfComponents < 1) {
    return;
  }
  // Obtain the sum of weights
  double weightSum(0.);
  for (int i = 0; i < numberOfComponents; ++i) {
    weightSum += mixture[i].weight;
  }
  double norm = 1. / weightSum;
  // Rescale so that total weighting is 1
  for (int i = 0; i < numberOfComponents; ++i) {
    mixture[i].weight *= norm;
  }
}

BH::MixtureParameters
getTransformedMixtureParameters(
  const std::array<BH::Polynomial, GSFConstants::maxNumberofMatComponents>&
    polynomialWeights,
  const std::array<BH::Polynomial, GSFConstants::maxNumberofMatComponents>&
    polynomialMeans,
  const std::array<BH::Polynomial, GSFConstants::maxNumberofMatComponents>&
    polynomialVariances,
  const double pathlengthInX0,
  const int numberOfComponents)
{
  BH::MixtureParameters mixture{};
  for (int i = 0; i < numberOfComponents; ++i) {
    const double updatedWeight =
      hornerEvaluate(polynomialWeights[i], pathlengthInX0);
    const double updatedMean =
      hornerEvaluate(polynomialMeans[i], pathlengthInX0);
    const double updatedVariance =
      hornerEvaluate(polynomialVariances[i], pathlengthInX0);
    mixture[i] = { logisticFunction(updatedWeight),
                   logisticFunction(updatedMean),
                   std::exp(updatedVariance) };
  }
  return mixture;
}

BH::MixtureParameters
getMixtureParameters(
  const std::array<BH::Polynomial, GSFConstants::maxNumberofMatComponents>&
    polynomialWeights,
  const std::array<BH::Polynomial, GSFConstants::maxNumberofMatComponents>&
    polynomialMeans,
  const std::array<BH::Polynomial, GSFConstants::maxNumberofMatComponents>&
    polynomialVariances,
  const double pathlengthInX0,
  const int numberOfComponents)
{
  BH::MixtureParameters mixture{};
  for (int i = 0; i < numberOfComponents; ++i) {
    const double updatedWeight =
      hornerEvaluate(polynomialWeights[i], pathlengthInX0);
    const double updatedMean =
      hornerEvaluate(polynomialMeans[i], pathlengthInX0);
    const double updatedVariance =
      hornerEvaluate(polynomialVariances[i], pathlengthInX0);
    mixture[i] = { updatedWeight,
                   updatedMean,
                   updatedVariance * updatedVariance };
  }
  return mixture;
}

inline void
scattering(GsfMaterial::Scattering& cache,
           const Trk::ComponentParameters& componentParameters,
           const Trk::MaterialProperties& materialProperties,
           double pathLength)
{
  // Reset the cache
  cache.reset();

  // Request track parameters from component parameters
  const Trk::TrackParameters* trackParameters = componentParameters.params.get();
  const AmgSymMatrix(5)* measuredTrackCov = trackParameters->covariance();

  if (!measuredTrackCov) {
    return;
  }

  const Amg::Vector3D& globalMomentum = trackParameters->momentum();
  const double p = globalMomentum.mag();
  double pathcorrection = 1.;
  if (materialProperties.thickness() != 0) {
    pathcorrection = pathLength / materialProperties.thickness();
  }
  const double t = pathcorrection * materialProperties.thicknessInX0();
  constexpr double m = Trk::ParticleMasses::mass[Trk::electron];
  const double E = sqrt(p * p + m * m);
  const double beta = p / E;
  const double sigma = Trk::MaterialInteraction::sigmaMS(t, p, beta);
  const double angularVariation = sigma * sigma;

  const double sinTheta = std::sin(trackParameters->parameters()[Trk::theta]);
  cache.deltaThetaCov = angularVariation;
  cache.deltaPhiCov = angularVariation / (sinTheta * sinTheta);
}

// Helper to read in polynomials
Trk::ElectronCombinedMaterialEffects::Polynomial
readPolynomial(std::ifstream& fin)
{
  Trk::ElectronCombinedMaterialEffects::Polynomial poly{};
  for (size_t i = 0; i < GSFConstants::polynomialCoefficients; ++i) {
    if (!fin) {
      throw std::logic_error("Reached end of stream but still expecting data.");
    }
    fin >> poly[i];
  }
  return poly;
}

} //  end of anonymous namespace

// ElectronCombinedMaterialEffects methods
Trk::ElectronCombinedMaterialEffects::ElectronCombinedMaterialEffects(
  const std::string& parameterisationFileName,
  const std::string& parameterisationFileNameHighX0)
{
  // The following is a bit repetitive code
  // we could consider refactoring
  // The low X0 polynomials
  {
    std::string resolvedFileName =
      PathResolver::find_file(parameterisationFileName, "DATAPATH");
    if (resolvedFileName.empty()) {
      std::ostringstream ss;
      ss << "Parameterisation file : " << parameterisationFileName
         << " not found";
      throw std::logic_error(ss.str());
    }

    const char* filename = resolvedFileName.c_str();
    std::ifstream fin(filename);
    if (fin.bad()) {
      std::ostringstream ss;
      ss << "Error opening file: " << resolvedFileName;
      throw std::logic_error(ss.str());
    }

    fin >> m_BHnumberOfComponents;
    int orderPolynomial = 0;
    fin >> orderPolynomial;
    fin >> m_BHtransformationCode;
    if (not inRange(
          m_BHnumberOfComponents, 0, GSFConstants::maxNumberofMatComponents)) {
      std::ostringstream ss;
      ss << "numberOfComponents Parameter out of range 0- "
         << GSFConstants::maxNumberofMatComponents << " : "
         << m_BHnumberOfComponents;
      throw std::logic_error(ss.str());
    }
    if (orderPolynomial != (GSFConstants::polynomialCoefficients - 1)) {
      std::ostringstream ss;
      ss << "orderPolynomial  order !=  "
         << (GSFConstants::polynomialCoefficients - 1);
      throw std::logic_error(ss.str());
    }
    if (not inRange(m_BHtransformationCode, 0, 1)) {
      std::ostringstream ss;
      ss << "transformationCode Parameter out of range 0-1: "
         << m_BHtransformationCode;
      throw std::logic_error(ss.str());
    }
    if (!fin) {
      std::ostringstream ss;
      ss << "Error while reading file : " << resolvedFileName;
      throw std::logic_error(ss.str());
    }
    // Fill the polynomials
    int componentIndex = 0;
    for (; componentIndex < m_BHnumberOfComponents; ++componentIndex) {
      m_BHpolynomialWeights[componentIndex] = readPolynomial(fin);
      m_BHpolynomialMeans[componentIndex] = readPolynomial(fin);
      m_BHpolynomialVariances[componentIndex] = readPolynomial(fin);
    }
  }
  // Read the high X0 polynomials
  {
    std::string resolvedFileName =
      PathResolver::find_file(parameterisationFileNameHighX0, "DATAPATH");
    if (resolvedFileName.empty()) {
      std::ostringstream ss;
      ss << "Parameterisation file : " << parameterisationFileNameHighX0
         << " not found";
      throw std::logic_error(ss.str());
    }

    const char* filename = resolvedFileName.c_str();
    std::ifstream fin(filename);
    if (fin.bad()) {
      std::ostringstream ss;
      ss << "Error opening file: " << resolvedFileName;
      throw std::logic_error(ss.str());
    }
    fin >> m_BHnumberOfComponentsHighX0;
    int orderPolynomial = 0;
    fin >> orderPolynomial;
    fin >> m_BHtransformationCodeHighX0;
    //
    if (not inRange(m_BHnumberOfComponentsHighX0,
                    0,
                    GSFConstants::maxNumberofMatComponents)) {
      std::ostringstream ss;
      ss << "numberOfComponentsHighX0 Parameter out of range 0- "
         << GSFConstants::maxNumberofMatComponents << " : "
         << m_BHnumberOfComponentsHighX0;
      throw std::logic_error(ss.str());
    }
    if (m_BHnumberOfComponentsHighX0 != m_BHnumberOfComponents) {
      std::ostringstream ss;
      ss << " numberOfComponentsHighX0 != numberOfComponents";
      throw std::logic_error(ss.str());
    }
    if (orderPolynomial != (GSFConstants::polynomialCoefficients - 1)) {
      std::ostringstream ss;
      ss << "orderPolynomial  order !=  "
         << (GSFConstants::polynomialCoefficients - 1);
      throw std::logic_error(ss.str());
    }
    if (not inRange(m_BHtransformationCodeHighX0, 0, 1)) {
      std::ostringstream ss;
      ss << "transformationCode Parameter out of range "
            "0-1: "
         << m_BHtransformationCodeHighX0;
      throw std::logic_error(ss.str());
    }

    // Fill the polynomials
    int componentIndex = 0;
    for (; componentIndex < m_BHnumberOfComponentsHighX0; ++componentIndex) {
      m_BHpolynomialWeightsHighX0[componentIndex] = readPolynomial(fin);
      m_BHpolynomialMeansHighX0[componentIndex] = readPolynomial(fin);
      m_BHpolynomialVariancesHighX0[componentIndex] = readPolynomial(fin);
    }
  }
}

void
Trk::ElectronCombinedMaterialEffects::compute(
  GsfMaterial::Combined& cache,
  const Trk::ComponentParameters& componentParameters,
  const Trk::MaterialProperties& materialProperties,
  double pathLength,
  Trk::PropDirection direction) const
{
  const AmgSymMatrix(5)* measuredCov = componentParameters.params->covariance();
  /*
   * 1.  Retrieve multiple scattering corrections
   */
  GsfMaterial::Scattering cache_multipleScatter;
  scattering(
    cache_multipleScatter, componentParameters, materialProperties, pathLength);
  /*
   * 2. Retrieve energy loss corrections
   */
  GsfMaterial::EnergyLoss cache_energyLoss;
  this->BetheHeitler(cache_energyLoss,
                     componentParameters,
                     materialProperties,
                     pathLength,
                     direction);
  // Protect if there are no new energy loss
  // components
  // we want at least on dummy to "combine"
  // with scattering
  if (cache_energyLoss.numElements == 0) {
    cache_energyLoss.elements[0] = { 1, 0, 0 };
    cache_energyLoss.numElements = 1;
  }
  /*
   * 3. Combine the multiple scattering with each of the  energy loss components
   */
  // Cache is to be filled so 0 entries here
  cache.numEntries = 0;
  for (int i = 0; i < cache_energyLoss.numElements; ++i) {
    double combinedWeight = cache_energyLoss.elements[i].weight;
    double combinedDeltaP = cache_energyLoss.elements[i].deltaP;
    cache.weights[i] = combinedWeight;
    cache.deltaPs[i] = combinedDeltaP;
    if (measuredCov) {
      // Create the covariance
      const double covPhi = cache_multipleScatter.deltaPhiCov;
      const double covTheta = cache_multipleScatter.deltaThetaCov;
      const double covQoverP = cache_energyLoss.elements[i].deltaQOvePCov;
      cache.deltaCovariances[i] << 0, 0, 0, 0, 0, // 5
        0, 0, 0, 0, 0,                            // 10
        0, 0, covPhi, 0, 0,                       // 15
        0, 0, 0, covTheta, 0,                     // 20
        0, 0, 0, 0, covQoverP;
    } else {
      cache.deltaCovariances[i].setZero();
    }
    ++cache.numEntries;
  } // end for loop over energy loss components
}

/*
 * Use polynomials to get the BetheHeitler
 * energy loss
 */
void
Trk::ElectronCombinedMaterialEffects::BetheHeitler(
  GsfMaterial::EnergyLoss& cache,
  const Trk::ComponentParameters& componentParameters,
  const Trk::MaterialProperties& materialProperties,
  double pathLength,
  Trk::PropDirection direction) const
{
  cache.numElements = 0;

  const Trk::TrackParameters* trackParameters = componentParameters.params.get();
  const Amg::Vector3D& globalMomentum = trackParameters->momentum();

  const double radiationLength = materialProperties.x0();
  const double momentum = globalMomentum.mag();
  double pathlengthInX0 = pathLength / radiationLength;

  if (pathlengthInX0 < s_singleGaussianRange) {
    cache.elements[0] = { 1., 0., 0. };
    cache.numElements = 1;
    return;
  }

  // If the amount of material is between 0.0001 and 0.01 return the gaussian
  // approximation to the Bethe-Heitler distribution
  if (pathlengthInX0 < s_lowerRange) {
    const double meanZ = std::exp(-1. * pathlengthInX0);
    const double sign = (direction == Trk::oppositeMomentum) ? 1. : -1.;
    const double varZ =
      std::exp(-1. * pathlengthInX0 * std::log(3.) / std::log(2.)) -
      std::exp(-2. * pathlengthInX0);
    double deltaP(0.);
    double varQoverP(0.);
    if (direction == Trk::alongMomentum) {
      deltaP = sign * momentum * (1. - meanZ);
      varQoverP = 1. / (meanZ * meanZ * momentum * momentum) * varZ;
    } else {
      deltaP = sign * momentum * (1. / meanZ - 1.);
      varQoverP = varZ / (momentum * momentum);
    }
    cache.elements[0] = { 1., deltaP, varQoverP };
    cache.numElements = 1;
    return;
  }

  // Now we do the full calculation
  if (pathlengthInX0 > s_upperRange) {
    pathlengthInX0 = s_upperRange;
  }

  // Get proper mixture parameters
  MixtureParameters mixture;
  if (pathlengthInX0 > s_xOverRange) {
    if (m_BHtransformationCodeHighX0) {
      mixture = getTransformedMixtureParameters(m_BHpolynomialWeightsHighX0,
                                               m_BHpolynomialMeansHighX0,
                                               m_BHpolynomialVariancesHighX0,
                                               pathlengthInX0,
                                               m_BHnumberOfComponents);
    } else {
      mixture = getMixtureParameters(m_BHpolynomialWeightsHighX0,
                                     m_BHpolynomialMeansHighX0,
                                     m_BHpolynomialVariancesHighX0,
                                     pathlengthInX0,
                                     m_BHnumberOfComponents);
    }
  } else {
    if (m_BHtransformationCode) {
      mixture = getTransformedMixtureParameters(m_BHpolynomialWeights,
                                               m_BHpolynomialMeans,
                                               m_BHpolynomialVariances,
                                               pathlengthInX0,
                                               m_BHnumberOfComponents);
    } else {
      mixture = getMixtureParameters(m_BHpolynomialWeights,
                                     m_BHpolynomialMeans,
                                     m_BHpolynomialVariances,
                                     pathlengthInX0,
                                     m_BHnumberOfComponents);
    }
  }
  // Correct the mixture
  correctWeights(mixture, m_BHnumberOfComponents);
  int componentIndex = 0;
  double weightToBeRemoved(0.);
  int componentWithHighestMean(0);
  for (; componentIndex < m_BHnumberOfComponents; ++componentIndex) {
    if (mixture[componentIndex].mean > mixture[componentWithHighestMean].mean) {
      componentWithHighestMean = componentIndex;
    }
    if (mixture[componentIndex].mean >= s_componentMeanCut) {
      continue;
    }
    weightToBeRemoved += mixture[componentIndex].weight;
  }
  // Fill the cache to be returned
  componentIndex = 0;
  for (; componentIndex < m_BHnumberOfComponents; ++componentIndex) {
    double varianceInverseMomentum = 0;
    // This is not mathematically correct but it does stabilize the GSF
    if (mixture[componentIndex].mean < s_componentMeanCut) {
      continue;
    }
    double weight = mixture[componentIndex].weight;
    if (componentIndex == componentWithHighestMean) {
      weight += weightToBeRemoved;
    }
    double deltaP(0.);
    if (direction == alongMomentum) {
      // For forward propagation
      deltaP = momentum * (mixture[componentIndex].mean - 1.);
      const double f = 1. / (momentum * mixture[componentIndex].mean);
      varianceInverseMomentum = f * f * mixture[componentIndex].variance;
    } // end forward propagation if clause
    else {
      // For backwards propagation
      deltaP = momentum * (1. / mixture[componentIndex].mean - 1.);
      varianceInverseMomentum =
        mixture[componentIndex].variance / (momentum * momentum);
    } // end backwards propagation if clause

    // set in the cache and increase the elements
    cache.elements[cache.numElements] = { weight,
                                          deltaP,
                                          varianceInverseMomentum };
    ++cache.numElements;
  } // end for loop over all components
}

