// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//

#ifndef CALORECGPU_TOOLS_PLOTTERAUXDEFINES_H
#define CALORECGPU_TOOLS_PLOTTERAUXDEFINES_H

#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "CaloRecGPU/Helpers.h"

#include <cmath>
#include <iostream>
#include <fstream>
#include <type_traits>
#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include <map>
#include <memory>
#include <sstream>
#include <numeric>
#include <iomanip>
#include <functional>

#include "CxxUtils/checker_macros.h"

#ifdef ATLAS_CHECK_THREAD_SAFETY

  ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#endif

#include <TROOT.h>    // the TROOT object is the entry point to the ROOT system
#include <TH1F.h>
#include <TCanvas.h>  // graphics canvas
#include <TFile.h>
#include <TStyle.h>
#include <TPaveStats.h>
#include <TH2F.h>
#include <THStack.h>
#include <TLatex.h>
#include <TLine.h>
#include <TLegend.h>
#include <TPave.h>
#include <TPad.h>
#include <TMarker.h>
#include <TMultiGraph.h>
#include <TGraphErrors.h>



inline bool sortCaloCellRatio(const int & cell1Tag, const int & cell2Tag)
{
  return cell1Tag > cell2Tag;
}


static constexpr const char * regNames[3] = {"central", "end-cap", "forward"};
static constexpr float regCuts[3] = {1.5, 3.2, 5};

static constexpr float SNR_thresholds[3] = {0., 2., 4.};

template <class T>
constexpr T pi = CaloRecGPU::Helpers::Constants::pi<T>;

template <class T>
T wrapPhi( const T phi )
{
  return phi;
  /*
  const T comp = phi / pi<T>;
  T ret = std::fmod(comp + 1, 2);
  if (ret < 0)
    {
      ret += 2;
    }
    return (ret - 1) * pi<T>;*/
}

template <class T1, class T2>
decltype(std::declval<T1>() - std::declval<T2>()) minDiffPhi(const T1 phi1, const T2 phi2)
{
  using T = decltype(phi1 - phi2);

  const T phi_r1 = wrapPhi<T>(phi1);
  const T phi_r2 = wrapPhi<T>(phi2);

  return wrapPhi<T>(phi_r1 - phi_r2);
}

template <class Tb, class Ta>
inline Tb proxim_ath(const Tb b, const Ta a)
{
  const Ta aplus = a + pi<Ta>;
  const Ta aminus = a - pi<Ta>;
  Tb ret = b;
  if (b > aplus)
    {
      do
        {
          ret -= 2 * pi<Tb>;
        }
      while (ret > aplus);
    }
  else if (b < aminus)
    {
      do
        {
          ret += 2 * pi<Tb>;
        }
      while (ret < aminus);
    }
  return ret;
}

template <class Obj, class F, class ... Args>
void operate_on_regions (Obj * arr, const float eta, F && f, Args && ... args)
//f must take an Obj * that points to the object in which to operate.
{
  f(arr, std::forward<Args>(args)...);
  const float abs_eta = std::abs(eta);
  if (abs_eta < regCuts[0])
    {
      f(arr + 1, std::forward<Args>(args)...);
    }
  else if (abs_eta < regCuts[1])
    {
      f(arr + 2, std::forward<Args>(args)...);
    }
  else if (abs_eta < regCuts[2])
    {
      f(arr + 3, std::forward<Args>(args)...);
    }
  else
    {
      //std::cout << "ERROR: " << abs_eta << " is outside expected eta parameters!" << std::endl;
    }
}

template <class Obj, class F, class ... Args>
void operate_on_types (Obj * arr, const float SNR, F && f, Args && ... args)
//f must take an Obj * that points to the object in which to operate.
{
  f(arr, std::forward<Args>(args)...);
  if (SNR > SNR_thresholds[2])
    {
      f(arr + 1, std::forward<Args>(args)...);
    }
  else if (SNR > SNR_thresholds[1])
    {
      f(arr + 2, std::forward<Args>(args)...);
    }
  if (SNR > SNR_thresholds[0])
    {
      f(arr + 3, std::forward<Args>(args)...);
    }
}

inline int get_legacy_tag(const CaloRecGPU::tag_type tag)
{
  if (CaloRecGPU::Tags::is_part_of_cluster(tag))
    {
      return CaloRecGPU::Tags::get_index_from_tag(tag);
    }
  else
    {
      return -1;
    }
}

struct ClusterData;

/*!
  Put a variable to its most positive possible value.
*/
template <class T> void set_to_highest(T && val)
{
  if constexpr (std::is_arithmetic_v< std::decay_t<T> >)
    {
      val = std::numeric_limits< std::decay_t<T> >::max();
    }
  else if constexpr (std::is_same_v< std::decay_t<T>, ClusterData >)
    {
      val.set_all_to_highest();
    }
  else
    {
      //Bleep.
    }
}

/*!
  Put a variable to its most negative possible value.
*/
template <class T> void set_to_lowest(T && val)
{
  if constexpr (std::is_arithmetic_v< std::decay_t<T> >)
    {
      val = std::numeric_limits< std::decay_t<T> >::lowest();
    }
  else if constexpr (std::is_same_v< std::decay_t<T>, ClusterData >)
    {
      val.set_all_to_lowest();
    }
  else
    {
      //Bleep.
    }
}

struct ClusterData
{
  int num_cells{};
  float energy{};
  float transverse_energy{};
  float eta{};
  float phi{};
  double abs_energy{};
  double eta_post{};
  double phi_post{};
  double energy_post{};

  ClusterData(const int n_c = 0, const float en = 0, const float e_t = 0, const float h = 0, const float f = 0, const double abs_e = 0,
              const double h_post = 0, const double f_post = 0, const double en_post = 0):
    num_cells(n_c), energy(en), transverse_energy(e_t), eta(h), phi(f), abs_energy(abs_e),
    eta_post(h_post), phi_post(f_post), energy_post(en_post)
  {
  }

  void set_all_to_highest()
  {
    set_to_highest(num_cells);
    set_to_highest(energy);
    set_to_highest(transverse_energy);
    set_to_highest(eta);
    set_to_highest(phi);
    set_to_highest(abs_energy);
    set_to_highest(eta_post);
    set_to_highest(phi_post);
    set_to_highest(energy_post);
  }

  void set_all_to_lowest()
  {
    set_to_lowest(num_cells);
    set_to_lowest(energy);
    set_to_lowest(transverse_energy);
    set_to_lowest(eta);
    set_to_lowest(phi);
    set_to_lowest(abs_energy);
    set_to_lowest(eta_post);
    set_to_lowest(phi_post);
    set_to_lowest(energy_post);
  }

  void set(const int n_num, const float n_ene, const float n_et, const float n_eta, const float n_phi, const double abs_e)
  {
    num_cells = n_num;
    energy = n_ene;
    transverse_energy = n_et;
    eta = n_eta;
    phi = n_phi;
    abs_energy = abs_e;
  }

  void set(const int n_num, const float n_ene, const float n_et, const float n_eta, const float n_phi)
  {
    num_cells = n_num;
    energy = n_ene;
    transverse_energy = n_et;
    eta = n_eta;
    phi = n_phi;
  }

  void set_to_min(const int n_num, const float n_ene, const float n_et,
                  const float n_eta, const float n_phi, const double n_abs_e,
                  const double n_eta_post, const double n_phi_post, const double n_ene_post)
  {
    num_cells = std::min(num_cells, n_num);
    energy = std::min(energy, n_ene);
    transverse_energy = std::min(transverse_energy, n_et);
    eta = std::min(eta, n_eta);
    phi = std::min(phi, n_phi);
    abs_energy = std::min(abs_energy, n_abs_e);
    eta_post = std::min(eta_post, n_eta_post);
    phi_post = std::min(phi_post, n_phi_post);
    energy_post = std::min(energy_post, n_ene_post);
  }


  void set_to_max(const int n_num, const float n_ene, const float n_et,
                  const float n_eta, const float n_phi, const double n_abs_e,
                  const double n_eta_post, const double n_phi_post, const double n_ene_post)
  {
    num_cells = std::max(num_cells, n_num);
    energy = std::max(energy, n_ene);
    transverse_energy = std::max(transverse_energy, n_et);
    eta = std::max(eta, n_eta);
    phi = std::max(phi, n_phi);
    abs_energy = std::max(abs_energy, n_abs_e);
    eta_post = std::max(eta_post, n_eta_post);
    phi_post = std::max(phi_post, n_phi_post);
    energy_post = std::max(energy_post, n_ene_post);
  }

  void set_to_min(const int n_num, const float n_ene, const float n_et, const float n_eta, const float n_phi, const double n_abs_e)
  {
    num_cells = std::min(num_cells, n_num);
    energy = std::min(energy, n_ene);
    transverse_energy = std::min(transverse_energy, n_et);
    eta = std::min(eta, n_eta);
    phi = std::min(phi, n_phi);
    abs_energy = std::min(abs_energy, n_abs_e);
  }

  void set_to_max(const int n_num, const float n_ene, const float n_et, const float n_eta, const float n_phi, const double n_abs_e)
  {
    num_cells = std::max(num_cells, n_num);
    energy = std::max(energy, n_ene);
    transverse_energy = std::max(transverse_energy, n_et);
    eta = std::max(eta, n_eta);
    phi = std::max(phi, n_phi);
    abs_energy = std::max(abs_energy, n_abs_e);
  }

  void set_to_min(const ClusterData & other)
  {
    set_to_min(other.num_cells, other.energy, other.transverse_energy, other.eta,
               other.phi, other.abs_energy, other.eta_post, other.phi_post, other.energy_post);
  }

  void set_to_max(const ClusterData & other)
  {
    set_to_max(other.num_cells, other.energy, other.transverse_energy, other.eta,
               other.phi, other.abs_energy, other.eta_post, other.phi_post, other.energy_post);
  }

  double delta_R(const double other_eta, const double other_phi) const
  {
    const double delta_eta = (eta - other_eta);
    const double delta_phi = minDiffPhi(phi, other_phi);

    return std::sqrt(delta_eta * delta_eta + delta_phi * delta_phi);
  }

  double delta_R(const ClusterData & other) const
  {
    return delta_R(other.eta, other.phi);
  }

  double delta_R_post(const double other_eta, const double other_phi) const
  {
    const double delta_eta = (eta_post - other_eta);
    const double delta_phi = minDiffPhi(phi_post, other_phi);

    return std::sqrt(delta_eta * delta_eta + delta_phi * delta_phi);
  }

  double delta_R_post(const ClusterData & other) const
  {
    return delta_R_post(other.eta_post, other.phi_post);
  }

  double delta_R_post() const
  {
    return delta_R(eta_post, phi_post);
  }

};


struct BasePlotter
{
  int num_bins = 250;

  int canvas_x = 800, canvas_y = 600;

  std::string plotter_name{"Results"};
  std::string ref_name{"CPU"}, test_name{"GPU"};
  std::string label_type_1{"Seed"}, label_type_2{"Growing"}, label_type_3{"Terminal"};
  std::string suffix_type_1{"seed"}, suffix_type_2{"grow"}, suffix_type_3{"term"};
  std::string label_region_1{"Center"}, label_region_2{"End-Cap"}, label_region_3{"Forward"};
  std::string suffix_region_1{"center"}, suffix_region_2{"endcap"}, suffix_region_3{"forward"};
  std::vector<std::string> file_extensions = {std::string("pdf"), std::string("eps"), std::string("png")};
  std::vector<std::string> print_options = {std::string("EmbedFonts"), std::string(), std::string()};
  //This is only valid for PDF files!
  //If file_extension is changed, this should be too!

