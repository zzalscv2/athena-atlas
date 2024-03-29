/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#define APReweight2D_cxx
#include "ReweightUtils/APReweight2D.h"
#include "ReweightUtils/APWeightEntry.h"
#include "ReweightUtils/MathTools.h"
#include <iostream>
#include "TTree.h"
#include "TH2.h"

using namespace std;

APReweight2D::APReweight2D() :
  APReweightBase(),
  m_denominator_hist(0),
  m_numerator_hist(0),
  m_n_bins_x(0),
  m_n_bins_y(0),
  m_axis_x(0),
  m_axis_y(0)
{
  m_isQuiet = false;
}

APReweight2D::APReweight2D(TH2* denominator_in, TH2* numerator_in, bool isTrig) : APReweightBase() {
  m_empty_weight = new APWeightEntry(0, 0, 1.);
  m_denominator_hist = (TH2D*) denominator_in->Clone("");
  m_numerator_hist = (TH2D*) numerator_in->Clone("");
  m_axis_x = (TAxis*) m_denominator_hist->GetXaxis()->Clone("");
  m_axis_y = (TAxis*) m_denominator_hist->GetYaxis()->Clone("");
  m_n_bins_x = m_denominator_hist->GetNbinsX();
  m_n_bins_y = m_denominator_hist->GetNbinsY();
  m_scale = (double) denominator_in->GetEntries() / (double) numerator_in->GetEntries();
  m_isTrig = isTrig;
  m_isQuiet = false;
  for (unsigned int i = 0; i < m_n_bins_x; ++i) {
    m_weights.push_back(vector<APWeightEntry*>());
    for (unsigned int j = 0; j < m_n_bins_y; ++j) {
      APWeightEntry* temp_entry = new APWeightEntry((unsigned int) m_denominator_hist->GetBinContent(i + 1, j + 1), (unsigned int) m_numerator_hist->GetBinContent(i + 1, j + 1), m_scale, m_isTrig);
      vector<int> temp_vec(2,0);
      vector<int> temp_vec_axes(2,0);
      temp_vec[0] = i+1;
      temp_vec[1] = j+1;
      temp_vec_axes[0] = m_n_bins_x;
      temp_vec_axes[1] = m_n_bins_y;
      temp_entry->SetCoordinates(temp_vec,temp_vec_axes);
      temp_entry->SetID(m_ID);
      m_weights[i].push_back(temp_entry);
    }
  }
    
  if( m_isTrig ) {
    for( int i = 1; i < m_numerator_hist->GetNbinsX()*m_numerator_hist->GetNbinsY(); ++i ) {
      if( m_numerator_hist->GetBinContent(i) > m_denominator_hist->GetBinContent(i) ) {
        std::cout << "WARNING in APReweight2D::~APReweight2D(TH2* denominator_in, TH2* numerator_in, bool isTrig) : Using histograms " << m_numerator_hist->GetName() << " and " << m_denominator_hist->GetName() << " the efficiency is larger than 1 for bin " << i << "! This is inconsistent and can lead to unwanted behaviour (weights > 1, variance < 0 )! Please check your input histograms. In order to avoid negative variances, the efficiency in this bin will be set to 0." << std::endl;
        m_numerator_hist->SetBinContent(i,0);
      }
    }
  }


}

void APReweight2D::ReadEfficiency(TH2* efficiency_in, TH2* err_low_in, TH2* err_high_in) {
  if (err_high_in == 0) err_high_in = err_low_in;
  m_empty_weight = new APWeightEntry(0, 0, 1.);
  m_denominator_hist = new TH2D("", "", 1, 0., 1., 1, 0., 1.);
  m_numerator_hist = new TH2D("", "", 1, 0., 1., 1, 0., 1.);
  m_axis_x = (TAxis*) efficiency_in->GetXaxis()->Clone("");
  m_axis_y = (TAxis*) efficiency_in->GetYaxis()->Clone("");
  m_n_bins_x = efficiency_in->GetNbinsX();
  m_n_bins_y = efficiency_in->GetNbinsY();
  m_scale = 1.0;
  m_isTrig = true;
  for (unsigned int i = 0; i < m_n_bins_x; ++i) {
    m_weights.push_back(vector<APWeightEntry*>());
    for (unsigned int j = 0; j < m_n_bins_y; ++j) {
      APWeightEntry *temp_entry = new APWeightEntry();
      temp_entry->ReadEfficiency(efficiency_in->GetBinContent(i + 1, j + 1), err_low_in->GetBinContent(i + 1, j + 1), err_high_in->GetBinContent(i + 1, j + 1));
      vector<int> temp_vec(2,0);
      vector<int> temp_vec_axes(2,0);
      temp_vec[0] = i+1;
      temp_vec[1] = j+1;
      temp_vec_axes[0] = m_n_bins_x;
      temp_vec_axes[1] = m_n_bins_y;
      temp_entry->SetCoordinates(temp_vec,temp_vec_axes);
      temp_entry->SetID(m_ID);
      m_weights[i].push_back(temp_entry);
    }
  }
}

APReweight2D::~APReweight2D() {
  delete m_denominator_hist;
  delete m_numerator_hist;
  delete m_axis_x;
  delete m_axis_y;
  delete m_empty_weight;
  for (unsigned int i = 0; i < m_n_bins_x; ++i) {
    for (vector<APWeightEntry*>::reverse_iterator it = m_weights[i].rbegin(); it != m_weights[i].rend(); ++it) {
      delete *it;
    }
  }
  m_weights.clear();
}

APWeightEntry* APReweight2D::GetBinWeight(unsigned int bin_x, unsigned int bin_y) const {
  if (bin_x == 0 || bin_y == 0) return m_empty_weight;
  return m_weights[bin_x - 1][bin_y - 1];
}

APWeightEntry* APReweight2D::GetWeight(double value_x, double value_y) const {
  return GetBinWeight(GetBinX(value_x), GetBinY(value_y));
}

const TH2D* APReweight2D::GetDenominatorHist() const {
  return m_denominator_hist;
}

const TH2D* APReweight2D::GetNumeratorHist() const {
  return m_numerator_hist;
}

double APReweight2D::GetSampleScale() const {
  return m_scale;
}

unsigned int APReweight2D::NBins() const {
  return m_n_bins_x*m_n_bins_y;
}

unsigned int APReweight2D::GetBinX(double value) const {
  for (unsigned int i = 1; i <= m_n_bins_x; ++i) {
    if (value >= m_axis_x->GetBinLowEdge(i) && value < m_axis_x->GetBinUpEdge(i)) {
      return i;
    }
  }
  if (!m_isQuiet) cout << "WARNING in APReweight2D::GetBinX: Value out of range! Returning 0." << endl;
  return 0;
}

unsigned int APReweight2D::GetBinY(double value) const {
  for (unsigned int i = 1; i <= m_n_bins_y; ++i) {
    if (value >= m_axis_y->GetBinLowEdge(i) && value < m_axis_y->GetBinUpEdge(i)) {
      return i;
    }
  }
  if (!m_isQuiet) cout << "WARNING in APReweight2D::GetBinY: Value out of range! Returning 0." << endl;
  return 0;
}

void APReweight2D::SetSystUncert(double rel_uncert) {
  for (unsigned int i = 0; i < m_n_bins_x; ++i) {
    for (unsigned int j = 0; j < m_n_bins_y; ++j) {
      GetBinWeight(i,j)->SetSystUncert(rel_uncert);
    }
  }
}

void APReweight2D::SetQuietMode(bool isQuiet) {
  m_isQuiet = isQuiet;
}
