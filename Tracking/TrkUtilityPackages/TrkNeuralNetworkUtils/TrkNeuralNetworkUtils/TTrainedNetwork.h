/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//-*-c++-*-
#ifndef TRKNEURALNETWORKUTILS_TTRAINEDNETWORK_H
#define TRKNEURALNETWORKUTILS_TTRAINEDNETWORK_H

#include "TObject.h"
#include "TMatrixD.h"
#include "TVectorD.h"
#include <math.h>
#include <vector>
#include <string> 
#include <map>

/******************************************************
  @class TTrainedNetwork
  Created : 18-02-2008
  @author Giacinto Piacquadio (giacinto.piacquadio AT physik.uni-freiburg.de)
********************************************************/

namespace nnopt { 
}

class TTrainedNetwork : public TObject
{
public:

  static const unsigned linearOutput = 1u << 0; 
  static const unsigned normalizeOutput = 1u << 1; 

  enum ActivationFunction { 
    SIGMOID = 1
  }; 

  struct Input { 
    std::string name; //<- requires unique strings or none at all
    double offset; //<- this value is added to the input before giving to nn
    double scale; //<- after offset is added, input is scaled by this value 
  }; 

  typedef std::vector<Double_t> DVec; 
  typedef std::map<std::string, double> DMap; 
  typedef DMap::const_iterator DMapI; 

  TTrainedNetwork();

  //NOTE: class takes ownership of all pointers.

  // old-school constructor (for compatability)
  TTrainedNetwork(Int_t nInput, 
		 Int_t nHidden, 
		 Int_t nOutput,
		 std::vector<Int_t> & nHiddenLayerSize, 
		 std::vector<TVectorD*> & thresholdVectors,
		 std::vector<TMatrixD*> & weightMatrices,
		 Int_t activationFunction,
		 bool linearOutput = false,
		 bool normalizeOutput = false); 

  // You can also add offsets and scales. 
  // This is intended as a workaround for older code that didn't include 
  // normalization. 
  void setOffsets(const std::vector<double>& offsets); 
  void setScales(const std::vector<double>& scales); 

  // new constructor for normalized nn. 
  // This avoids some chances for logical inconsistency by constructing 
  // the hidden layer size from the thresholdVectors and weightMatrices. 
  // Also runs a consistency check on thresholdVectors and weightMatrices. 
  TTrainedNetwork(const std::vector<TTrainedNetwork::Input>& inputs,
		 unsigned nOutput,
		 std::vector<TVectorD*> & thresholdVectors,
		 std::vector<TMatrixD*> & weightMatrices,
		 ActivationFunction activationFunction = SIGMOID,
		 unsigned options = 0);

  ~TTrainedNetwork();

  // returns an empty vector if normalization isn't set
  std::vector<Input> getInputs() const; 

  void setNewWeights(std::vector<TVectorD*> & thresholdVectors,
		     std::vector<TMatrixD*> & weightMatrices);

  Int_t getnInput() const {return m_nInput;};

  Int_t getnHidden() const {return m_nHidden;};

  Int_t getnOutput() const {return m_nOutput;};

  const std::vector<Int_t> &  getnHiddenLayerSize() const {
    return m_nHiddenLayerSize;
  };

  Int_t getActivationFunction() const {return m_ActivationFunction;};

  const std::vector<TVectorD*> & getThresholdVectors() const {
    return m_ThresholdVectors;
  };

  const std::vector<TMatrixD*> & weightMatrices() const {
    return m_WeightMatrices;
  };

  // these methods should be optimized. 
  DVec calculateOutputValues(const DVec & input) const;
  DVec calculateNormalized(const DVec& input) const;

  // these are intentionally slow: the NN must 
  // rearrange the inputs each time these functions are called. 
  // They are designed for robustness and ease of use, not for highly
  // optimized code. 
  DVec calculateNormalized(const DMap & input) const;

  bool getIfLinearOutput() const { return m_LinearOutput; };

  bool getIfNormalizeOutput() const { return m_NormalizeOutput; };

private:


  unsigned m_nInput;
  unsigned m_nHidden;
  unsigned m_nOutput;

  // in an ideal world these would be one object in a vector, but 
  // storing classes within classes in root is ugly
  std::vector<Double_t> m_input_node_offset; 
  std::vector<Double_t> m_input_node_scale; 

  std::map<std::string,int> m_inputStringToNode; 

  std::vector<Int_t> m_nHiddenLayerSize;

  std::vector<TVectorD*> m_ThresholdVectors;
  std::vector<TMatrixD*> m_WeightMatrices;

  unsigned int   m_bufferSizeMax;              //! cache of the maximum needed size, not persisitified

  Int_t m_ActivationFunction;

  bool m_LinearOutput;

  bool m_NormalizeOutput;

  double m_maxExpValue;

  Double_t sigmoid(Double_t x) const; 

  // assertions to check for nonsense initialization 
  bool is_consistent() const; 
  bool check_norm_size(unsigned size) const; 


  ClassDef( TTrainedNetwork, 3 )
  
};

#endif