  virtual size_t num() const
  {
    return 1;
  }

  enum class StyleKinds
  {
    ref = 0, test, joined, Number
  };

  template <class T> struct PlotKinds
  {
    T ref;
    T test;
    T joined;
  };

  struct ElementStyle_t
  {
    Color_t color;
    Style_t style;
    float alpha;
    int size;
  };

  struct GraphStyle_t
  {
    ElementStyle_t line, fill, marker;
  };

  std::array<GraphStyle_t, int(StyleKinds::Number)>
  Styles{ GraphStyle_t{ ElementStyle_t{kBlue, 1, 1.0f, 1}, ElementStyle_t{kBlue,  0/*3004*/, 1.0f, 1}, ElementStyle_t{kBlue,  20, 1.0f, 1} },   //Ref
            GraphStyle_t{ ElementStyle_t{kRed, 1, 1.0f, 1}, ElementStyle_t{kRed,  0/*3005*/, 1.0f, 1}, ElementStyle_t{kRed,  21, 1.0f, 1} },   //Test
            GraphStyle_t{ ElementStyle_t{1, 1, 1.0f, 1}, ElementStyle_t{kGray + 1, 0, 1.0f, 1}, ElementStyle_t{kGray + 1,  22, 1.0f, 1} } }; //Joined
  // Line,         Fill
  // (c, s, a)      (c, s, a)
  //So Styles[<kind>].<element>.<style>
  //(e. g. Styles[StyleKinds::test].fill.color)

  inline static bool place_title = false;
  inline static bool yaxis_bins = true;
  inline static std::string plot_label = "";
  inline static bool place_ATLAS_label = true;

  static std::string stringify_pretty_number (const double num)
  {
    std::stringstream sstr;
    sstr << std::setprecision(2) << std::fixed << num;
    //std::cout << sstr.str() << std::endl;
    return sstr.str();
  }

  static std::string approximate_string(const double val)
  {
    if (val >= 1e-2 && val <= 1e4)
      {
        return stringify_pretty_number(std::ceil(val * 1e3) * 1e-3);
      }
    else
      {
        double log = std::floor(std::log10(val));
        return stringify_pretty_number(std::ceil(val * pow(10, 3 - log)) * 1e-3) + " #times 10^{" + std::to_string(int(log)) + "}";
      }
  }

  template <class Plot>
  static std::string get_bin_label(Plot * gr)
  {
    const double width = gr->GetXaxis()->GetBinWidth(1);
    std::string_view xtext = gr->GetXaxis()->GetTitle();
    const size_t pos_l = xtext.find_last_of('[');
    const size_t pos_r = xtext.find_last_of(']');
    if (pos_l == std::string_view::npos || pos_r == std::string_view::npos || pos_r < pos_l)
      {
        return approximate_string(width);
      }
    else
      {
        return approximate_string(width) + " " + std::string(xtext.substr(pos_l + 1, pos_r - pos_l - 1));
      }
  }

  static void ATLASLabel(Double_t x, Double_t y)
  {
    TLatex l; //l.SetTextAlign(12); l.SetTextSize(tsize);
    l.SetNDC();
    l.SetTextFont(72);
    l.SetTextColor(1);

    double delx = 0.115 * 696 * gPad->GetWh() / (472 * gPad->GetWw());

    l.DrawLatex(x, y, "ATLAS");
    if (plot_label != "")
      {
        TLatex p;
        p.SetNDC();
        p.SetTextFont(42);
        p.SetTextColor(1);
        p.DrawLatex(x + delx, y, plot_label.c_str());
        //    p.DrawLatex(x,y,"#sqrt{s}=900GeV");
      }
  }

  inline static double labelstart_x = 0.625;
  inline static double labelstart_y = 0.625;
  //Well, legend, but...
  inline static double labelsize_x = 0.25;
  inline static double labelsize_y = 0.25;

  inline static double labeloffset_x = -0.225;
  inline static double labeloffset_y = labelsize_y + 0.01;
  //These control the position of the "ATLAS ..." label
  //in relation to the plot legend.

  inline static double extraspacefactor_add = 0.;
  inline static double extraspacefactor_multiply = 2.5;

  inline static bool normalize = false;

  inline static std::string extra_text = "#sqrt{s} = 13 TeV";

  inline static double extralabeloffset_x = 0.075;
  inline static double extralabeloffset_y = -0.04;

  static void extra_text_label(Double_t x, Double_t y)
  {
    TLatex l; //l.SetTextAlign(12); l.SetTextSize(tsize);
    l.SetNDC();
    l.SetTextColor(1);
    l.SetTextSize(0.04);

    l.DrawLatex(x, y, extra_text.c_str());
  }

  static void plot_one(TCanvas * cv, TH1 * gr, const std::string & file,
                       const std::vector<std::string> & exts,  const std::vector<std::string> & options)
  {
    cv->cd();
    cv->Clear();
    if (place_ATLAS_label)
      {
        gStyle->SetOptStat(0000);
        gStyle->SetOptTitle(place_title);

        if (yaxis_bins)
          {
            std::string title = gr->GetYaxis()->GetTitle();
            title += " / " + get_bin_label(gr);
            gr->GetYaxis()->SetTitle(title.c_str());
          }
        gr->SetMaximum(gr->GetMaximum() * extraspacefactor_multiply + extraspacefactor_add);
        gr->Draw("");
        ATLASLabel(labelstart_x + labeloffset_x, labelstart_y + labeloffset_y);
        extra_text_label(labelstart_x + extralabeloffset_x, labelstart_y + extralabeloffset_y);
      }
    else
      {
        gStyle->SetOptStat(2200);
        gStyle->SetOptTitle(place_title);
        gr->Draw("");
      }
    cv->SetLogy(1);
    for (size_t i = 0; i < exts.size(); ++i)
      {
        cv->SaveAs((file + "." + exts[i]).c_str(), options[i].c_str());
      }
  }

  static void plot_one(TCanvas * cv, TH2 * gr, const std::string & file,
                       const std::vector<std::string> & exts,  const std::vector<std::string> & options)
  {
    cv->cd();
    cv->Clear();
    if (place_ATLAS_label)
      {
        gStyle->SetOptStat(0000);
      }
    else
      {
        gStyle->SetOptStat(2200);
      }
    gStyle->SetOptTitle(place_title);
    gr->Draw("colsz");
    cv->SetLogy(0);
    cv->SetLogz(1);
    for (size_t i = 0; i < exts.size(); ++i)
      {
        cv->SaveAs((file + "." + exts[i]).c_str(), options[i].c_str());
      }
  }

  static void plot_one(TCanvas * cv, THStack * hs, const std::string & file,
                       const std::vector<std::string> & exts,  const std::vector<std::string> & options)
  {
    cv->cd();
    cv->Clear();
    if (place_ATLAS_label)
      {
        gStyle->SetOptStat(0000);
      }
    else
      {
        gStyle->SetOptStat(2200);
      }
    gStyle->SetOptTitle(place_title);
    std::vector <Style_t> styles(hs->GetNhists());
    TIter iter(hs->GetHists());
    iter.Begin();
    for (int i = 0; iter != iter.End(); iter(), ++i)
      {
        TH1F * hist = (TH1F *) *iter;
        styles[i] = hist->GetLineStyle();
        hist->SetLineStyle(0);
        if (place_ATLAS_label)
          {
            if (yaxis_bins)
              {
                std::string title = hist->GetYaxis()->GetTitle();
                title += " / " + get_bin_label(hist);
                hist->GetYaxis()->SetTitle(title.c_str());
              }
            hist->SetMaximum(hist->GetMaximum() * extraspacefactor_multiply + extraspacefactor_add);
          }
      }
    hs->Draw("nostack");
    if (place_ATLAS_label)
      {
        if (yaxis_bins)
          {
            std::string title = hs->GetYaxis()->GetTitle();
            title += " / " + get_bin_label(hs);
            hs->GetYaxis()->SetTitle(title.c_str());
          }
        TLegend * leg = gPad->BuildLegend(labelstart_x, labelstart_y, labelstart_x + labelsize_x, labelstart_y + labelsize_y, "", "l");
        if (place_ATLAS_label)
          {
            ATLASLabel(labelstart_x + labeloffset_x, labelstart_y + labeloffset_y);
            extra_text_label(labelstart_x + extralabeloffset_x, labelstart_y + extralabeloffset_y);
          }

        leg->SetBorderSize(0);
        leg->SetFillColor(0);
        leg->SetTextFont(42);
        leg->SetTextSize(0.0275);
      }
    else
      {
        gPad->BuildLegend(0.75, 0.75, 0.95, 0.95, "");
      }
    iter.Begin();
    for (int i = 0; iter != iter.End(); iter(), ++i)
      {
        TH1F * hist = (TH1F *) *iter;
        hist->SetLineStyle(styles[i]);
        styles[i] = hist->GetFillStyle();
        hist->SetFillStyle(0);
      }
    hs->Draw("nostack same");
    iter.Begin();
    for (int i = 0; iter != iter.End(); iter(), ++i)
      {
        TH1F * hist = (TH1F *) *iter;
        hist->SetFillStyle(styles[i]);
      }
    /*
      //This is an attempt for plot overlap to work
      //without transparency/patterns.
      //It kinda does, but it isn't pretty...
    std::vector<std::unique_ptr<TH1F>> temps;
    TIter iter(hs->GetHists());
    for (iter.Begin(); iter != iter.End(); iter() )
      {
    temps.emplace_back(std::make_unique<TH1F>(* ((TH1F *) *iter)));
    temps.back()->SetLineColorAlpha(0, 0.);
    //temps.back()->Rebin(, "", );
    for(int i = 1; i <= temps.back()->GetNbinsX(); ++i)
    {
      double min = temps.back()->GetBinContent(i);
      TIter other = iter;
      for (other.Begin(); other != iter; other())
        {
    TH1F * otherhistptr = (TH1F *) *other;
    min = std::min(min, otherhistptr->GetBinContent(i));
        }
      temps.back()->SetBinContent(i, min);
      //We could just set to 0,
      //but I'll keep like this
      //so we can check what happens
      //if we drop SetLineColorAlpha(0,0.)
    }
    temps.back()->Draw("same");
    }
    */
    gPad->Modified();
    gPad->Update();
    cv->Update();
    cv->SetLogy(1);
    for (size_t i = 0; i < exts.size(); ++i)
      {
        cv->SaveAs((file + "." + exts[i]).c_str(), options[i].c_str());
      }
  }

  static void plot_one(TCanvas * cv, TGraphErrors * gr, const std::string & file,
                       const std::vector<std::string> & exts,  const std::vector<std::string> & options)
  {
    cv->cd();
    cv->Clear();
    if (place_ATLAS_label)
      {
        //gStyle->SetOptStat(0000);
        gStyle->SetOptTitle(place_title);

        gr->SetMaximum(gr->GetMaximum() * extraspacefactor_multiply + extraspacefactor_add);
        gr->Draw("ACP");
        ATLASLabel(labelstart_x + labeloffset_x, labelstart_y + labeloffset_y);
        extra_text_label(labelstart_x + extralabeloffset_x, labelstart_y + extralabeloffset_y);
      }
    else
      {
        //gStyle->SetOptStat(2200);
        gStyle->SetOptTitle(place_title);
        gr->Draw("ACP");
      }
    cv->SetLogy(0);
    for (size_t i = 0; i < exts.size(); ++i)
      {
        cv->SaveAs((file + "." + exts[i]).c_str(), options[i].c_str());
      }
  }

  static void plot_one(TCanvas * cv, TMultiGraph * mg, const std::string & file,
                       const std::vector<std::string> & exts,  const std::vector<std::string> & options)
  {
    cv->cd();
    cv->Clear();
    /*
    if (place_ATLAS_label)
      {
        gStyle->SetOptStat(0000);
      }
    else
      {
        gStyle->SetOptStat(2200);
      }
    */
    gStyle->SetOptTitle(place_title);
    mg->Draw("ACP");
    if (place_ATLAS_label)
      {
        if (yaxis_bins)
          {
            std::string title = mg->GetYaxis()->GetTitle();
            title += " / " + get_bin_label(mg);
            mg->GetYaxis()->SetTitle(title.c_str());
          }
        TLegend * leg = gPad->BuildLegend(labelstart_x, labelstart_y, labelstart_x + labelsize_x, labelstart_y + labelsize_y, "", "l");
        if (place_ATLAS_label)
          {
            ATLASLabel(labelstart_x + labeloffset_x, labelstart_y + labeloffset_y);
            extra_text_label(labelstart_x + extralabeloffset_x, labelstart_y + extralabeloffset_y);
          }

        leg->SetBorderSize(0);
        leg->SetFillColor(0);
        leg->SetTextFont(42);
        leg->SetTextSize(0.0275);
      }
    else
      {
        gPad->BuildLegend(0.75, 0.75, 0.95, 0.95, "");
      }
    gPad->Modified();
    gPad->Update();
    cv->Update();
    cv->SetLogy(0);
    for (size_t i = 0; i < exts.size(); ++i)
      {
        cv->SaveAs((file + "." + exts[i]).c_str(), options[i].c_str());
      }
  }

  struct hist_stacker
  {
    std::unique_ptr<THStack> global, t1, t2, t3;
    hist_stacker(const std::string & histname, const std::string & hist_title, const std::string & x_label,
                 const std::string & y_label, const std::string & t1_label, const std::string & t2_label,
                 const std::string & t3_label):
      global(new THStack((histname + "_g").c_str(), (hist_title + " ;" + x_label + " ;" + y_label).c_str())),
      t1(new THStack((histname + "_t1").c_str(), (hist_title + " (" + t1_label + ") ;" + x_label + " ;" + y_label).c_str())),
      t2(new THStack((histname + "_t2").c_str(), (hist_title + " (" + t2_label + ") ;" + x_label + " ;" + y_label).c_str())),
      t3(new THStack((histname + "_t3").c_str(), (hist_title + " (" + t3_label + ") ;" + x_label + " ;" + y_label).c_str()))
    {
    }

    hist_stacker(): global(nullptr), t1(nullptr), t2(nullptr), t3(nullptr)
    {
    }
  };

  struct graph_stacker
  {
    std::unique_ptr<TMultiGraph> global, t1, t2, t3;
    graph_stacker(const std::string & graphname, const std::string & hist_title, const std::string & x_label,
                  const std::string & y_label, const std::string & t1_label, const std::string & t2_label,
                  const std::string & t3_label):
      global(new TMultiGraph((graphname + "_g").c_str(), (hist_title + " ;" + x_label + " ;" + y_label).c_str())),
      t1(new TMultiGraph((graphname + "_t1").c_str(), (hist_title + " (" + t1_label + ") ;" + x_label + " ;" + y_label).c_str())),
      t2(new TMultiGraph((graphname + "_t2").c_str(), (hist_title + " (" + t2_label + ") ;" + x_label + " ;" + y_label).c_str())),
      t3(new TMultiGraph((graphname + "_t3").c_str(), (hist_title + " (" + t3_label + ") ;" + x_label + " ;" + y_label).c_str()))
    {
    }

    graph_stacker(): global(nullptr), t1(nullptr), t2(nullptr), t3(nullptr)
    {
    }
  };

  template <class T>
  struct hist_group
  {
    std::unique_ptr<T> global, t1, t2, t3;

    void set_style(const GraphStyle_t & gs)
    {
      global->SetLineColorAlpha(gs.line.color, gs.line.alpha);
      global->SetLineStyle(gs.line.style);
      global->SetFillColorAlpha(gs.fill.color, gs.fill.alpha);
      global->SetFillStyle(gs.fill.style);
      global->SetMarkerStyle(gs.marker.style);
      global->SetMarkerColorAlpha(gs.marker.color, gs.marker.alpha);
      global->SetMarkerSize(gs.marker.size);

      t1->SetLineColorAlpha(gs.line.color, gs.line.alpha);
      t1->SetLineStyle(gs.line.style);
      t1->SetFillColorAlpha(gs.fill.color, gs.fill.alpha);
      t1->SetFillStyle(gs.fill.style);
      t1->SetMarkerStyle(gs.marker.style);
      t1->SetMarkerColorAlpha(gs.marker.color, gs.marker.alpha);
      t1->SetMarkerSize(gs.marker.size);

      t2->SetLineColorAlpha(gs.line.color, gs.line.alpha);
      t2->SetLineStyle(gs.line.style);
      t2->SetFillColorAlpha(gs.fill.color, gs.fill.alpha);
      t2->SetFillStyle(gs.fill.style);
      t2->SetMarkerStyle(gs.marker.style);
      t2->SetMarkerColorAlpha(gs.marker.color, gs.marker.alpha);
      t2->SetMarkerSize(gs.marker.size);

      t3->SetLineColorAlpha(gs.line.color, gs.line.alpha);
      t3->SetLineStyle(gs.line.style);
      t3->SetFillColorAlpha(gs.fill.color, gs.fill.alpha);
      t3->SetFillStyle(gs.fill.style);
      t3->SetMarkerStyle(gs.marker.style);
      t3->SetMarkerColorAlpha(gs.marker.color, gs.marker.alpha);
      t3->SetMarkerSize(gs.marker.size);
    }

  };

  struct hist_group_1D : public hist_group<TH1F>
  {
    hist_group_1D(const std::array<double, 4> & min, const std::array<double, 4> & max, const int num_bins,
                  const std::string & histname, const std::string & hist_title, const std::string & x_label,
                  const std::string & y_label, const std::string & t1_label, const std::string & t2_label,
                  const std::string & t3_label)
    {
      global.reset(new TH1F( (histname + "_g").c_str(), (hist_title + " ;" + x_label + " ;" + y_label).c_str(),
                             num_bins, min[0], max[0]));
      t1.reset(new TH1F( (histname + "_t1").c_str(), (hist_title + " (" + t1_label + ") ;" + x_label + " ;" + y_label).c_str(),
                         num_bins, min[1], max[1]));
      t2.reset(new TH1F( (histname + "_t2").c_str(), (hist_title + " (" + t2_label + ") ;" + x_label + " ;" + y_label).c_str(),
                         num_bins, min[2], max[2]));
      t3.reset(new TH1F( (histname + "_t3").c_str(), (hist_title + " (" + t3_label + ") ;" + x_label + " ;" + y_label).c_str(),
                         num_bins, min[3], max[3]));
      /*
      std::cout << "Building " << histname << "\nRanges: ";
      for (int i = 0; i < 4; ++i)
      {
      std::cout << "(" << min[i] << " | " << max[i] << ")";
      }
      std::cout << std::endl;
      */
    }

    hist_group_1D(const std::array<double, 4> & min, const std::array<double, 4> & max, const int num_bins,
                  const std::string & histname, const std::string & hist_title)
    {
      global.reset(new TH1F( (histname + "_g").c_str(), hist_title.c_str(), num_bins, min[0], max[0]));
      t1.reset(new TH1F( (histname + "_t1").c_str(), hist_title.c_str(), num_bins, min[1], max[1]));
      t2.reset(new TH1F( (histname + "_t2").c_str(), hist_title.c_str(), num_bins, min[2], max[2]));
      t3.reset(new TH1F( (histname + "_t3").c_str(), hist_title.c_str(), num_bins, min[3], max[3]));
      /*
      std::cout << "Building " << histname << "\nRanges: ";
      for (int i = 0; i < 4; ++i)
      {
      std::cout << "(" << min[i] << " | " << max[i] << ")";
      }
      std::cout << std::endl;
      */
    }

    template <class F>
    void populate (F && f, const int cumulative, const size_t num_events)
    {
      f(this);
      if (cumulative != 0)
        {
          global->Sumw2(false);
          t1->Sumw2(false);
          t2->Sumw2(false);
          t3->Sumw2(false);
          std::unique_ptr<TH1F> a((TH1F *) global->GetCumulative(cumulative > 0));
          std::unique_ptr<TH1F> b((TH1F *) t1->GetCumulative(cumulative > 0));
          std::unique_ptr<TH1F> c((TH1F *) t2->GetCumulative(cumulative > 0));
          std::unique_ptr<TH1F> d((TH1F *) t3->GetCumulative(cumulative > 0));
          a->Scale(1. / global->GetEntries());
          b->Scale(1. / t1->GetEntries());
          c->Scale(1. / t2->GetEntries());
          d->Scale(1. / t3->GetEntries());
          a.swap(global);
          b.swap(t1);
          c.swap(t2);
          d.swap(t3);
          global->Sumw2(false);
          t1->Sumw2(false);
          t2->Sumw2(false);
          t3->Sumw2(false);
        }
      else if (normalize)
        {
          global->Scale(1. / num_events);
          t1->Scale(1. / num_events);
          t2->Scale(1. / num_events);
          t3->Scale(1. / num_events);
          global->Sumw2(false);
          t1->Sumw2(false);
          t2->Sumw2(false);
          t3->Sumw2(false);
        }
    }


    void add_to_stack(hist_stacker & stack)
    {
      stack.global->Add(global.get());
      stack.t1->Add(t1.get());
      stack.t2->Add(t2.get());
      stack.t3->Add(t3.get());
    }

  };

  struct hist_group_2D : public hist_group<TH2F>
  {
    hist_group_2D(const std::array<double, 4> & min_x, const std::array<double, 4> & max_x,
                  const std::array<double, 4> & min_y, const std::array<double, 4> & max_y, const int num_bins,
                  const std::string & histname, const std::string & hist_title, const std::string & x_label,
                  const std::string & y_label, const std::string & t1_label, const std::string & t2_label,
                  const std::string & t3_label)
    {
      global.reset(new TH2F( (histname + "_g").c_str(), (hist_title + " ;" + x_label + " ;" + y_label).c_str(),
                             num_bins, min_x[0], max_x[0], num_bins, min_y[0], max_y[0]));
      t1.reset(new TH2F( (histname + "_t1").c_str(), (hist_title + " (" + t1_label + ") ;" + x_label + " ;" + y_label).c_str(),
                         num_bins, min_x[1], max_x[1], num_bins, min_y[1], max_y[1]));
      t2.reset(new TH2F( (histname + "_t2").c_str(), (hist_title + " (" + t2_label + ") ;" + x_label + " ;" + y_label).c_str(),
                         num_bins, min_x[2], max_x[2], num_bins, min_y[2], max_y[2]));
      t3.reset(new TH2F( (histname + "_t3").c_str(), (hist_title + " (" + t3_label + ") ;" + x_label + " ;" + y_label).c_str(),
                         num_bins, min_x[2], max_x[3], num_bins, min_y[3], max_y[3]));
    }

    template <class F>
    void populate (F && f, const int /*ignore*/, const size_t /*also_ignore*/)
    {
      f(this);
    }

  };

  struct graph_group_1D : public hist_group<TGraphErrors>
  {
    std::array<double, 4> maxes, mines;

   private:

    static void construct_pretty(std::unique_ptr<TGraphErrors> & ptr,
                                 const std::string & graphname, const std::string & graph_title,
                                 const double min, const double max)
    {
      ptr.reset(new TGraphErrors());
      ptr->SetName(graphname.c_str());
      ptr->SetTitle(graph_title.c_str());
      ptr->SetMinimum(min);
      ptr->SetMaximum(max);
    }

   public:

    graph_group_1D(const std::array<double, 4> & min, const std::array<double, 4> & max, const int /*num_bins*/,
                   const std::string & graphname, const std::string & graph_title, const std::string & x_label,
                   const std::string & y_label, const std::string & t1_label, const std::string & t2_label,
                   const std::string & t3_label): maxes(max), mines(min)
    {
      construct_pretty(global, graphname + "_g", graph_title + " ;" + x_label + " ;" + y_label, min[0], max[0]);
      construct_pretty(t1, graphname + "_t1", graph_title + " (" + t1_label + ") ;" + x_label + " ;" + y_label, min[1], max[1]);
      construct_pretty(t2, graphname + "_t2", graph_title + " (" + t2_label + ") ;" + x_label + " ;" + y_label, min[2], max[2]);
      construct_pretty(t3, graphname + "_t3", graph_title + " (" + t3_label + ") ;" + x_label + " ;" + y_label, min[3], max[3]);
    }

    graph_group_1D(const std::array<double, 4> & min, const std::array<double, 4> & max, const int /*num_bins*/,
                   const std::string & graphname, const std::string & graph_title): maxes(max), mines(min)
    {
      construct_pretty(global, graphname + "_g", graph_title, min[0], max[0]);
      construct_pretty(t1, graphname + "_t1", graph_title, min[1], max[1]);
      construct_pretty(t2, graphname + "_t2", graph_title, min[2], max[2]);
      construct_pretty(t3, graphname + "_t3", graph_title, min[3], max[3]);
    }

    template <class F>
    void populate (F && f, const int /*ignore*/, const size_t /*also_ignore*/)
    {
      f(this);
      if (maxes[0] != mines[0])
      {
        global->GetYaxis()->SetLimits(mines[0], maxes[0]);
      }
    }


    void add_to_stack(graph_stacker & stack)
    {
      stack.global->Add(global.release());
      stack.t1->Add(t1.release());
      stack.t2->Add(t2.release());
      stack.t3->Add(t3.release());
    }
  };

  enum class PlotterKind
  {
    type = 0, region, time, type2D, region2D, graph, Number
  };

  struct plotter_base
  {
    std::string hist_name, hist_title, x_label, y_label;
    const BasePlotter * parent{};

    virtual PlotterKind plotter_kind() const = 0;

    plotter_base() = default;
    plotter_base(const plotter_base &) = default;
    plotter_base(plotter_base &&) = default;
    plotter_base & operator= (const plotter_base &) = default;
    plotter_base & operator= (plotter_base &&) = default;

    virtual ~plotter_base() {}

    virtual void plot(const std::string & path, const std::string & prefix = "", const std::string & suffix = "") const = 0;
    virtual void plot(const std::array<double, 4> & wanted_min_x, const std::array<double, 4> & wanted_max_x,
                      const std::string & path, const std::string & prefix = "", const std::string & suffix = "") const = 0;
    virtual void plot(const std::array<double, 4> & wanted_min_x, const std::array<double, 4> & wanted_max_x,
                      const std::array<double, 4> & wanted_min_y, const std::array<double, 4> & wanted_max_y,
                      const std::string & path, const std::string & prefix = "", const std::string & suffix = "") const = 0;

    virtual void plot(const double wanted_min_x, const double wanted_max_x,
                      const std::string & path, const std::string & prefix = "", const std::string & suffix = "") const
    {
      this->plot(std::array<double, 4> {wanted_min_x, wanted_min_x, wanted_min_x, wanted_min_x},
                 std::array<double, 4> {wanted_max_x, wanted_max_x, wanted_max_x, wanted_max_x},
                 path, prefix, suffix);
    }

    virtual void plot(const double wanted_min_x, const double wanted_max_x,
                      const double wanted_min_y, const double wanted_max_y,
                      const std::string & path, const std::string & prefix = "", const std::string & suffix = "") const
    {
      this->plot(std::array<double, 4> {wanted_min_x, wanted_min_x, wanted_min_x, wanted_min_x},
                 std::array<double, 4> {wanted_max_x, wanted_max_x, wanted_max_x, wanted_max_x},
                 std::array<double, 4> {wanted_min_y, wanted_min_y, wanted_min_y, wanted_min_y},
                 std::array<double, 4> {wanted_max_y, wanted_max_y, wanted_max_y, wanted_max_y},
                 path, prefix, suffix);
    }

    virtual void calc_data_range(std::array<double, 4> & /*min*/, std::array<double, 4> & /*max*/) const
    {
      std::cout << "ERROR: Should not be seeing this!" << std::endl;
    }

    virtual void calc_data_range(std::array<double, 4> & /*min*/, std::array<double, 4> & /*max*/,
                                 const std::array<double, 4> & /*wanted_min*/, const std::array<double, 4> & /*wanted_max*/) const
    {
      std::cout << "ERROR: Should not be seeing this!" << std::endl;
    }
  };

  template <class group_T>
  struct basic_plotter : public plotter_base
  {
    using plotter_base::plot;
    using plotter_base::calc_data_range;

    bool can_be_negative{};

    int cumulative{};
    //0 means no, < 0 means backward, > 0 means forward accumulation

    StyleKinds style_ref;
    std::function<void(group_T *)> populator;

    virtual void save(const std::string & base_name, const group_T & group) const = 0;

    virtual group_T create_group(const std::string & title_override = "") const
    {
      group_T ret = this->construct_group(title_override);
      ret.populate(populator, cumulative, parent->num());
      ret.set_style(parent->Styles[int(style_ref)]);
      return ret;
    }

    virtual group_T create_group(const std::array<double, 4> & wanted_min_x,
                                 const std::array<double, 4> & wanted_max_x,
                                 const std::array<double, 4> & wanted_min_y,
                                 const std::array<double, 4> & wanted_max_y,
                                 const std::string & title_override = "",
                                 const bool force_range = false) const
    {
      group_T ret = this->construct_group(wanted_min_x, wanted_max_x, wanted_min_y, wanted_max_y, title_override, force_range);
      ret.populate(populator, cumulative, parent->num());
      ret.set_style(parent->Styles[int(style_ref)]);
      return ret;
    }

    virtual group_T create_group(const std::array<double, 4> & wanted_min_x,
                                 const std::array<double, 4> & wanted_max_x,
                                 const std::string & title_override = "",
                                 const bool force_range = false) const
    {
      group_T ret = this->construct_group(wanted_min_x, wanted_max_x, title_override, force_range);
      ret.populate(populator, cumulative, parent->num());
      ret.set_style(parent->Styles[int(style_ref)]);
      return ret;
    }


    virtual group_T create_group(const double wanted_min_x, const double wanted_max_x,
                                 const double wanted_min_y, const double wanted_max_y,
                                 const std::string & title_override = "",
                                 const bool force_range = false) const
    {
      group_T ret = this->construct_group(wanted_min_x, wanted_max_x, wanted_min_y, wanted_max_y, title_override, force_range);
      ret.populate(populator, cumulative, parent->num());
      ret.set_style(parent->Styles[int(style_ref)]);
      return ret;
    }

    virtual group_T create_group(const double wanted_min_x, const double wanted_max_x,
                                 const std::string & title_override = "",
                                 const bool force_range = false) const
    {
      group_T ret = this->construct_group(wanted_min_x, wanted_max_x, title_override, force_range);
      ret.populate(populator, cumulative, parent->num());
      ret.set_style(parent->Styles[int(style_ref)]);
      return ret;
    }


    virtual group_T construct_group(const std::string & title_override = "") const = 0;

    virtual group_T construct_group(const std::array<double, 4> & wanted_min_x,
                                    const std::array<double, 4> & wanted_max_x,
                                    const std::array<double, 4> & wanted_min_y,
                                    const std::array<double, 4> & wanted_max_y,
                                    const std::string & title_override = "",
                                    const bool force_range = false) const = 0;

    virtual group_T construct_group(const std::array<double, 4> & wanted_min_x,
                                    const std::array<double, 4> & wanted_max_x,
                                    const std::string & title_override = "",
                                    const bool force_range = false) const = 0;


    virtual group_T construct_group(const double wanted_min_x, const double wanted_max_x,
                                    const double wanted_min_y, const double wanted_max_y,
                                    const std::string & title_override = "",
                                    const bool force_range = false) const
    {
      return this->construct_group(std::array<double, 4> {wanted_min_x, wanted_min_x, wanted_min_x, wanted_min_x},
                                   std::array<double, 4> {wanted_max_x, wanted_max_x, wanted_max_x, wanted_max_x},
                                   std::array<double, 4> {wanted_min_y, wanted_min_y, wanted_min_y, wanted_min_y},
                                   std::array<double, 4> {wanted_max_y, wanted_max_y, wanted_max_y, wanted_max_y},
                                   title_override, force_range);
    }

    virtual group_T construct_group(const double wanted_min_x, const double wanted_max_x,
                                    const std::string & title_override = "",
                                    const bool force_range = false) const
    {
      return this->construct_group(std::array<double, 4> {wanted_min_x, wanted_min_x, wanted_min_x, wanted_min_x},
                                   std::array<double, 4> {wanted_max_x, wanted_max_x, wanted_max_x, wanted_max_x},
                                   title_override, force_range);
    }


    virtual void plot(const std::string & path, const std::string & prefix = "", const std::string & suffix = "") const
    {
      group_T group = this->create_group();
      this->save(path + "/" + prefix + hist_name + suffix, group);
    }

    virtual void plot(const std::array<double, 4> & wanted_min_x, const std::array<double, 4> & wanted_max_x,
                      const std::string & path, const std::string & prefix = "", const std::string & suffix = "") const
    {
      group_T group = this->create_group(wanted_min_x, wanted_max_x);
      this->save(path + "/" + prefix + hist_name + suffix, group);
    }

    virtual void plot(const std::array<double, 4> & wanted_min_x, const std::array<double, 4> & wanted_max_x,
                      const std::array<double, 4> & wanted_min_y, const std::array<double, 4> & wanted_max_y,
                      const std::string & path, const std::string & prefix = "", const std::string & suffix = "") const
    {
      group_T group = this->create_group(wanted_min_x, wanted_max_x, wanted_min_y, wanted_max_y);
      this->save(path + "/" + prefix + hist_name + suffix, group);
    }

  };

  struct H1D_plotter : public basic_plotter<hist_group_1D>
  {
    using basic_plotter<hist_group_1D>::plot;
    using basic_plotter<hist_group_1D>::calc_data_range;
    using basic_plotter<hist_group_1D>::create_group;
    using basic_plotter<hist_group_1D>::construct_group;

    std::function<double(int)> min_calc;
    std::function<double(int)> max_calc;
    template <class F1, class F2, class F3>
    H1D_plotter(const BasePlotter * p, const std::string & name, F1 && minc, F2 && maxc, const bool cbn, const int cml,
                const std::string & title, const std::string & xlbl, const std::string & ylbl,
                F3 && popl, const StyleKinds & stl)
    {
      hist_name = name;
      hist_title = title;
      x_label = xlbl;
      y_label = ylbl;
      parent = p;
      can_be_negative = cbn;
      cumulative = cml;
      style_ref = stl;
      populator = popl;
      min_calc = minc;
      max_calc = maxc;
    }

    void calc_data_range(std::array<double, 4> & min, std::array<double, 4> & max) const override
    {
      for (int i = 0; i < 4; ++i)
        {
          const double data_min = min_calc(i);
          const double data_max = max_calc(i);

          const double data_range = data_max - data_min;

          min[i] = ( can_be_negative ?
                     data_min - 0.05 * data_range :
                     std::max(data_min - 0.05 * data_range, -0.5) );
          max[i] = data_max + 0.05 * data_range;
        }
    }

    void calc_data_range(std::array<double, 4> & min, std::array<double, 4> & max,
                         const std::array<double, 4> & wanted_min, const std::array<double, 4> & wanted_max) const override
    {
      for (int i = 0; i < 4; ++i)
        {
          if ( (wanted_min[i] <= -1 && !can_be_negative) || wanted_max[i] < 0)
            {
              const double data_min = min_calc(i);
              const double data_max = max_calc(i);

              const double data_range = data_max - data_min;

              min[i] = ( can_be_negative ?
                         data_min - 0.05 * data_range :
                         std::max(data_min - 0.05 * data_range, -0.5) );
              max[i] = data_max + 0.05 * data_range;
            }
          else
            {
              min[i] = wanted_min[i];
              max[i] = wanted_max[i];
            }
        }
    }
  };

  struct H1D_plotter_type : public H1D_plotter
  {
    using H1D_plotter::plot;
    using H1D_plotter::calc_data_range;
    using H1D_plotter::create_group;
    using H1D_plotter::construct_group;
    
    PlotterKind plotter_kind() const
    {
      return PlotterKind::type;
    }

    using H1D_plotter::H1D_plotter;

    hist_group_1D construct_group(const std::string & title_override = "") const
    {
      std::array<double, 4> min, max;

      calc_data_range(min, max);

      if (title_override == "")
        {
          return hist_group_1D(min, max, parent->num_bins, hist_name, hist_title,
                               x_label, y_label, parent->label_type_1, parent->label_type_2, parent->label_type_3);
        }
      else
        {
          return hist_group_1D(min, max, parent->num_bins, hist_name, title_override);
        }
    }

    hist_group_1D construct_group(const std::array<double, 4> & wanted_min, const std::array<double, 4> & wanted_max,
                                  const std::string & title_override = "",
                                  const bool force_range = false) const
    {
      std::array<double, 4> min, max;

      if (force_range)
        {
          for (int i = 0; i < 4; ++i)
            {
              min[i] = wanted_min[i];
              max[i] = wanted_max[i];
            }
        }
      else
        {
          calc_data_range(min, max, wanted_min, wanted_max);
        }

      if (title_override == "")
        {
          return hist_group_1D(min, max, parent->num_bins, hist_name, hist_title,
                               x_label, y_label, parent->label_type_1, parent->label_type_2, parent->label_type_3);
        }
      else
        {
          return hist_group_1D(min, max, parent->num_bins, hist_name, title_override);
        }
    }

    hist_group_1D construct_group(const std::array<double, 4> & min, const std::array<double, 4> & max,
                                  const std::array<double, 4> & /*ignore1*/, const std::array<double, 4> & /*ignore2*/,
                                  const std::string & title_override = "",
                                  const bool force_range = false) const
    {
      return this->construct_group(min, max, title_override, force_range);
    }

    void save(const std::string & base_name, const hist_group_1D & group) const
    {
      TCanvas cv("cv", "canvas", parent->canvas_x, parent->canvas_y);
      plot_one(&cv, group.global.get(), base_name, parent->file_extensions, parent->print_options);
      plot_one(&cv, group.t1.get(), base_name + "_" + parent->suffix_type_1, parent->file_extensions, parent->print_options);
      plot_one(&cv, group.t2.get(), base_name + "_" + parent->suffix_type_2, parent->file_extensions, parent->print_options);
      plot_one(&cv, group.t3.get(), base_name + "_" + parent->suffix_type_3, parent->file_extensions, parent->print_options);
    }
  };

  struct H1D_plotter_region : public H1D_plotter
  {
    using H1D_plotter::plot;
    using H1D_plotter::calc_data_range;
    using H1D_plotter::create_group;
    using H1D_plotter::construct_group;
    
    PlotterKind plotter_kind() const
    {
      return PlotterKind::region;
    }

    using H1D_plotter::H1D_plotter;

    hist_group_1D construct_group(const std::string & title_override = "") const
    {
      std::array<double, 4> min, max;

      calc_data_range(min, max);

      if (title_override == "")
        {
          return hist_group_1D(min, max, parent->num_bins, hist_name, hist_title, x_label, y_label,
                               parent->label_region_1, parent->label_region_2, parent->label_region_3);
        }
      else
        {
          return hist_group_1D(min, max, parent->num_bins, hist_name, title_override);
        }
    }

    hist_group_1D construct_group(const std::array<double, 4> & wanted_min, const std::array<double, 4> & wanted_max,
                                  const std::string & title_override = "",
                                  const bool force_range = false) const
    {
      std::array<double, 4> min, max;

      if (force_range)
        {
          for (int i = 0; i < 4; ++i)
            {
              min[i] = wanted_min[i];
              max[i] = wanted_max[i];
            }
        }
      else
        {
          calc_data_range(min, max, wanted_min, wanted_max);
        }

      if (title_override == "")
        {
          return hist_group_1D(min, max, parent->num_bins, hist_name, hist_title, x_label, y_label,
                               parent->label_region_1, parent->label_region_2, parent->label_region_3);
        }
      else
        {
          return hist_group_1D(min, max, parent->num_bins, hist_name, title_override);
        }
    }

    hist_group_1D construct_group(const std::array<double, 4> & min, const std::array<double, 4> & max,
                                  const std::array<double, 4> & /*ignore1*/, const std::array<double, 4> & /*ignore2*/,
                                  const std::string & title_override = "",
                                  const bool force_range = false) const
    {
      return this->construct_group(min, max, title_override, force_range);
    }

    void save(const std::string & base_name, const hist_group_1D & group) const
    {
      TCanvas cv("cv", "canvas", parent->canvas_x, parent->canvas_y);
      plot_one(&cv, group.global.get(), base_name, parent->file_extensions, parent->print_options);
      plot_one(&cv, group.t1.get(), base_name + "_" + parent->suffix_region_1, parent->file_extensions, parent->print_options);
      plot_one(&cv, group.t2.get(), base_name + "_" + parent->suffix_region_2, parent->file_extensions, parent->print_options);
      plot_one(&cv, group.t3.get(), base_name + "_" + parent->suffix_region_3, parent->file_extensions, parent->print_options);
    }
  };


  struct H1D_plotter_time : public H1D_plotter
  {
    using H1D_plotter::plot;
    using H1D_plotter::calc_data_range;
    using H1D_plotter::create_group;
    using H1D_plotter::construct_group;
    
    PlotterKind plotter_kind() const
    {
      return PlotterKind::time;
    }

    using H1D_plotter::H1D_plotter;

    hist_group_1D construct_group(const std::string & title_override = "") const
    {
      std::array<double, 4> min, max;

      calc_data_range(min, max);

      if (title_override == "")
        {
          return hist_group_1D(min, max, parent->num_bins, hist_name, hist_title,
                               x_label, y_label, "", "", "");
        }
      else
        {
          return hist_group_1D(min, max, parent->num_bins, hist_name, title_override);
        }
    }

    hist_group_1D construct_group(const std::array<double, 4> & wanted_min, const std::array<double, 4> & wanted_max,
                                  const std::string & title_override = "",
                                  const bool force_range = false) const
    {
      std::array<double, 4> min, max;

      if (force_range)
        {
          for (int i = 0; i < 4; ++i)
            {
              min[i] = wanted_min[i];
              max[i] = wanted_max[i];
            }
        }
      else
        {
          calc_data_range(min, max, wanted_min, wanted_max);
        }

      if (title_override == "")
        {
          return hist_group_1D(min, max, parent->num_bins, hist_name, hist_title,
                               x_label, y_label, "", "", "");
        }
      else
        {
          return hist_group_1D(min, max, parent->num_bins, hist_name, title_override);
        }
    }

    hist_group_1D construct_group(const std::array<double, 4> & min, const std::array<double, 4> & max,
                                  const std::array<double, 4> & /*ignore1*/, const std::array<double, 4> & /*ignore2*/,
                                  const std::string & title_override = "",
                                  const bool force_range = false) const
    {
      return this->construct_group(min, max, title_override, force_range);
    }

    void save(const std::string & base_name, const hist_group_1D & group) const
    {
      TCanvas cv("cv", "canvas", parent->canvas_x, parent->canvas_y);
      plot_one(&cv, group.global.get(), base_name, parent->file_extensions, parent->print_options);
    }
  };


  struct graph_plotter : public basic_plotter<graph_group_1D>
  {
    using basic_plotter<graph_group_1D>::plot;
    using basic_plotter<graph_group_1D>::calc_data_range;
    using basic_plotter<graph_group_1D>::create_group;
    using basic_plotter<graph_group_1D>::construct_group;
    
    std::function<double(int)> min_calc;
    std::function<double(int)> max_calc;
    template <class F1, class F2, class F3>
    graph_plotter(const BasePlotter * p, const std::string & name, F1 && minc, F2 && maxc, const bool cbn, const int cml,
                  const std::string & title, const std::string & xlbl, const std::string & ylbl,
                  F3 && popl, const StyleKinds & stl)
    {
      hist_name = name;
      hist_title = title;
      x_label = xlbl;
      y_label = ylbl;
      parent = p;
      can_be_negative = cbn;
      cumulative = cml;
      style_ref = stl;
      populator = popl;
      min_calc = minc;
      max_calc = maxc;
    }

    void calc_data_range(std::array<double, 4> & min, std::array<double, 4> & max) const override
    {
      for (int i = 0; i < 4; ++i)
        {
          const double data_min = min_calc(i);
          const double data_max = max_calc(i);

          const double data_range = data_max - data_min;

          min[i] = ( can_be_negative ?
                     data_min - 0.05 * data_range :
                     std::max(data_min - 0.05 * data_range, -0.5) );
          max[i] = data_max + 0.05 * data_range;
        }
    }

    void calc_data_range(std::array<double, 4> & min, std::array<double, 4> & max,
                         const std::array<double, 4> & wanted_min, const std::array<double, 4> & wanted_max) const override
    {
      for (int i = 0; i < 4; ++i)
        {
          if ( (wanted_min[i] <= -1 && !can_be_negative) || wanted_max[i] < 0)
            {
              const double data_min = min_calc(i);
              const double data_max = max_calc(i);

              const double data_range = data_max - data_min;

              min[i] = ( can_be_negative ?
                         data_min - 0.05 * data_range :
                         std::max(data_min - 0.05 * data_range, -0.5) );
              max[i] = data_max + 0.05 * data_range;
            }
          else
            {
              min[i] = wanted_min[i];
              max[i] = wanted_max[i];
            }
        }
    }

    PlotterKind plotter_kind() const override
    {
      return PlotterKind::graph;
    }

    graph_group_1D construct_group(const std::string & title_override = "") const override
    {
      std::array<double, 4> min, max;

      calc_data_range(min, max);

      if (title_override == "")
        {
          return graph_group_1D(min, max, parent->num_bins, hist_name, hist_title,
                                x_label, y_label, "", "", "");
        }
      else
        {
          return graph_group_1D(min, max, parent->num_bins, hist_name, title_override);
        }
    }

    graph_group_1D construct_group(const std::array<double, 4> & wanted_min, const std::array<double, 4> & wanted_max,
                                   const std::string & title_override = "",
                                   const bool force_range = false) const override
    {
      std::array<double, 4> min{}, max{};

      if (force_range)
        {
          for (int i = 0; i < 4; ++i)
            {
              min[i] = wanted_min[i];
              max[i] = wanted_max[i];
            }
        }
      else
        {
          calc_data_range(min, max, wanted_min, wanted_max);
        }

      if (title_override == "")
        {
          return graph_group_1D(min, max, parent->num_bins, hist_name, hist_title,
                                x_label, y_label, "", "", "");
        }
      else
        {
          return graph_group_1D(min, max, parent->num_bins, hist_name, title_override);
        }
    }

    graph_group_1D construct_group(const std::array<double, 4> & min, const std::array<double, 4> & max,
                                   const std::array<double, 4> & /*ignore1*/, const std::array<double, 4> & /*ignore2*/,
                                   const std::string & title_override = "",
                                   const bool force_range = false) const override
    {
      return this->construct_group(min, max, title_override, force_range);
    }

    void save(const std::string & base_name, const graph_group_1D & group) const override
    {
      TCanvas cv("cv", "canvas", parent->canvas_x, parent->canvas_y);
      plot_one(&cv, group.global.get(), base_name, parent->file_extensions, parent->print_options);
    }
  };

  struct H2D_plotter : public basic_plotter<hist_group_2D>
  {
    using basic_plotter<hist_group_2D>::plot;
    using basic_plotter<hist_group_2D>::calc_data_range;
    using basic_plotter<hist_group_2D>::create_group;
    using basic_plotter<hist_group_2D>::construct_group;

    std::function<double(int)> min_calc_x;
    std::function<double(int)> max_calc_x;
    std::function<double(int)> min_calc_y;
    std::function<double(int)> max_calc_y;
    template <class F1, class F2, class F3, class F4, class F5>
    H2D_plotter(const BasePlotter * p, const std::string & name, F1 && mincx, F2 && maxcx, F3 && mincy, F4 && maxcy,
                const bool cbn, const std::string & title, const std::string & xlbl, const std::string & ylbl,
                F5 && popl, const StyleKinds & stl)
    {
      hist_name = name;
      hist_title = title;
      x_label = xlbl;
      y_label = ylbl;
      parent = p;
      can_be_negative = cbn;
      style_ref = stl;
      populator = popl;
      min_calc_x = mincx;
      max_calc_x = maxcx;
      min_calc_y = mincy;
      max_calc_y = maxcy;
    }

    void calc_data_range_x(std::array<double, 4> & min, std::array<double, 4> & max) const
    {
      for (int i = 0; i < 4; ++i)
        {
          const double data_min = min_calc_x(i);
          const double data_max = max_calc_x(i);

          const double data_range = data_max - data_min;

          min[i] = ( can_be_negative ?
                     data_min - 0.05 * data_range :
                     std::max(data_min - 0.05 * data_range, -0.5) );
          max[i] = data_max + 0.05 * data_range;
        }
    }

    void calc_data_range_x(std::array<double, 4> & min, std::array<double, 4> & max,
                           const std::array<double, 4> & wanted_min, const std::array<double, 4> & wanted_max) const
    {
      for (int i = 0; i < 4; ++i)
        {
          if ( (wanted_min[i] <= -1 && !can_be_negative) || wanted_max[i] < 0)
            {
              const double data_min = min_calc_x(i);
              const double data_max = max_calc_x(i);

              const double data_range = data_max - data_min;

              min[i] = ( can_be_negative ?
                         data_min - 0.05 * data_range :
                         std::max(data_min - 0.05 * data_range, -0.5) );
              max[i] = data_max + 0.05 * data_range;
            }
          else
            {
              min[i] = wanted_min[i];
              max[i] = wanted_max[i];
            }
        }
    }

    void calc_data_range_y(std::array<double, 4> & min, std::array<double, 4> & max) const
    {
      for (int i = 0; i < 4; ++i)
        {
          const double data_min = min_calc_y(i);
          const double data_max = max_calc_y(i);

          const double data_range = data_max - data_min;

          min[i] = ( can_be_negative ?
                     data_min - 0.05 * data_range :
                     std::max(data_min - 0.05 * data_range, -0.5) );
          max[i] = data_max + 0.05 * data_range;
        }
    }

    void calc_data_range_y(std::array<double, 4> & min, std::array<double, 4> & max,
                           const std::array<double, 4> & wanted_min, const std::array<double, 4> & wanted_max) const
    {
      for (int i = 0; i < 4; ++i)
        {
          if ( (wanted_min[i] <= -1 && !can_be_negative) || wanted_max[i] < 0)
            {
              const double data_min = min_calc_y(i);
              const double data_max = max_calc_y(i);

              const double data_range = data_max - data_min;

              min[i] = ( can_be_negative ?
                         data_min - 0.05 * data_range :
                         std::max(data_min - 0.05 * data_range, -0.5) );
              max[i] = data_max + 0.05 * data_range;
            }
          else
            {
              min[i] = wanted_min[i];
              max[i] = wanted_max[i];
            }
        }
    }
  };

  struct H2D_plotter_type : public H2D_plotter
  {
    using H2D_plotter::plot;
    using H2D_plotter::calc_data_range;
    using H2D_plotter::calc_data_range_x;
    using H2D_plotter::calc_data_range_y;
    using H2D_plotter::create_group;
    using H2D_plotter::construct_group;
    
    PlotterKind plotter_kind() const
    {
      return PlotterKind::type2D;
    }

    using H2D_plotter::H2D_plotter;

    hist_group_2D construct_group(const std::string & /*title_override = ""*/) const
    {
      std::array<double, 4> min_x, max_x, min_y, max_y;

      calc_data_range_x(min_x, max_x);
      calc_data_range_y(min_y, max_y);

      return hist_group_2D(min_x, max_x, min_y, max_y, parent->num_bins, hist_name, hist_title, x_label, y_label,
                           parent->label_type_1, parent->label_type_2, parent->label_type_3);
    }

    hist_group_2D construct_group(const std::array<double, 4> & wanted_min, const std::array<double, 4> & wanted_max,
                                  const std::string & /*title_override = ""*/,
                                  const bool force_range = false) const
    {
      std::array<double, 4> min_x, max_x, min_y, max_y;

      if (force_range)
        {
          for (int i = 0; i < 4; ++i)
            {
              min_x[i] = wanted_min[i];
              max_x[i] = wanted_max[i];
            }
        }
      else
        {
          calc_data_range_x(min_x, max_x, wanted_min, wanted_max);
        }

      calc_data_range_y(min_y, max_y);

      return hist_group_2D(min_x, max_x, min_y, max_y, parent->num_bins, hist_name, hist_title, x_label, y_label,
                           parent->label_type_1, parent->label_type_2, parent->label_type_3);
    }

    hist_group_2D construct_group(const std::array<double, 4> & wanted_min_x, const std::array<double, 4> & wanted_max_x,
                                  const std::array<double, 4> & wanted_min_y, const std::array<double, 4> & wanted_max_y,
                                  const std::string & /*title_override = ""*/,
                                  const bool force_range = false) const
    {
      std::array<double, 4> min_x, max_x, min_y, max_y;

      if (force_range)
        {
          for (int i = 0; i < 4; ++i)
            {
              min_x[i] = wanted_min_x[i];
              max_x[i] = wanted_max_x[i];
              min_y[i] = wanted_min_y[i];
              max_y[i] = wanted_max_y[i];
            }
        }
      else
        {
          calc_data_range_x(min_x, max_x, wanted_min_x, wanted_max_x);
          calc_data_range_y(min_y, max_y, wanted_min_y, wanted_max_y);
        }

      return hist_group_2D(min_x, max_x, min_y, max_y, parent->num_bins, hist_name, hist_title, x_label, y_label,
                           parent->label_type_1, parent->label_type_2, parent->label_type_3);
    }

    void save(const std::string & base_name, const hist_group_2D & group) const
    {
      TCanvas cv("cv", "canvas", parent->canvas_x, parent->canvas_y);
      plot_one(&cv, group.global.get(), base_name, parent->file_extensions, parent->print_options);
      plot_one(&cv, group.t1.get(), base_name + "_" + parent->suffix_type_1, parent->file_extensions, parent->print_options);
      plot_one(&cv, group.t2.get(), base_name + "_" + parent->suffix_type_2, parent->file_extensions, parent->print_options);
      plot_one(&cv, group.t3.get(), base_name + "_" + parent->suffix_type_3, parent->file_extensions, parent->print_options);
    }
  };


  struct H2D_plotter_region : public H2D_plotter
  {
    using H2D_plotter::plot;
    using H2D_plotter::calc_data_range;
    using H2D_plotter::calc_data_range_x;
    using H2D_plotter::calc_data_range_y;
    using H2D_plotter::create_group;
    using H2D_plotter::construct_group;
    
    PlotterKind plotter_kind() const
    {
      return PlotterKind::region2D;
    }

    using H2D_plotter::H2D_plotter;

    hist_group_2D construct_group(const std::string & /*title_override = ""*/) const
    {
      std::array<double, 4> min_x, max_x, min_y, max_y;

      calc_data_range_x(min_x, max_x);
      calc_data_range_y(min_y, max_y);

      return hist_group_2D(min_x, max_x, min_y, max_y, parent->num_bins, hist_name, hist_title, x_label, y_label,
                           parent->label_region_1, parent->label_region_2, parent->label_region_3);
    }

    hist_group_2D construct_group(const std::array<double, 4> & wanted_min, const std::array<double, 4> & wanted_max,
                                  const std::string & /*title_override = ""*/,
                                  const bool force_range = false) const
    {
      std::array<double, 4> min_x, max_x, min_y, max_y;

      if (force_range)
        {
          for (int i = 0; i < 4; ++i)
            {
              min_x[i] = wanted_min[i];
              max_x[i] = wanted_max[i];
            }
        }
      else
        {
          calc_data_range_x(min_x, max_x, wanted_min, wanted_max);
        }

      calc_data_range_y(min_y, max_y);

      return hist_group_2D(min_x, max_x, min_y, max_y, parent->num_bins, hist_name, hist_title, x_label, y_label,
                           parent->label_region_1, parent->label_region_2, parent->label_region_3);
    }

    hist_group_2D construct_group(const std::array<double, 4> & wanted_min_x, const std::array<double, 4> & wanted_max_x,
                                  const std::array<double, 4> & wanted_min_y, const std::array<double, 4> & wanted_max_y,
                                  const std::string & /*title_override = ""*/,
                                  const bool force_range = false) const
    {
      std::array<double, 4> min_x, max_x, min_y, max_y;

      if (force_range)
        {
          for (int i = 0; i < 4; ++i)
            {
              min_x[i] = wanted_min_x[i];
              max_x[i] = wanted_max_x[i];
              min_y[i] = wanted_min_y[i];
              max_y[i] = wanted_max_y[i];
            }
        }
      else
        {
          calc_data_range_x(min_x, max_x, wanted_min_x, wanted_max_x);
          calc_data_range_y(min_y, max_y, wanted_min_y, wanted_max_y);
        }

      return hist_group_2D(min_x, max_x, min_y, max_y, parent->num_bins, hist_name, hist_title, x_label, y_label,
                           parent->label_region_1, parent->label_region_2, parent->label_region_3);
    }

    void save(const std::string & base_name, const hist_group_2D & group) const
    {
      TCanvas cv("cv", "canvas", parent->canvas_x, parent->canvas_y);
      plot_one(&cv, group.global.get(), base_name, parent->file_extensions, parent->print_options);
      plot_one(&cv, group.t1.get(), base_name + "_" + parent->suffix_type_1, parent->file_extensions, parent->print_options);
      plot_one(&cv, group.t2.get(), base_name + "_" + parent->suffix_type_2, parent->file_extensions, parent->print_options);
      plot_one(&cv, group.t3.get(), base_name + "_" + parent->suffix_type_3, parent->file_extensions, parent->print_options);
    }
  };

  struct joined_plotter : public plotter_base
  {
    using plotter_base::plot;
    using plotter_base::calc_data_range;
    
    std::vector<const plotter_base *> parts;
    std::vector<std::string> labels;

    void add_plot()
    {
    }

    void add_plot(const plotter_base * pplt, const std::string & label)
    {
      const H1D_plotter * conv_1 = dynamic_cast<const H1D_plotter *>(pplt);
      const joined_plotter * conv_2 = dynamic_cast<const joined_plotter *>(pplt);
      const graph_plotter * conv_3 = dynamic_cast<const graph_plotter *>(pplt);
      if (conv_1 != nullptr)
        {
          parts.push_back(conv_1);
          labels.push_back(label);
          //std::cout << "Added " << pplt->hist_name << " as " << label << std::endl;
        }
      else if (conv_2 != nullptr && this->plotter_kind() == conv_2->plotter_kind())
        {
          for (size_t i = 0; i < conv_2->parts.size(); ++i)
            {
              parts.push_back(conv_2->parts[i]);
              labels.push_back(label + ": " + conv_2->labels[i]);
            }
          //std::cout << "Added all from " << pplt->hist_name << " as " << label << std::endl;
        }
      else if (conv_3 != nullptr)
        {
          parts.push_back(conv_3);
          labels.push_back(label);
          //std::cout << "Added " << pplt->hist_name << " as " << label << std::endl;
        }
      else
        {
          std::cout << "WARNING: Trying to add to plot '" << hist_name
                    << "' an invalid plot: '" << pplt->hist_name << "' (labelled '" << label << "')" << std::endl;
        }
    }

    template <class ... Rest>
    void add_plot(const plotter_base * pplt, const std::string & label, Rest && ... rest)
    {
      add_plot(pplt, label);
      add_plot(std::forward<Rest>(rest)...);
    }

    joined_plotter(const BasePlotter * p, const std::string & name, const std::string & title, const std::string & xlbl, const std::string & ylbl)
    {
      hist_name = name;
      hist_title = title;
      x_label = xlbl;
      y_label = ylbl;
      parent = p;
    }

    template <class ... Rest>
    joined_plotter(const BasePlotter * p, const std::string & name, const std::string & title,
                   const std::string & xlbl, const std::string & ylbl, Rest && ... rest):
      joined_plotter(p, name, title, xlbl, ylbl)
    {
      add_plot(std::forward<Rest>(rest)...);
    }

    virtual hist_stacker create_stack() const = 0;
    virtual void save(const std::string & base_name, const hist_stacker & stacker) const = 0;

   protected:
    virtual void do_plots(const std::array<double, 4> & min_x, const std::array<double, 4> & max_x,
                          const std::string & path, const std::string & prefix = "", const std::string & suffix = "") const
    {
      std::vector<hist_group_1D> hists;
      hist_stacker stacker = this->create_stack();

      hists.reserve(parts.size());

      for (size_t i = 0; i < parts.size(); ++i)
        {
          const H1D_plotter * ptr = dynamic_cast<const H1D_plotter *>(parts[i]);
          if (ptr == nullptr)
            {
              std::cout << "ERROR: Converting to graph plotter (" << i << ", " << labels[i] << ")." << std::endl;
              continue;
            }
          hists.push_back(ptr->create_group(min_x, max_x, labels[i], true));
          hists.back().add_to_stack(stacker);
        }

      this->save(path + "/" + prefix + hist_name + suffix, stacker);
    }

   public:


    virtual void plot(const std::string & path, const std::string & prefix = "", const std::string & suffix = "") const
    {
      std::array<double, 4> real_min_x;
      std::array<double, 4> real_max_x;

      for (int j = 0; j < 4; ++j)
        {
          set_to_highest(real_min_x[j]);
          set_to_lowest(real_max_x[j]);
        }

      for (size_t i = 0; i < parts.size(); ++i)
        {
          std::array<double, 4> temp_min_x, temp_max_x;
          parts[i]->calc_data_range(temp_min_x, temp_max_x);
          for (int j = 0; j < 4; ++j)
            {
              real_min_x[j] = std::min(real_min_x[j], temp_min_x[j]);
              real_max_x[j] = std::max(real_max_x[j], temp_max_x[j]);
            }
        }

      this->do_plots(real_min_x, real_max_x, path, prefix, suffix);
    }

    virtual void plot(const std::array<double, 4> & wanted_min_x, const std::array<double, 4> & wanted_max_x,
                      const std::string & path, const std::string & prefix = "", const std::string & suffix = "") const
    {
      std::array<double, 4> real_min_x;
      std::array<double, 4> real_max_x;

      for (int j = 0; j < 4; ++j)
        {
          set_to_highest(real_min_x[j]);
          set_to_lowest(real_max_x[j]);
        }

      for (size_t i = 0; i < parts.size(); ++i)
        {
          std::array<double, 4> temp_min_x, temp_max_x;
          parts[i]->calc_data_range(temp_min_x, temp_max_x, wanted_min_x, wanted_max_x);
          for (int j = 0; j < 4; ++j)
            {
              real_min_x[j] = std::min(real_min_x[j], temp_min_x[j]);
              real_max_x[j] = std::max(real_max_x[j], temp_max_x[j]);
            }
        }

      this->do_plots(real_min_x, real_max_x, path, prefix, suffix);
    }

    virtual void plot(const std::array<double, 4> & wanted_min_x, const std::array<double, 4> & wanted_max_x,
                      const std::array<double, 4> & /*ignore1*/, const std::array<double, 4> & /*ignore2*/,
                      const std::string & path, const std::string & prefix = "", const std::string & suffix = "") const
    {
      this->plot(wanted_min_x, wanted_max_x, path, prefix, suffix);
    }

  };

  struct joined_plotter_type : public joined_plotter
  {
    using joined_plotter::plot;
    using joined_plotter::do_plots;
    using joined_plotter::calc_data_range;

    PlotterKind plotter_kind() const
    {
      return PlotterKind::type;
    }

    using joined_plotter::joined_plotter;

    virtual hist_stacker create_stack() const
    {
      return hist_stacker(hist_name, hist_title, x_label, y_label, parent->label_type_1, parent->label_type_2, parent->label_type_3);
    }

    virtual void save(const std::string & base_name, const hist_stacker & stacker) const
    {
      TCanvas cv("cv", "canvas", parent->canvas_x, parent->canvas_y);
      plot_one(&cv, stacker.global.get(), base_name, parent->file_extensions, parent->print_options);
      plot_one(&cv, stacker.t1.get(), base_name + "_" + parent->suffix_type_1, parent->file_extensions, parent->print_options);
      plot_one(&cv, stacker.t2.get(), base_name + "_" + parent->suffix_type_2, parent->file_extensions, parent->print_options);
      plot_one(&cv, stacker.t3.get(), base_name + "_" + parent->suffix_type_3, parent->file_extensions, parent->print_options);
    }
  };

  struct joined_plotter_region : public joined_plotter
  {
    using joined_plotter::plot;
    using joined_plotter::do_plots;
    using joined_plotter::calc_data_range;
    
    PlotterKind plotter_kind() const
    {
      return PlotterKind::region;
    }

    using joined_plotter::joined_plotter;

    virtual hist_stacker create_stack() const
    {
      return hist_stacker(hist_name, hist_title, x_label, y_label, parent->label_region_1, parent->label_region_2, parent->label_region_3);
    }

    virtual void save(const std::string & base_name, const hist_stacker & stacker) const
    {
      TCanvas cv("cv", "canvas", parent->canvas_x, parent->canvas_y);
      plot_one(&cv, stacker.global.get(), base_name, parent->file_extensions, parent->print_options);
      plot_one(&cv, stacker.t1.get(), base_name + "_" + parent->suffix_region_1, parent->file_extensions, parent->print_options);
      plot_one(&cv, stacker.t2.get(), base_name + "_" + parent->suffix_region_2, parent->file_extensions, parent->print_options);
      plot_one(&cv, stacker.t3.get(), base_name + "_" + parent->suffix_region_3, parent->file_extensions, parent->print_options);
    }
  };

  struct joined_plotter_time : public joined_plotter
  {
    using joined_plotter::plot;
    using joined_plotter::do_plots;
    using joined_plotter::calc_data_range;
    
    PlotterKind plotter_kind() const
    {
      return PlotterKind::time;
    }

    using joined_plotter::joined_plotter;

    virtual hist_stacker create_stack() const
    {
      return hist_stacker(hist_name, hist_title, x_label, y_label, "", "", "");
    }

    virtual void save(const std::string & base_name, const hist_stacker & stacker) const
    {
      TCanvas cv("cv", "canvas", parent->canvas_x, parent->canvas_y);
      plot_one(&cv, stacker.global.get(), base_name, parent->file_extensions, parent->print_options);
    }
  };

  struct joined_plotter_graph : public joined_plotter
  {
    using joined_plotter::plot;
    using joined_plotter::do_plots;
    using joined_plotter::calc_data_range;
    
    PlotterKind plotter_kind() const override
    {
      return PlotterKind::graph;
    }

    virtual hist_stacker create_stack() const override
    {
      std::cout << "ERROR: You shouldn't be seeing this!" << std::endl;
      return hist_stacker{};
    }

    using joined_plotter::joined_plotter;

    virtual void save(const std::string & base_name, const graph_stacker & stacker) const
    {
      TCanvas cv("cv", "canvas", parent->canvas_x, parent->canvas_y);
      plot_one(&cv, stacker.global.get(), base_name, parent->file_extensions, parent->print_options);
    }


    virtual void save(const std::string & /*base_name*/, const hist_stacker & /*stacker*/) const override
    {
      std::cout << "ERROR: You shouldn't be seeing this!" << std::endl;
      return;
    }

   protected:

    void do_plots(const std::array<double, 4> & min_x, const std::array<double, 4> & max_x,
                  const std::string & path, const std::string & prefix = "", const std::string & suffix = "") const override
    {
      std::vector<graph_group_1D> hists;
      graph_stacker stacker(hist_name, hist_title, x_label, y_label, "", "", "");

      hists.reserve(parts.size());

      for (size_t i = 0; i < parts.size(); ++i)
        {
          const graph_plotter * pltr = dynamic_cast<const graph_plotter *>(parts[i]);
          if (pltr == nullptr)
            {
              std::cout << "ERROR: Converting to graph plotter (" << i << ", " << labels[i] << ")." << std::endl;
              continue;
            }
          hists.push_back(pltr->create_group(min_x, max_x, labels[i], true));
          hists.back().add_to_stack(stacker);
        }

      this->save(path + "/" + prefix + hist_name + suffix, stacker);
    }
  };

  std::map<std::string, std::unique_ptr<plotter_base>> plots;
  //We need this to be ordered so one can use a lexicographical compare
  //to allow a limited form of globbing in the end.
  //(Should futurely be expanded to a more general form?)

  template <class T, class ... Args>
  plotter_base * add_plot(Args && ... args)
  {
    std::unique_ptr<plotter_base> ptr = std::make_unique<T>(this, std::forward<Args>(args)...);
    if (ptr){
      auto it = plots.try_emplace(ptr->hist_name, std::move(ptr));
      return it.first->second.get();
    }
    return nullptr;
  }


  template <class Graph, class ... Args>
  void fill_regions(Graph * all, Graph * central, Graph * endcap, Graph * forward, const float eta, Args && ... args) const
  {

    all->Fill(std::forward<Args>(args)...);

    const float abs_eta = std::abs(eta);
    if (abs_eta < regCuts[0])
      {
        central->Fill(std::forward<Args>(args)...);
      }
    else if (abs_eta < regCuts[1])
      {
        endcap->Fill(std::forward<Args>(args)...);
      }
    else if (abs_eta < regCuts[2])
      {
        forward->Fill(std::forward<Args>(args)...);
      }
    else
      {
        //std::cout << "ERROR: " << abs_eta << " is outside expected eta parameters!" << std::endl;
      }
  }

  template <class Graph, class ... Args>
  void fill_types(Graph * all, Graph * seed, Graph * grow, Graph * terminal, const float SNR, Args && ... args) const
  {

    all->Fill(std::forward<Args>(args)...);

    if (SNR > SNR_thresholds[2])
      {
        seed->Fill(std::forward<Args>(args)...);
      }
    else if (SNR > SNR_thresholds[1])
      {
        grow->Fill(std::forward<Args>(args)...);
      }
    else if (SNR > SNR_thresholds[0])
      {
        terminal->Fill(std::forward<Args>(args)...);
      }
    else
      {
        //std::cout << "ERROR: " << abs_eta << " is outside expected eta parameters!" << std::endl;
      }
  }

  template <class Graph, class ... Args>
  void fill_regions(hist_group<Graph> * group, const float x, Args && ... args) const
  {
    fill_regions(group->global.get(), group->t1.get(), group->t2.get(), group->t3.get(), x, std::forward<Args>(args)...);
  }

  template <class Graph, class ... Args>
  void fill_types(hist_group<Graph> * group, const float x, Args && ... args) const
  {
    fill_types(group->global.get(), group->t1.get(), group->t2.get(), group->t3.get(), x, std::forward<Args>(args)...);
  }

  std::vector<plotter_base *> get_plots(const std::string & name) const
  {
    std::vector<plotter_base *> ret;
    std::cout << "INFO: Trying to find plot(s) called '" << name << "'. " << std::flush;
    const auto it = plots.find(name);
    if (it != plots.end())
      {
        ret.push_back(it->second.get());
      }
    else
      {
        const auto str_find = name.rfind("*");
        if (str_find != std::string::npos)
          {
            const std::string substr = name.substr(0, str_find);
            auto map_it = plots.lower_bound(substr);
            if (map_it != plots.end() && map_it->first.find(substr) == std::string::npos)
              {
                ++map_it;
              }
            for (; map_it != plots.end() && map_it->first.find(substr) != std::string::npos; ++map_it)
              {
                ret.push_back(map_it->second.get());
              }
          }
      }
    std::cout << "Found " << ret.size() << "." << std::endl;
    return ret;
  }

  template <class ... Args>
  void plot(const std::string & name, Args && ... args) const
  {
    const auto plots = get_plots(name);
    if (plots.size() == 0)
      {
        std::cout << "ERROR: Trying to plot non-existent plot: '" << name << "'" << std::endl;
      }
    else
      {
        for (const auto & plt : plots)
          {
            plt->plot(std::forward<Args>(args)...);
          }
      }
  }
};


template <class PlotT>
void plot_together_helper(const std::string & /*name*/, PlotT * /*p*/)
{
}

template <class PlotT>
void plot_together_helper(const std::string & name, PlotT * p, const BasePlotter * plotter)
{
  const auto it = plotter->plots.find(name);
  p->add_plot(it->second.get(), plotter->plotter_name);
}


template <class PlotT>
void plot_together_helper(const std::string & name, PlotT * p, const BasePlotter & plotter)
{
  const auto it = plotter.plots.find(name);
  p->add_plot(it->second.get(), plotter.plotter_name);
}

template <class PlotT, class ... Rest>
void plot_together_helper(const std::string & name, PlotT * p, const BasePlotter & plotter, Rest && ... rest)
{
  plot_together_helper(name, p, plotter);
  plot_together_helper(name, p, std::forward<Rest>(rest)...);
}

template <class PlotT, class ... Rest>
void plot_together_helper(const std::string & name, PlotT * p, const BasePlotter * plotter, Rest && ... rest)
{
  plot_together_helper(name, p, plotter);
  plot_together_helper(name, p, std::forward<Rest>(rest)...);
}


template <class ... Rest>
std::vector<std::unique_ptr<BasePlotter::plotter_base>> plot_together(const std::string & name, const BasePlotter & first, Rest && ... rest)
{
  std::vector <std::unique_ptr<BasePlotter::plotter_base>> togetherplots;

  const auto plotters = first.get_plots(name);

  if (plotters.size() == 0)
    {
      std::cout << "ERROR: Trying to plot non-existent plot: '" << name << "'" << std::endl;
      return togetherplots;
    }

  for (const auto & pplt : plotters)
    {
      switch (pplt->plotter_kind())
        {
          case BasePlotter::PlotterKind::type:
            {
              auto ret = std::make_unique<BasePlotter::joined_plotter_type>(&first, pplt->hist_name, pplt->hist_title, pplt->x_label, pplt->y_label);
              ret->add_plot(pplt, first.plotter_name);
              plot_together_helper(ret, std::forward<Rest>(rest)...);
              togetherplots.emplace_back(ret.release());
            }
            break;
          case BasePlotter::PlotterKind::region:
            {
              auto ret = std::make_unique<BasePlotter::joined_plotter_region>(&first, pplt->hist_name, pplt->hist_title, pplt->x_label, pplt->y_label);
              ret->add_plot(pplt, first.plotter_name);
              plot_together_helper(pplt->hist_name, ret, std::forward<Rest>(rest)...);
              togetherplots.emplace_back(ret.release());
            }
            break;
          case BasePlotter::PlotterKind::time:
            {
              auto ret = std::make_unique<BasePlotter::joined_plotter_time>(&first, pplt->hist_name, pplt->hist_title, pplt->x_label, pplt->y_label);
              ret->add_plot(pplt, first.plotter_name);
              plot_together_helper(pplt->hist_name, ret, std::forward<Rest>(rest)...);
              togetherplots.emplace_back(ret.release());
            }
            break;
          case BasePlotter::PlotterKind::graph:
            {
              auto ret = std::make_unique<BasePlotter::joined_plotter_graph>(&first, pplt->hist_name, pplt->hist_title, pplt->x_label, pplt->y_label);
              ret->add_plot(pplt, first.plotter_name);
              plot_together_helper(pplt->hist_name, ret, std::forward<Rest>(rest)...);
              togetherplots.emplace_back(ret.release());
            }
            break;
          default:
            std::cout << "ERROR: Unsupported kind of plotter for joint plotting: " << int(pplt->plotter_kind()) << "." << std::endl;
        }
    }
  return togetherplots;
}

std::vector<std::unique_ptr<BasePlotter::plotter_base>> plot_together(const std::string & name, const std::vector<const BasePlotter *> & plots)
{
  std::vector <std::unique_ptr<BasePlotter::plotter_base>> togetherplots;

  if (plots.size() == 0)
    {
      return togetherplots;
    }

  const auto plotters = plots[0]->get_plots(name);

  if (plotters.size() == 0)
    {
      std::cout << "ERROR: Trying to plot non-existent plot: '" << name << "'" << std::endl;
      return togetherplots;
    }

  for (const auto & pplt : plotters)
    {
      switch (pplt->plotter_kind())
        {
          case BasePlotter::PlotterKind::type:
            {
              auto ret = std::make_unique<BasePlotter::joined_plotter_type>(plots[0], pplt->hist_name, pplt->hist_title, pplt->x_label, pplt->y_label);
              ret->add_plot(pplt, plots[0]->plotter_name);
              for (size_t i = 1; i < plots.size(); ++i)
                {
                  const auto it2 = plots[i]->plots.find(pplt->hist_name);
                  ret->add_plot(it2->second.get(), plots[i]->plotter_name);
                }
              togetherplots.emplace_back(ret.release());
            }
            break;
          case BasePlotter::PlotterKind::region:
            {
              auto ret = std::make_unique<BasePlotter::joined_plotter_region>(plots[0], pplt->hist_name, pplt->hist_title, pplt->x_label, pplt->y_label);
              ret->add_plot(pplt, plots[0]->plotter_name);
              for (size_t i = 1; i < plots.size(); ++i)
                {
                  const auto it2 = plots[i]->plots.find(pplt->hist_name);
                  ret->add_plot(it2->second.get(), plots[i]->plotter_name);
                }
              togetherplots.emplace_back(ret.release());
            }
            break;
          case BasePlotter::PlotterKind::time:
            {
              auto ret = std::make_unique<BasePlotter::joined_plotter_time>(plots[0], pplt->hist_name, pplt->hist_title, pplt->x_label, pplt->y_label);
              ret->add_plot(pplt, plots[0]->plotter_name);
              for (size_t i = 1; i < plots.size(); ++i)
                {
                  const auto it2 = plots[i]->plots.find(pplt->hist_name);
                  ret->add_plot(it2->second.get(), plots[i]->plotter_name);
                }
              togetherplots.emplace_back(ret.release());
            }
            break;
          case BasePlotter::PlotterKind::graph:
            {
              auto ret = std::make_unique<BasePlotter::joined_plotter_graph>(plots[0], pplt->hist_name, pplt->hist_title, pplt->x_label, pplt->y_label);
              ret->add_plot(pplt, plots[0]->plotter_name);
              for (size_t i = 1; i < plots.size(); ++i)
                {
                  const auto it2 = plots[i]->plots.find(pplt->hist_name);
                  ret->add_plot(it2->second.get(), plots[i]->plotter_name);
                }
              togetherplots.emplace_back(ret.release());
            }
            break;
          default:
            std::cout << "ERROR: Unsupported kind of plotter for joint plotting: " << int(pplt->plotter_kind()) << "." << std::endl;
        }
    }

  return togetherplots;
}

struct TogetherPlot
{
  std::vector<const BasePlotter *> plots;


  void add_plot(const BasePlotter * bp)
  {
    //std::cout << "Adding " << bp->plotter_name << std::endl;
    plots.push_back(bp);
  }

  void add_plot()
  {
  }

  template <class ... Rest>
  void add_plot(const BasePlotter * bp, Rest && ... rest)
  {
    add_plot(bp);
    add_plot(std::forward<Rest>(rest)...);
  }

  template<class ... Args>
  TogetherPlot(Args && ... args)
  {
    add_plot(std::forward<Args>(args)...);
  }


  template <class ... Args>
  void plot(const std::string & name, Args && ... args) const
  {
    std::vector< std::unique_ptr<BasePlotter::plotter_base> > plotters = plot_together(name, plots);
    for (const auto & plt : plotters)
      {
        plt->plot(std::forward<Args>(args)...);
      }
  }
};

#endif //CALORECGPU_TOOLS_PLOTTERAUXDEFINES_H
