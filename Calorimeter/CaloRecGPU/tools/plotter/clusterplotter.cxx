//
// Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
//
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <map>
#include <string>
#include <cstring>
#include <fstream>
#include <list>
#include "PlotterAuxDefines.h"

#include <boost/filesystem.hpp>

#include "CxxUtils/checker_macros.h"

#ifdef ATLAS_CHECK_THREAD_SAFETY

  ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#endif

#include "ClusterPlotter.h"
#include "TimePlotter.h"


struct PlotDefs
{
  std::string name;
  std::string pref;
  std::string suf;
  double xmin, xmax, ymin, ymax;
  int bin_size;
  double labelx, labely;

  PlotDefs(const std::string & n = "", const std::string & p = "", const std::string & s = "",
           const double xmi = -1, const double xma = -1, const double ymi = -1, const double yma = -1,
           const int bs = -1, const double lx = -1, const double ly = -1):
    name(n), pref(p), suf(s), xmin(xmi), xmax(xma), ymin(ymi), ymax(yma), bin_size(bs), labelx(lx), labely(ly)
  {
  }

  void reset()
  {
    name = "";
    pref = "";
    suf = "";
    xmin = -1;
    xmax = -1;
    ymin = -1;
    ymax = -1;
    bin_size = -1;
    labelx = -1;
    labely = -1;
  }

  template <class Stream> friend
  Stream & operator << (Stream & str, const PlotDefs & pd)
  {
    str << pd.name << "|" << pd.pref << "|" << pd.suf << "|" << pd.xmin << " " << pd.xmax << " " << pd.ymin << " " << pd.ymax << " " << pd.bin_size << " " << pd.labelx << " " << pd.labely << std::endl;
    return str;
  }

  template <class Stream> friend
  Stream & operator>> (Stream & str, PlotDefs & pd)
  {
    pd.reset();

    std::string line;
    std::getline(str, line);

    const size_t pos_1 = line.find('|');
    if (pos_1 == std::string::npos)
      {
        return str;
      }
    pd.name = line.substr(0, pos_1);

    size_t pos_final = pos_1;

    const size_t pos_2 = line.find('|', pos_1 + 1);


    if (pos_2 != std::string::npos)
      {
        pd.pref = line.substr(pos_1 + 1, pos_2 - pos_1 - 1);
        pos_final = pos_2;

        const size_t pos_3 = line.find('|', pos_2 + 1);
        if (pos_3 != std::string::npos)
          {
            pd.suf = line.substr(pos_2 + 1, pos_3 - pos_2 - 1);
            pos_final = pos_3;
          }
      }

    std::stringstream readstr(line.substr(pos_final + 1));

    readstr >> pd.xmin >> pd.xmax >> pd.ymin >> pd.ymax >> pd.bin_size >> pd.labelx >> pd.labely;

    return str;
  }
};

struct ProgArgs
{
  std::vector<std::string> in_paths;
  std::vector<std::string> names;
  std::string out_path;
  double min_similarity = 0.;
  double term_weight= 0.;
  double grow_weight= 0.;
  double seed_weight= 0.;
  bool place_titles{};
  std::string plot_label;
  std::vector<PlotDefs> plot_reqs;
  std::vector<PlotDefs> togetherplot_reqs;
  std::vector<PlotDefs> timeplot_reqs;
  std::vector<PlotDefs> timetogetherplot_reqs;
  bool normalize_hists{};
  bool skip_almost_everything{};
  bool use_ATLAS_style{};
  std::string file_filter;
  int max_events{};
  bool just_png{};
  bool do_clusters{};
  bool do_times{};
  std::string constant_foldername;
  std::string reference_foldername;
  std::string test_foldername;
  void reset()
  {
    in_paths.clear();
    names.clear();
    out_path = "./plots/";
    min_similarity = default_min_similarity;
    term_weight = default_term_weight;
    grow_weight = default_grow_weight;
    seed_weight = default_seed_weight;
    place_titles = false;
    plot_label = "";
    plot_reqs.clear();
    togetherplot_reqs.clear();
    timeplot_reqs.clear();
    timetogetherplot_reqs.clear();
    normalize_hists = false;
    skip_almost_everything = false;
    use_ATLAS_style = true;
    file_filter = "";
    max_events = -1;
    just_png = false;
    do_clusters = true;
    do_times = true;
    constant_foldername = "val_modified_grow";
    reference_foldername = "val_default_grow";
    test_foldername = "val_modified_grow";
  }
};

void read_plots_file(const boost::filesystem::path & file, std::vector<PlotDefs> & vec)
{
  vec.clear();
  if (file == "none" || file == "NONE")
    {
      return;
    }
  std::ifstream in(file);
  while (in.good())
    {
      if (in.peek() == '\n')
        {
          in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
      else if (in.peek() == '#')
        {
          in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
      else
        {
          PlotDefs pd;
          in >> pd;
          if (pd.name != "")
            {
              vec.push_back(pd);
            }
        }
    }
  /*
    for (const auto & pd : vec)
      {
        std::cout << " READ: " << pd << std::endl;
        }
  */
}


void set_default_plots(ProgArgs & args)
{
  [[maybe_unused]] constexpr int default_num_bins = 250;
  [[maybe_unused]] constexpr int shorter_num_bins = 100;
  [[maybe_unused]] constexpr int small_num_bins = 50;
  [[maybe_unused]] constexpr int smallest_num_bins = 25;
  /*

    args.plot_reqs.emplace_back("diff_num_hist", "", "", -1, -1, -1, -1, smallest_num_bins);
    args.plot_reqs.emplace_back("diff_num_hist", "", "_zoom", -10, 10);
    args.plot_reqs.emplace_back("diff_num_rel_hist", "", "", -1, -1);
    args.plot_reqs.emplace_back("frac_cell_diff_reg_hist", "", "", -1, -1);
    args.plot_reqs.emplace_back("frac_cell_diff_type_hist", "", "", -1, -1);
    args.plot_reqs.emplace_back("frac_cell_diff_reg_hist", "", "_zoom", 0, 0.2);
    args.plot_reqs.emplace_back("frac_cell_diff_type_hist", "", "_zoom", 0, 0.2);

    args.plot_reqs.emplace_back("diff_cell_per_cluster", "", "", -1, -1, -1, -1, default_num_bins);
    args.plot_reqs.emplace_back("diff_cell_per_cluster", "", "_zoom1", 0, 2500);
    args.plot_reqs.emplace_back("diff_cell_per_cluster", "", "_zoom2", 0, 500);
    args.plot_reqs.emplace_back("diff_cell_per_cluster", "", "_zoom3", 0, 100);

    args.plot_reqs.emplace_back("rel_diff_cell_per_cluster", "", "", -1, -1);
    args.plot_reqs.emplace_back("rel_diff_cell_per_cluster", "", "_zoom1", 0, 0.5);
    args.plot_reqs.emplace_back("rel_diff_cell_per_cluster", "", "_zoom2", 0., 0.25);
    args.plot_reqs.emplace_back("rel_diff_cell_per_cluster", "", "_zoom3", 0., 0.05);

    args.plot_reqs.emplace_back("delta_R_hist", "", "", -1,  -1);
    args.plot_reqs.emplace_back("delta_R_hist", "", "_zoom1", 0.,  0.25);
    args.plot_reqs.emplace_back("delta_R_hist", "", "_zoom2", 0.,  0.001);
    args.plot_reqs.emplace_back("delta_R_hist", "", "_zoom3", 0.,  0.0001);
  */

  args.plot_reqs.emplace_back("delta_cells_vs_delta_R", "", "", -1, -1, -1, -1, default_num_bins);
  args.plot_reqs.emplace_back("delta_cells_vs_delta_R", "", "_zoom1", 0., 0.25, -1, -1);
  args.plot_reqs.emplace_back("delta_cells_vs_delta_R", "", "_zoom2", 0., 0.001, -1, -1);
  args.plot_reqs.emplace_back("delta_cells_vs_delta_R", "", "_zoom3", 0., 0.0001, -1, -1);

  args.plot_reqs.emplace_back("delta_R_vs_E", "", "", -1, -1, -1, -1);
  args.plot_reqs.emplace_back("delta_R_vs_E", "", "_zoom", -1000, 1000, 0, 0.15);
  args.plot_reqs.emplace_back("delta_R_vs_Et", "", "", -1, -1, -1, -1);
  args.plot_reqs.emplace_back("delta_R_vs_Et", "", "_zoom1", -175, 175, 0, 0.15);
  args.plot_reqs.emplace_back("delta_R_vs_Et", "", "_zoom2", -75, 75, 0, 0.15);
  args.plot_reqs.emplace_back("delta_R_vs_Eta", "", "", -1, -1, -1, -1);
  args.plot_reqs.emplace_back("delta_R_vs_Eta", "", "_zoom", -7.5, 7.5, 0, 0.15);
  args.plot_reqs.emplace_back("delta_R_vs_Phi", "", "", -1, -1, -1, -1);
  args.plot_reqs.emplace_back("delta_R_vs_Phi", "", "_zoom", -1, -1, 0, 0.15);
  args.plot_reqs.emplace_back("delta_R_vs_size", "", "", -1, -1, -1, -1);
  args.plot_reqs.emplace_back("delta_R_vs_size", "", "_zoom", 0, 3000, 0., 0.15);

  args.plot_reqs.emplace_back("delta_Et_rel_vs_Et", "", "", -1, -1, -1., 10.);
  args.plot_reqs.emplace_back("delta_Et_rel_vs_Et", "", "_zoom1", -150, 150, -0.25, 0.25);
  args.plot_reqs.emplace_back("delta_Et_rel_vs_Et", "", "_zoom2", -75, 75, -0.25, 0.25);
  args.plot_reqs.emplace_back("delta_Et_rel_hist", "", "", -1, -1);
  args.plot_reqs.emplace_back("delta_Et_rel_hist", "", "_zoom", -0.5, 0.5);
  args.plot_reqs.emplace_back("Et1_vs_Et2", "", "", -1, -1, -1, -1);
  args.plot_reqs.emplace_back("Et1_vs_Et2", "", "_zoom", -100, 100, -100, 100);
  args.plot_reqs.emplace_back("E1_vs_E2", "", "", -1, -1, -1, -1);
  args.plot_reqs.emplace_back("E1_vs_E2", "", "_zoom", -200, 200, -200, 200);
  /*
    args.plot_reqs.emplace_back("Et_hist", "", "", -1, -1);
    args.plot_reqs.emplace_back("Et_hist", "", "_zoom1", -200, 200);
    args.plot_reqs.emplace_back("Et_hist", "", "_zoom2", -100, 100);
    args.plot_reqs.emplace_back("E_hist", "", "", -1, -1);
    args.plot_reqs.emplace_back("E_hist", "", "_zoom", -1500, 1500);
    args.plot_reqs.emplace_back("cluster_size_hist", "", "", -1, -1);
    args.plot_reqs.emplace_back("cluster_size_hist", "", "_zoom", 0, 1000);
    args.plot_reqs.emplace_back("cluster_eta_hist", "", "", -1, -1);
    args.plot_reqs.emplace_back("cluster_eta_hist", "", "_full", -7.5, 7.5);
    args.plot_reqs.emplace_back("cluster_eta_hist", "", "_zoom0", -50, 50);
    args.plot_reqs.emplace_back("cluster_eta_hist", "", "_zoom1", -10, 10);
    args.plot_reqs.emplace_back("cluster_eta_hist", "", "_zoom2", -3, 3);
    args.plot_reqs.emplace_back("cluster_phi_hist", "", "", -1, -1);
    args.plot_reqs.emplace_back("cluster_phi_hist", "", "_full", -pi<double>, pi<double>);


    args.plot_reqs.emplace_back("delta_eta_hist", "", "", -1,  -1);
    args.plot_reqs.emplace_back("delta_eta_hist", "", "_zoom1", -10,  10);
    args.plot_reqs.emplace_back("delta_eta_hist", "", "_zoom3", -1,  1);
    args.plot_reqs.emplace_back("delta_eta_hist", "", "_zoom3", -0.25,  0.25);
    args.plot_reqs.emplace_back("delta_eta_hist", "", "_zoom4", -0.1,  0.1);
    args.plot_reqs.emplace_back("delta_eta_hist", "", "_zoom5", -0.01,  0.01);
    args.plot_reqs.emplace_back("delta_eta_hist", "", "_zoom6", -0.001,  0.001);

    args.plot_reqs.emplace_back("delta_phi_hist", "", "", -1,  -1);
    args.plot_reqs.emplace_back("delta_phi_hist", "", "_full", -pi<double>,  pi<double>);

    args.plot_reqs.emplace_back("delta_E_hist", "", "", -1,  -1);
    args.plot_reqs.emplace_back("delta_E_hist", "", "_zoom1", -1000,  1000);
    args.plot_reqs.emplace_back("delta_E_hist", "", "_zoom2", -500,  500);
    args.plot_reqs.emplace_back("delta_E_hist", "", "_zoom3", -100,  100);
    args.plot_reqs.emplace_back("delta_E_hist", "", "_zoom4", -50,  50);
    args.plot_reqs.emplace_back("delta_E_hist", "", "_zoom5", -10,  10);
    args.plot_reqs.emplace_back("delta_E_hist", "", "_zoom6", -1,  1);
    args.plot_reqs.emplace_back("delta_E_hist", "", "_zoom7", -0.1,  0.1);
    args.plot_reqs.emplace_back("delta_E_hist", "", "_zoom8", -0.01,  0.01);
    args.plot_reqs.emplace_back("delta_E_hist", "", "_zoom9", -0.001,  0.001);

    args.plot_reqs.emplace_back("delta_Et_hist", "", "", -1,  -1);
    args.plot_reqs.emplace_back("delta_Et_hist", "", "_zoom1", -1000,  1000);
    args.plot_reqs.emplace_back("delta_Et_hist", "", "_zoom2", -500,  500);
    args.plot_reqs.emplace_back("delta_Et_hist", "", "_zoom3", -100,  100);
    args.plot_reqs.emplace_back("delta_Et_hist", "", "_zoom4", -50,  50);
    args.plot_reqs.emplace_back("delta_Et_hist", "", "_zoom5", -10,  10);
    args.plot_reqs.emplace_back("delta_Et_hist", "", "_zoom6", -1,  1);
    args.plot_reqs.emplace_back("delta_Et_hist", "", "_zoom7", -0.1,  0.1);
    args.plot_reqs.emplace_back("delta_Et_hist", "", "_zoom8", -0.01,  0.01);
    args.plot_reqs.emplace_back("delta_Et_hist", "", "_zoom9", -0.001,  0.001);



    args.plot_reqs.emplace_back("cluster_number_hist", "", "", -1, -1, -1, -1, smallest_num_bins);
    args.plot_reqs.emplace_back("term_cell_E_perc_hist", "", "", -1, -1);
  */

  args.plot_reqs.emplace_back("term_cell_E_perc_vs_E_ref", "", "", -1, -1, -1, -1, default_num_bins);
  args.plot_reqs.emplace_back("term_cell_E_perc_vs_E_test", "", "", -1, -1, -1, -1);

  /*


    args.plot_reqs.emplace_back("unmatched_number_hist", "", "", -1, -1, -1, -1, smallest_num_bins);


    args.plot_reqs.emplace_back("unmatched_perc_hist", "", "", -1, -1, -1, -1, small_num_bins);


    args.plot_reqs.emplace_back("unmatched_E_hist", "", "", -1, -1, -1, -1,  shorter_num_bins);
    args.plot_reqs.emplace_back("unmatched_E_hist", "", "_zoom", -50, 100);
    args.plot_reqs.emplace_back("unmatched_Et_hist", "", "", -1, -1);
    args.plot_reqs.emplace_back("unmatched_Et_hist", "", "_zoom1", -50, 50);
    args.plot_reqs.emplace_back("unmatched_Et_hist", "", "_zoom2", -20, 20);
    args.plot_reqs.emplace_back("unmatched_sizes_hist", "", "", -1, -1);
    args.plot_reqs.emplace_back("unmatched_sizes_hist", "", "_zoom", 0, 1000);
    args.plot_reqs.emplace_back("unmatched_eta_hist", "", "", -1, -1);
    args.plot_reqs.emplace_back("unmatched_eta_hist", "", "_full", -7.5, 7.5);
    args.plot_reqs.emplace_back("unmatched_eta_hist", "", "_zoom", -3, 3);
    args.plot_reqs.emplace_back("unmatched_phi_hist", "", "", -1, -1);
    args.plot_reqs.emplace_back("unmatched_phi_hist", "", "_full", -pi<double>, pi<double>);

    args.plot_reqs.emplace_back("equal_cells_delta_R", "", "", -1,  -1, -1, -1, default_num_bins);
    args.plot_reqs.emplace_back("equal_cells_delta_R", "", "_zoom1", 0.,  0.25);
    args.plot_reqs.emplace_back("equal_cells_delta_R", "", "_zoom2", 0.,  0.001);
    args.plot_reqs.emplace_back("equal_cells_delta_R", "", "_zoom3", 0.,  0.0001);


    args.plot_reqs.emplace_back("equal_cells_delta_eta", "", "", -1,  -1);
    args.plot_reqs.emplace_back("equal_cells_delta_eta", "", "_zoom1", -10,  10);
    args.plot_reqs.emplace_back("equal_cells_delta_eta", "", "_zoom2", -1,  1);
    args.plot_reqs.emplace_back("equal_cells_delta_eta", "", "_zoom3", -0.25,  0.25);
    args.plot_reqs.emplace_back("equal_cells_delta_eta", "", "_zoom4", -0.1,  0.1);
    args.plot_reqs.emplace_back("equal_cells_delta_eta", "", "_zoom5", -0.01,  0.01);
    args.plot_reqs.emplace_back("equal_cells_delta_eta", "", "_zoom6", -0.001,  0.001);


    args.plot_reqs.emplace_back("equal_cells_delta_phi", "", "", -1,  -1);
    args.plot_reqs.emplace_back("equal_cells_delta_phi", "", "_full", -pi<double>,  pi<double>);
    args.plot_reqs.emplace_back("equal_cells_delta_phi", "", "_zoom1", -1,  1);
    args.plot_reqs.emplace_back("equal_cells_delta_phi", "", "_zoom2", -0.1,  0.1);

    args.plot_reqs.emplace_back("equal_cells_delta_E", "", "", -1,  -1);
    args.plot_reqs.emplace_back("equal_cells_delta_E", "", "_zoom1", -1000,  1000);
    args.plot_reqs.emplace_back("equal_cells_delta_E", "", "_zoom2", -500,  500);
    args.plot_reqs.emplace_back("equal_cells_delta_E", "", "_zoom3", -100,  100);
    args.plot_reqs.emplace_back("equal_cells_delta_E", "", "_zoom4", -50,  50);
    args.plot_reqs.emplace_back("equal_cells_delta_E", "", "_zoom5", -10,  10);
    args.plot_reqs.emplace_back("equal_cells_delta_E", "", "_zoom6", -1,  1);
    args.plot_reqs.emplace_back("equal_cells_delta_E", "", "_zoom7", -0.1,  0.1);
    args.plot_reqs.emplace_back("equal_cells_delta_E", "", "_zoom8", -0.01,  0.01);
    args.plot_reqs.emplace_back("equal_cells_delta_E", "", "_zoom9", -0.001,  0.001);

    args.plot_reqs.emplace_back("equal_cells_delta_E_rel", "", "", -1,  -1);
    args.plot_reqs.emplace_back("equal_cells_delta_E_rel", "", "_zoom1", -1,  1);
    args.plot_reqs.emplace_back("equal_cells_delta_E_rel", "", "_zoom2", -0.5,  0.5);
    args.plot_reqs.emplace_back("equal_cells_delta_E_rel", "", "_zoom3", -0.1,  0.1);

    args.plot_reqs.emplace_back("equal_cells_delta_Et", "", "", -1,  -1);
    args.plot_reqs.emplace_back("equal_cells_delta_Et", "", "_zoom1", -1000,  1000);
    args.plot_reqs.emplace_back("equal_cells_delta_Et", "", "_zoom2", -500,  500);
    args.plot_reqs.emplace_back("equal_cells_delta_Et", "", "_zoom3", -100,  100);
    args.plot_reqs.emplace_back("equal_cells_delta_Et", "", "_zoom4", -50,  50);
    args.plot_reqs.emplace_back("equal_cells_delta_Et", "", "_zoom5", -10,  10);
    args.plot_reqs.emplace_back("equal_cells_delta_Et", "", "_zoom6", -1,  1);
    args.plot_reqs.emplace_back("equal_cells_delta_Et", "", "_zoom7", -0.1,  0.1);
    args.plot_reqs.emplace_back("equal_cells_delta_Et", "", "_zoom8", -0.01,  0.01);
    args.plot_reqs.emplace_back("equal_cells_delta_Et", "", "_zoom9", -0.001,  0.001);

    args.plot_reqs.emplace_back("equal_cells_delta_Et_rel", "", "", -1,  -1);
    args.plot_reqs.emplace_back("equal_cells_delta_Et_rel", "", "_zoom1", -1,  1);
    args.plot_reqs.emplace_back("equal_cells_delta_Et_rel", "", "_zoom2", -0.5,  0.5);
    args.plot_reqs.emplace_back("equal_cells_delta_Et_rel", "", "_zoom3", -0.1,  0.1);
  */
  /*

    args.plot_reqs.emplace_back("post_calc_delta_R", "", "", -1,  -1);
    args.plot_reqs.emplace_back("post_calc_delta_R", "", "_zoom1", 0.,  0.25);
    args.plot_reqs.emplace_back("post_calc_delta_R", "", "_zoom2", 0.,  0.001);
    args.plot_reqs.emplace_back("post_calc_delta_R", "", "_zoom3", 0.,  0.0001);


    args.plot_reqs.emplace_back("post_calc_delta_eta", "", "", -1,  -1);
    args.plot_reqs.emplace_back("post_calc_delta_eta", "", "_zoom1", -10,  10);
    args.plot_reqs.emplace_back("post_calc_delta_eta", "", "_zoom2", -1,  1);
    args.plot_reqs.emplace_back("post_calc_delta_eta", "", "_zoom3", -0.25,  0.25);
    args.plot_reqs.emplace_back("post_calc_delta_eta", "", "_zoom4", -0.1,  0.1);
    args.plot_reqs.emplace_back("post_calc_delta_eta", "", "_zoom5", -0.01,  0.01);


    args.plot_reqs.emplace_back("post_calc_delta_phi", "", "", -1,  -1);
    args.plot_reqs.emplace_back("post_calc_delta_phi", "", "_full", -pi<double>,  pi<double>);
    args.plot_reqs.emplace_back("post_calc_delta_phi", "", "_zoom1", -1,  1);
    args.plot_reqs.emplace_back("post_calc_delta_phi", "", "_zoom2", -0.1,  0.1);

    args.plot_reqs.emplace_back("post_calc_delta_E", "", "", -1,  -1);
    args.plot_reqs.emplace_back("post_calc_delta_E", "", "_zoom1", -1000,  1000);
    args.plot_reqs.emplace_back("post_calc_delta_E", "", "_zoom2", -500,  500);
    args.plot_reqs.emplace_back("post_calc_delta_E", "", "_zoom3", -100,  100);
    args.plot_reqs.emplace_back("post_calc_delta_E", "", "_zoom4", -50,  50);
    args.plot_reqs.emplace_back("post_calc_delta_E", "", "_zoom5", -10,  10);
    args.plot_reqs.emplace_back("post_calc_delta_E", "", "_zoom6", -1,  1);
    args.plot_reqs.emplace_back("post_calc_delta_E", "", "_zoom7", -0.1,  0.1);
    args.plot_reqs.emplace_back("post_calc_delta_E", "", "_zoom8", -0.01,  0.01);
    args.plot_reqs.emplace_back("post_calc_delta_E", "", "_zoom9", -0.001,  0.001);

    args.plot_reqs.emplace_back("post_calc_delta_E_rel", "", "", -1,  -1);
    args.plot_reqs.emplace_back("post_calc_delta_E_rel", "", "_zoom1", -1,  1);
    args.plot_reqs.emplace_back("post_calc_delta_E_rel", "", "_zoom2", -0.5,  0.5);
    args.plot_reqs.emplace_back("post_calc_delta_E_rel", "", "_zoom3", -0.1,  0.1);
    args.plot_reqs.emplace_back("post_calc_delta_E_rel", "", "_zoom4", -0.005,  0.005);


    args.plot_reqs.emplace_back("delta_post_E", "", "", -1,  -1);
    args.plot_reqs.emplace_back("delta_post_E", "", "_zoom1", -1000,  1000);
    args.plot_reqs.emplace_back("delta_post_E", "", "_zoom2", -500,  500);
    args.plot_reqs.emplace_back("delta_post_E", "", "_zoom3", -100,  100);
    args.plot_reqs.emplace_back("delta_post_E", "", "_zoom4", -50,  50);
    args.plot_reqs.emplace_back("delta_post_E", "", "_zoom5", -10,  10);
    args.plot_reqs.emplace_back("delta_post_E", "", "_zoom6", -1,  1);


    args.plot_reqs.emplace_back("delta_post_E_rel", "", "", -1,  -1);
    args.plot_reqs.emplace_back("delta_post_E_rel", "", "_zoom1", -1,  1);
    args.plot_reqs.emplace_back("delta_post_E_rel", "", "_zoom2", -0.5,  0.5);
    args.plot_reqs.emplace_back("delta_post_E_rel", "", "_zoom3", -0.1,  0.1);
    args.plot_reqs.emplace_back("delta_post_E_rel", "", "_zoom4", -0.005,  0.005);

    args.plot_reqs.emplace_back("delta_post_eta", "", "", -1,  -1);
    args.plot_reqs.emplace_back("delta_post_eta", "", "_zoom1", -10,  10);
    args.plot_reqs.emplace_back("delta_post_eta", "", "_zoom2", -1,  1);
    args.plot_reqs.emplace_back("delta_post_eta", "", "_zoom3", -0.25,  0.25);
    args.plot_reqs.emplace_back("delta_post_eta", "", "_zoom4", -0.1,  0.1);


    args.plot_reqs.emplace_back("delta_post_phi", "", "", -1,  -1);
    args.plot_reqs.emplace_back("delta_post_phi", "", "_full", -pi<double>,  pi<double>);
    args.plot_reqs.emplace_back("delta_post_phi", "", "_zoom1", -1,  1);
    args.plot_reqs.emplace_back("delta_post_phi", "", "_zoom2", -0.1,  0.1);




  args.plot_reqs.emplace_back("time_histogram", "", "",  -1,  -1, small_num_bins);
  args.plot_reqs.emplace_back("time_histogram", "", "_zoom1",  0,  60000);
  args.plot_reqs.emplace_back("time_histogram", "", "_zoom2",  5000,  60000);

  args.plot_reqs.emplace_back("time_ratio_histogram", "", "",  -1,  -1);
  args.plot_reqs.emplace_back("time_ratio_histogram", "", "_zoom1",  0,  5);
  args.plot_reqs.emplace_back("time_ratio_histogram", "", "_zoom2",  0,  2);
  args.plot_reqs.emplace_back("time_ratio_histogram", "", "_zoom3",  0,  1);
  args.plot_reqs.emplace_back("time_ratio_histogram", "", "_zoom4",  0,  0.75);
  args.plot_reqs.emplace_back("time_ratio_histogram", "", "_zoom5",  0,  0.5);

  args.plot_reqs.emplace_back("time_calc_gpu", "", "",  -1,  -1);

  */


  args.togetherplot_reqs.emplace_back("cluster_number_hist", "", "", -1, -1, -1, -1, smallest_num_bins);
  args.togetherplot_reqs.emplace_back("cluster_number_hist", "", "_zoom1", 0, 1800);
  args.togetherplot_reqs.emplace_back("diff_num_hist", "", "", -1, -1);

  args.togetherplot_reqs.emplace_back("diff_num_hist", "", "_zoom1", -10.5, 10.5, -1, -1, 21);

  args.togetherplot_reqs.emplace_back("unmatched_number_hist", "", "", -0.5,  5.5, -1, -1, 6);

/*
  args.togetherplot_reqs.emplace_back("time_histogram", "", "",  -1,  -1, -1, -1, small_num_bins);
  args.togetherplot_reqs.emplace_back("time_histogram", "", "_zoom1",  0,  60000);
  args.togetherplot_reqs.emplace_back("time_histogram", "", "_zoom2",  5000,  60000);

  args.togetherplot_reqs.emplace_back("time_ratio_histogram", "", "",  -1,  -1);
  args.togetherplot_reqs.emplace_back("time_ratio_histogram", "", "_zoom1",  0,  5);
  args.togetherplot_reqs.emplace_back("time_ratio_histogram", "", "_zoom2",  0,  2);
  args.togetherplot_reqs.emplace_back("time_ratio_histogram", "", "_zoom3",  0,  1);
  args.togetherplot_reqs.emplace_back("time_ratio_histogram", "", "_zoom4",  0,  0.75);
  args.togetherplot_reqs.emplace_back("time_ratio_histogram", "", "_zoom5",  0,  0.5);


  args.togetherplot_reqs.emplace_back("time_invratio_histogram", "", "",  -1,  -1);
  args.togetherplot_reqs.emplace_back("time_invratio_histogram", "", "_zoom1",  0,  25);
  args.togetherplot_reqs.emplace_back("time_invratio_histogram", "", "_zoom2",  0,  20);
  args.togetherplot_reqs.emplace_back("time_invratio_histogram", "", "_zoom3",  0,  15);
  args.togetherplot_reqs.emplace_back("time_invratio_histogram", "", "_zoom4",  0,  10);
  args.togetherplot_reqs.emplace_back("time_invratio_histogram", "", "_zoom5",  0,  5);
  args.togetherplot_reqs.emplace_back("time_invratio_histogram", "", "_zoom5",  0,  5);

  args.togetherplot_reqs.emplace_back("time_calc_gpu", "", "",  -1,  -1);


  args.togetherplot_reqs.emplace_back("time_histogram_modified", "", "",  -1,  -1);
*/

  args.togetherplot_reqs.emplace_back("unmatched_E_hist", "", "", -1, -1, -1, -1, smallest_num_bins);
  args.togetherplot_reqs.emplace_back("unmatched_E_hist", "", "_zoom1", -50, 50);
  args.togetherplot_reqs.emplace_back("unmatched_E_hist", "", "_zoom2", -20, 20);
  args.togetherplot_reqs.emplace_back("unmatched_Et_hist", "", "", -1, -1);
  args.togetherplot_reqs.emplace_back("unmatched_Et_hist", "", "_zoom1", -50, 50);
  args.togetherplot_reqs.emplace_back("unmatched_Et_hist", "", "_zoom2", -20, 20);
  args.togetherplot_reqs.emplace_back("unmatched_sizes_hist", "", "", -1, -1);
  args.togetherplot_reqs.emplace_back("unmatched_sizes_hist", "", "_zoom", 0, 1000);
  args.togetherplot_reqs.emplace_back("unmatched_sizes_hist_cumul", "", "", -1, -1);
  args.togetherplot_reqs.emplace_back("unmatched_sizes_hist_cumul", "", "_zoom1", 0, 1000);
  args.togetherplot_reqs.emplace_back("unmatched_sizes_hist_cumul", "", "_zoom2", 0, 500);
  args.togetherplot_reqs.emplace_back("unmatched_sizes_hist_cumul", "", "_zoom3", 0, 250);
  args.togetherplot_reqs.emplace_back("unmatched_sizes_hist_cumul", "", "_zoom4", 0, 100);
  args.togetherplot_reqs.emplace_back("unmatched_eta_hist", "", "", -1, -1);
  args.togetherplot_reqs.emplace_back("unmatched_eta_hist", "", "_full", -7.5, 7.5);
  args.togetherplot_reqs.emplace_back("unmatched_eta_hist", "", "_zoom", -3, 3);
  args.togetherplot_reqs.emplace_back("unmatched_phi_hist", "", "", -1, -1);

  args.togetherplot_reqs.emplace_back("Et_hist", "", "", -1, -1, -1, -1, default_num_bins);
  args.togetherplot_reqs.emplace_back("Et_hist", "", "_zoom1", 0, 200);
  args.togetherplot_reqs.emplace_back("Et_hist", "", "_zoom2", 0, 100);
  args.togetherplot_reqs.emplace_back("E_hist", "", "", -1, -1);
  args.togetherplot_reqs.emplace_back("E_hist", "", "_zoom", -1500, 1500);
  args.togetherplot_reqs.emplace_back("cluster_size_hist", "", "", -1, -1);
  args.togetherplot_reqs.emplace_back("cluster_size_hist", "", "_zoom", 0, 1000);
  args.togetherplot_reqs.emplace_back("cluster_size_hist_cumul", "", "", -1, -1);
  args.togetherplot_reqs.emplace_back("cluster_size_hist_cumul", "", "_zoom1", 0, 1000);
  args.togetherplot_reqs.emplace_back("cluster_size_hist_cumul", "", "_zoom2", 0, 500);
  args.togetherplot_reqs.emplace_back("cluster_size_hist_cumul", "", "_zoom3", 0, 250);
  args.togetherplot_reqs.emplace_back("cluster_size_hist_cumul", "", "_zoom4", 0, 100);
  args.togetherplot_reqs.emplace_back("cluster_eta_hist", "", "", -1, -1);
  args.togetherplot_reqs.emplace_back("cluster_eta_hist", "", "_full", -7.5, 7.5);
  args.togetherplot_reqs.emplace_back("cluster_eta_hist", "", "_zoom0", -50, 50);
  args.togetherplot_reqs.emplace_back("cluster_eta_hist", "", "_zoom1", -10, 10);
  args.togetherplot_reqs.emplace_back("cluster_eta_hist", "", "_zoom2", -3, 3);
  args.togetherplot_reqs.emplace_back("cluster_phi_hist", "", "", -1, -1);
  args.togetherplot_reqs.emplace_back("cluster_phi_hist", "", "_full", -pi<double>, pi<double>);

  args.togetherplot_reqs.emplace_back("delta_R_hist", "", "", -1,  -1);
  args.togetherplot_reqs.emplace_back("delta_R_hist", "", "_zoom1", 0.,  0.25);
  args.togetherplot_reqs.emplace_back("delta_R_hist", "", "_zoom2", 0.,  0.001);
  args.togetherplot_reqs.emplace_back("delta_R_hist", "", "_zoom3", 0.,  0.0001);

  args.togetherplot_reqs.emplace_back("delta_R_hist_cumul", "", "", -1,  -1);
  args.togetherplot_reqs.emplace_back("delta_R_hist_cumul", "", "_zoom1", 0.,  0.25);
  args.togetherplot_reqs.emplace_back("delta_R_hist_cumul", "", "_zoom2", 0.,  0.001);
  args.togetherplot_reqs.emplace_back("delta_R_hist_cumul", "", "_zoom3", 0.,  0.0001);


  args.togetherplot_reqs.emplace_back("diff_cell_per_cluster", "", "", -1, -1, -1, -1, small_num_bins);
  args.togetherplot_reqs.emplace_back("diff_cell_per_cluster", "", "_zoom1", 0, 2500);
  args.togetherplot_reqs.emplace_back("diff_cell_per_cluster", "", "_zoom1p5", 0, 1000);
  args.togetherplot_reqs.emplace_back("diff_cell_per_cluster", "", "_zoom2", 0, 500);
  args.togetherplot_reqs.emplace_back("diff_cell_per_cluster", "", "_zoom2p5", 0, 125);
  args.togetherplot_reqs.emplace_back("diff_cell_per_cluster", "", "_zoom3", 0, 100);

  args.togetherplot_reqs.emplace_back("diff_cell_per_cluster", "", "_zoom4", -0.5, 50.5, -1, -1, 51);

  args.togetherplot_reqs.emplace_back("rel_diff_cell_per_cluster", "", "", -1, -1, -1, -1, small_num_bins);
  args.togetherplot_reqs.emplace_back("rel_diff_cell_per_cluster", "", "_zoom1", 0, 0.5);
  args.togetherplot_reqs.emplace_back("rel_diff_cell_per_cluster", "", "_zoom2", 0., 0.25);
  args.togetherplot_reqs.emplace_back("rel_diff_cell_per_cluster", "", "_zoom3", 0., 0.05);
  args.togetherplot_reqs.emplace_back("rel_diff_cell_per_cluster", "", "_zoom4", 0., 0.01);

  args.togetherplot_reqs.emplace_back("diff_cell_per_cluster_cumul", "", "", -1, -1);
  args.togetherplot_reqs.emplace_back("diff_cell_per_cluster_cumul", "", "_zoom1", 0, 2500);
  args.togetherplot_reqs.emplace_back("diff_cell_per_cluster_cumul", "", "_zoom1p5", 0, 1000);
  args.togetherplot_reqs.emplace_back("diff_cell_per_cluster_cumul", "", "_zoom2", 0, 500);
  args.togetherplot_reqs.emplace_back("diff_cell_per_cluster_cumul", "", "_zoom2p5", 0, 125);
  args.togetherplot_reqs.emplace_back("diff_cell_per_cluster_cumul", "", "_zoom3", 0, 100);

  args.togetherplot_reqs.emplace_back("diff_cell_per_cluster_cumul", "", "_zoom3p5", -0.5, 75.5, -1, -1, 76);

  args.togetherplot_reqs.emplace_back("diff_cell_per_cluster_cumul", "", "_zoom4", -0.5, 50.5, -1, -1, 51);

  args.togetherplot_reqs.emplace_back("diff_cell_per_cluster_cumul", "", "_zoom5", -0.5, 25.5, -1, -1, 26);

  args.togetherplot_reqs.emplace_back("diff_cell_per_cluster_cumul", "", "_zoom6", -0.5, 10.5, -1, -1, 11);

  args.togetherplot_reqs.emplace_back("rel_diff_cell_per_cluster_cumul", "", "", -1, -1, -1, -1, small_num_bins);
  args.togetherplot_reqs.emplace_back("rel_diff_cell_per_cluster_cumul", "", "_zoom1", 0, 0.5);
  args.togetherplot_reqs.emplace_back("rel_diff_cell_per_cluster_cumul", "", "_zoom2", 0., 0.25);
  args.togetherplot_reqs.emplace_back("rel_diff_cell_per_cluster_cumul", "", "_zoom2p5", 0., 0.2);
  args.togetherplot_reqs.emplace_back("rel_diff_cell_per_cluster_cumul", "", "_zoom3", 0., 0.05);
  args.togetherplot_reqs.emplace_back("rel_diff_cell_per_cluster_cumul", "", "_zoom4", 0., 0.01);
  args.togetherplot_reqs.emplace_back("rel_diff_cell_per_cluster_cumul", "", "_zoom5", 0., 0.005);
  args.togetherplot_reqs.emplace_back("rel_diff_cell_per_cluster_cumul", "", "_zoom6", 0., 0.002);

  args.togetherplot_reqs.emplace_back("equal_cells_delta_R", "", "", -1,  -1, -1, -1, default_num_bins);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_R", "", "_zoom1", 0.,  0.25);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_R", "", "_zoom2", 0.,  0.001);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_R", "", "_zoom3", 0.,  0.0001);

  args.togetherplot_reqs.emplace_back("equal_cells_delta_R_cumul", "", "", -1,  -1);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_R_cumul", "", "_zoom1", 0.,  0.25);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_R_cumul", "", "_zoom2", 0.,  0.001);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_R_cumul", "", "_zoom3", 0.,  0.0001);


  args.togetherplot_reqs.emplace_back("equal_cells_delta_eta", "", "", -1,  -1);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_eta", "", "_zoom1", -10,  10);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_eta", "", "_zoom2", -1,  1);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_eta", "", "_zoom3", -0.25,  0.25);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_eta", "", "_zoom4", -0.1,  0.1);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_eta", "", "_zoom5", -0.01,  0.01);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_eta", "", "_zoom6", -0.001,  0.001);


  args.togetherplot_reqs.emplace_back("equal_cells_delta_phi", "", "", -1,  -1);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_phi", "", "_full", -pi<double>,  pi<double>);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_phi", "", "_zoom1", -1,  1);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_phi", "", "_zoom2", -0.1,  0.1);

  args.togetherplot_reqs.emplace_back("equal_cells_delta_E", "", "", -1,  -1);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_E", "", "_zoom1", -1000,  1000);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_E", "", "_zoom2", -500,  500);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_E", "", "_zoom3", -100,  100);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_E", "", "_zoom4", -50,  50);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_E", "", "_zoom5", -10,  10);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_E", "", "_zoom6", -1,  1);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_E", "", "_zoom7", -0.1,  0.1);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_E", "", "_zoom8", -0.01,  0.01);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_E", "", "_zoom9", -0.001,  0.001);

  args.togetherplot_reqs.emplace_back("equal_cells_delta_E_rel", "", "", -1,  -1);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_E_rel", "", "_zoom1", -1,  1);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_E_rel", "", "_zoom2", -0.5,  0.5);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_E_rel", "", "_zoom3", -0.1,  0.1);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_E_rel", "", "_zoom4", -0.01,  0.01);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_E_rel", "", "_zoom5", -0.001,  0.001);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_E_rel", "", "_zoom5p5", -0.0004,  0.0004);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_E_rel", "", "_zoom6", -0.0001,  0.0001);

  args.togetherplot_reqs.emplace_back("equal_cells_delta_Et", "", "", -1,  -1);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_Et", "", "_zoom1", -1000,  1000);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_Et", "", "_zoom2", -500,  500);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_Et", "", "_zoom3", -100,  100);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_Et", "", "_zoom4", -50,  50);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_Et", "", "_zoom5", -10,  10);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_Et", "", "_zoom6", -1,  1);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_Et", "", "_zoom7", -0.1,  0.1);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_Et", "", "_zoom8", -0.01,  0.01);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_Et", "", "_zoom9", -0.001,  0.001);

  args.togetherplot_reqs.emplace_back("equal_cells_delta_Et_rel", "", "", -1,  -1);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_Et_rel", "", "_zoom1", -1,  1);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_Et_rel", "", "_zoom2", -0.5,  0.5);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_Et_rel", "", "_zoom3", -0.1,  0.1);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_Et_rel", "", "_zoom4", -0.01,  0.01);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_Et_rel", "", "_zoom5", -0.001,  0.001);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_Et_rel", "", "_zoom5p5", -0.0002,  0.0002);
  args.togetherplot_reqs.emplace_back("equal_cells_delta_Et_rel", "", "_zoom6", -0.0001,  0.0001);

  args.togetherplot_reqs.emplace_back("delta_eta_hist", "", "", -1,  -1);
  args.togetherplot_reqs.emplace_back("delta_eta_hist", "", "_zoom1", -10,  10);
  args.togetherplot_reqs.emplace_back("delta_eta_hist", "", "_zoom3", -1,  1);
  args.togetherplot_reqs.emplace_back("delta_eta_hist", "", "_zoom3", -0.25,  0.25);
  args.togetherplot_reqs.emplace_back("delta_eta_hist", "", "_zoom4", -0.1,  0.1);
  args.togetherplot_reqs.emplace_back("delta_eta_hist", "", "_zoom5", -0.01,  0.01);
  args.togetherplot_reqs.emplace_back("delta_eta_hist", "", "_zoom6", -0.001,  0.001);

  args.togetherplot_reqs.emplace_back("delta_phi_hist", "", "", -1,  -1);
  args.togetherplot_reqs.emplace_back("delta_phi_hist", "", "_full", -pi<double>,  pi<double>);

  args.togetherplot_reqs.emplace_back("delta_E_hist", "", "", -1,  -1);
  args.togetherplot_reqs.emplace_back("delta_E_hist", "", "_zoom1", -1000,  1000);
  args.togetherplot_reqs.emplace_back("delta_E_hist", "", "_zoom2", -500,  500);
  args.togetherplot_reqs.emplace_back("delta_E_hist", "", "_zoom2p5", -200,  200);
  args.togetherplot_reqs.emplace_back("delta_E_hist", "", "_zoom3", -100,  100);
  args.togetherplot_reqs.emplace_back("delta_E_hist", "", "_zoom4", -50,  50);
  args.togetherplot_reqs.emplace_back("delta_E_hist", "", "_zoom5", -10,  10);
  args.togetherplot_reqs.emplace_back("delta_E_hist", "", "_zoom6", -1,  1);
  args.togetherplot_reqs.emplace_back("delta_E_hist", "", "_zoom7", -0.1,  0.1);
  args.togetherplot_reqs.emplace_back("delta_E_hist", "", "_zoom8", -0.01,  0.01);
  args.togetherplot_reqs.emplace_back("delta_E_hist", "", "_zoom9", -0.001,  0.001);

  args.togetherplot_reqs.emplace_back("delta_Et_hist", "", "", -1,  -1);
  args.togetherplot_reqs.emplace_back("delta_Et_hist", "", "_zoom1", -1000,  1000);
  args.togetherplot_reqs.emplace_back("delta_Et_hist", "", "_zoom2", -500,  500);
  args.togetherplot_reqs.emplace_back("delta_Et_hist", "", "_zoom3", -100,  100);
  args.togetherplot_reqs.emplace_back("delta_Et_hist", "", "_zoom4", -50,  50);
  args.togetherplot_reqs.emplace_back("delta_Et_hist", "", "_zoom4p5", -20,  20);
  args.togetherplot_reqs.emplace_back("delta_Et_hist", "", "_zoom5", -10,  10);
  args.togetherplot_reqs.emplace_back("delta_Et_hist", "", "_zoom6", -1,  1);
  args.togetherplot_reqs.emplace_back("delta_Et_hist", "", "_zoom7", -0.1,  0.1);
  args.togetherplot_reqs.emplace_back("delta_Et_hist", "", "_zoom8", -0.01,  0.01);
  args.togetherplot_reqs.emplace_back("delta_Et_hist", "", "_zoom9", -0.001,  0.001);


  args.togetherplot_reqs.emplace_back("delta_E_rel_hist", "", "", -1,  -1);
  args.togetherplot_reqs.emplace_back("delta_E_rel_hist", "", "_zoom0", -1000,  1000);
  args.togetherplot_reqs.emplace_back("delta_E_rel_hist", "", "_zoom1", -100,  100);
  args.togetherplot_reqs.emplace_back("delta_E_rel_hist", "", "_zoom2", -10,  10);
  args.togetherplot_reqs.emplace_back("delta_E_rel_hist", "", "_zoom3", -1,  1);
  args.togetherplot_reqs.emplace_back("delta_E_rel_hist", "", "_zoom4", -0.5,  0.5);
  args.togetherplot_reqs.emplace_back("delta_E_rel_hist", "", "_zoom5", -0.1,  0.1);
  args.togetherplot_reqs.emplace_back("delta_E_rel_hist", "", "_zoom6", -0.01,  0.01);
  args.togetherplot_reqs.emplace_back("delta_E_rel_hist", "", "_zoom7", -0.001,  0.001);
  args.togetherplot_reqs.emplace_back("delta_E_rel_hist", "", "_zoom8", -0.0001,  0.0001);

  args.togetherplot_reqs.emplace_back("delta_Et_rel_hist", "", "", -1,  -1);
  args.togetherplot_reqs.emplace_back("delta_Et_rel_hist", "", "_zoom0", -1000,  1000);
  args.togetherplot_reqs.emplace_back("delta_Et_rel_hist", "", "_zoom1", -100,  100);
  args.togetherplot_reqs.emplace_back("delta_Et_rel_hist", "", "_zoom2", -10,  10);
  args.togetherplot_reqs.emplace_back("delta_Et_rel_hist", "", "_zoom3", -1,  1);
  args.togetherplot_reqs.emplace_back("delta_Et_rel_hist", "", "_zoom4", -0.5,  0.5);
  args.togetherplot_reqs.emplace_back("delta_Et_rel_hist", "", "_zoom5", -0.1,  0.1);
  args.togetherplot_reqs.emplace_back("delta_Et_rel_hist", "", "_zoom6", -0.01,  0.01);
  args.togetherplot_reqs.emplace_back("delta_Et_rel_hist", "", "_zoom7", -0.001,  0.001);
  args.togetherplot_reqs.emplace_back("delta_Et_rel_hist", "", "_zoom8", -0.0001,  0.0001);
}


void set_default_time_plots(ProgArgs & args)
{
  [[maybe_unused]] constexpr int default_num_bins = 250;
  [[maybe_unused]] constexpr int shorter_num_bins = 100;
  [[maybe_unused]] constexpr int small_num_bins = 50;
  [[maybe_unused]] constexpr int smallest_num_bins = 25;

  args.timeplot_reqs.emplace_back("time_dist_*", "", "", -1, -1);

  args.timeplot_reqs.emplace_back("time_frac_*", "", "", -1, -1);

  args.timeplot_reqs.emplace_back("time_ratio_*", "", "", -1, -1);

  args.timeplot_reqs.emplace_back("time_speedup_*", "", "", -1, -1);

  args.timetogetherplot_reqs.emplace_back("time_global_*", "", "", -1, -1);
  
  /*
  args.timetogetherplot_reqs.emplace_back("time_global_frac*", "", "_zoom1", 0, 1);
  args.timetogetherplot_reqs.emplace_back("time_global_frac*", "", "_zoom2", 0, 0.5);
  args.timetogetherplot_reqs.emplace_back("time_global_frac*", "", "_zoom3", 0, 0.25);
  args.timetogetherplot_reqs.emplace_back("time_global_frac*", "", "_zoom4", 0, 0.125);*/
  
}

bool setup_arguments(ProgArgs & args, size_t argc, char ** argv)
{
  args.reset();

  if (argc < 2)
    {
      std::cout << "Expected arguments: <list of input dirs ...>\n"
                "Optional arguments: [-o <output dir>]\n"
                "                    [-s <maximum similarity>]\n"
                "                    [-w <term cell weight> <grow cell weight> <seed cell weight>]\n"
                "                    [-n <list of plotter names>]\n"
                "                    [-t {for plots with titles}]\n"
                "                    [-l <plot labels>]\n"
                "                    [-i <line-separated file with separate plots> (<line-separated file with together plots>)]\n"
                "                    [-u <line-separated file with separate time plots> (<line-separated file with time together plots>)]\n"
                "                    [-r {to renormalize histograms with the number of events}]\n"
                "                    [-c {to skip filling anything other than cell information}]\n"
                "                    [-b {to use the default style rather than ATLAS}]\n"
                "                    [-f <required string in event names>]\n"
                "                    [-e <max events to load (< 0 for unlimited)>]\n"
                "                    [-j {for emitting just .png output}]\n"
                "                    [-x {for disabling the cluster plots}]\n"
                "                    [-z {for disabling the time plots}]\n"
                "                    [-d <constant folder> <reference results folder> <test results folder>]\n"
                << std::endl;
      return true;
    }

  size_t arg = 1;

  bool specified_plot_files = false;
  bool specified_time_plot_files = false;

  for (; arg < argc; ++arg)
    {
      std::string text(argv[arg]);
      if (text.size() > 0 && text[0] == '-')
        //Not the most elegant solution, but eh...
        {
          break;
        }
      if (!boost::filesystem::exists(text))
        {
          std::cout << "ERROR: Input directory '" << text << "' does not exist!" << std::endl;
          return true;
        }
      args.in_paths.push_back(text);
    }
  for (; arg < argc; ++arg)
    {
      if (!std::strcmp(argv[arg], "-o"))
        {
          if (argc > arg + 1)
            {
              ++arg;
              args.out_path = argv[arg];
            }
          else
            {
              std::cout << "ERROR: '-o' provided without specifying output dir!" << std::endl;
              return true;
            }
        }
      else if (!std::strcmp(argv[arg], "-s"))
        {
          if (argc > arg + 1)
            {
              ++arg;
              args.min_similarity = std::atof(argv[arg]);
            }
          else
            {
              std::cout << "ERROR: '-s' provided without specifying minimum similarity!" << std::endl;
              return true;
            }
        }
      else if (!std::strcmp(argv[arg], "-w"))
        {
          if (argc > arg + 3)
            {
              args.term_weight = std::atof(argv[arg + 1]);
              args.grow_weight = std::atof(argv[arg + 2]);
              args.seed_weight = std::atof(argv[arg + 3]);
              arg += 3;
            }
          else
            {
              std::cout << "ERROR: '-w' provided without properly specifying the weights!" << std::endl;
              return true;
            }
        }
      else if (!std::strcmp(argv[arg], "-n"))
        {
          if (argc > arg + args.in_paths.size())
            {
              ++arg;
              for (size_t i = 0; i < args.in_paths.size() && arg < argc; ++i, ++arg)
                {
                  args.names.emplace_back(argv[arg]);
                }
            }
          else
            {
              std::cout << "ERROR: '-n' provided with an insufficient number of plots!" << std::endl;
              return true;
            }
        }
      else if (!std::strcmp(argv[arg], "-t"))
        {
          args.place_titles = true;
        }
      else if (!std::strcmp(argv[arg], "-l"))
        {
          if (argc > arg + 1)
            {
              ++arg;
              args.plot_label = argv[arg];
            }
          else
            {
              std::cout << "ERROR: '-l' provided without specifying plot label!" << std::endl;
              return true;
            }
        }
      else if (!std::strcmp(argv[arg], "-i"))
        {
          if (argc > arg + 1)
            {
              ++arg;
              read_plots_file(argv[arg], args.plot_reqs);
              if (argc > arg + 1)
                {
                  ++arg;
                  read_plots_file(argv[arg], args.togetherplot_reqs);
                }
              specified_plot_files = true;
            }
          else
            {
              std::cout << "ERROR: '-i' provided without specifying valid plot files!" << std::endl;
              return true;
            }
        }
      else if (!std::strcmp(argv[arg], "-u"))
        {
          if (argc > arg + 1)
            {
              ++arg;
              read_plots_file(argv[arg], args.timeplot_reqs);
              if (argc > arg + 1)
                {
                  ++arg;
                  read_plots_file(argv[arg], args.timetogetherplot_reqs);
                }
              specified_time_plot_files = true;
            }
          else
            {
              std::cout << "ERROR: '-u' provided without specifying valid time plot files!" << std::endl;
              return true;
            }
        }
      else if (!std::strcmp(argv[arg], "-r"))
        {
          args.normalize_hists = true;
        }
      else if (!std::strcmp(argv[arg], "-c"))
        {
          args.skip_almost_everything = true;
        }
      else if (!std::strcmp(argv[arg], "-b"))
        {
          args.use_ATLAS_style = false;
        }
      if (!std::strcmp(argv[arg], "-f"))
        {
          if (argc > arg + 1)
            {
              ++arg;
              args.file_filter = argv[arg];
            }
          else
            {
              std::cout << "ERROR: '-f' provided without specifying file filter!" << std::endl;
              return true;
            }
        }
      else if (!std::strcmp(argv[arg], "-e"))
        {
          if (argc > arg + 1)
            {
              ++arg;
              args.max_events = std::atoi(argv[arg]);
            }
          else
            {
              std::cout << "ERROR: '-e' provided without specifying max number of events!" << std::endl;
              return true;
            }
        }
      else if (!std::strcmp(argv[arg], "-j"))
        {
          args.just_png = true;
        }
      else if (!std::strcmp(argv[arg], "-x"))
        {
          args.do_clusters = false;
        }
      else if (!std::strcmp(argv[arg], "-z"))
        {
          args.do_times = false;
        }
      else if (!std::strcmp(argv[arg], "-d"))
        {
          if (argc > arg + 3)
            {
              args.constant_foldername = argv[arg + 1];
              args.reference_foldername = argv[arg + 2];
              args.test_foldername = argv[arg + 3];
              arg += 3;
            }
          else
            {
              std::cout << "ERROR: '-d' provided without properly specifying the folders!" << std::endl;
              return true;
            }
        }
    }

  if (!boost::filesystem::exists(args.out_path))
    {
      if (boost::filesystem::create_directories(args.out_path))
        {
          std::cout << "WARNING: Output path '" << args.out_path << "' did not exist and was created!" << std::endl;
        }
      else
        {
          std::cout << "ERROR: Output path '" << args.out_path << "' did not exist and couldn't be created!" << std::endl;
          return true;
        }
    }

  BasePlotter::place_title = args.place_titles;
  BasePlotter::normalize = args.normalize_hists;
  BasePlotter::place_ATLAS_label = args.use_ATLAS_style;

  if (!specified_plot_files)
    {
      set_default_plots(args);
    }

  if (!specified_time_plot_files)
    {
      set_default_time_plots(args);
    }

  return false;
}




void do_plots (const ProgArgs & args)
{

  std::cout << "Plotting:\n";
  for (size_t i = 0; i < args.in_paths.size(); ++i)
    {
      std::string name = (args.names.size() > 0 ? args.names[i] : "");
      for (int k = name.size(); k < 32; ++k)
        {
          name += " ";
        }
      std::cout << "          " << name << " " << args.in_paths[i] << "\n";
    }
  std::cout << std::endl;

  std::list<ClusterPlotter> plotters;
  std::list<TimePlotter> time_plotters;

  /*
  if (args.do_clusters)
    {
      plotters.reserve(args.in_paths.size());
    }
  if (args.do_times)
    {
      time_plotters.reserve(args.in_paths.size());
    }
  */
  //No need to reserve with lists,
  //as pointer stability is ensured.
  //In fact, no *way* to reserve with lists.

  TogetherPlot together, time_together;

  for (const auto & in_path : args.in_paths)
    {
      if (args.do_clusters)
        {
          plotters.emplace_back(in_path + "/" + args.constant_foldername,
                                in_path + "/" + args.reference_foldername,
                                in_path + "/" + args.test_foldername,
                                args.max_events,
                                args.file_filter,
                                args.min_similarity,
                                args.term_weight,
                                args.grow_weight,
                                args.seed_weight);
        }
      if (args.do_times)
        {
          time_plotters.emplace_back(in_path);
        }

      if (args.names.size() > 0 && args.names.size() >= plotters.size())
        {
          if (args.do_clusters)
            {
              plotters.back().plotter_name = args.names[plotters.size() - 1];
            }
          if (args.do_times)
            {
              time_plotters.back().plotter_name = args.names[time_plotters.size() - 1];
            }
        }

      auto set_single_properties = [&](BasePlotter & ptr)
      {
        if (!args.use_ATLAS_style)
          {
            ptr.Styles[0].line.color = 1;
            ptr.Styles[1].line.color = 1;
            ptr.Styles[2].line.color = 1;
            
            ptr.Styles[0].fill.style = 3004;
            ptr.Styles[1].fill.style = 3005;
            ptr.Styles[2].fill.style = 1001;
            
            ptr.Styles[0].fill.color = kBlue;
            ptr.Styles[1].fill.color = kRed;
            ptr.Styles[2].fill.color = kGray + 1;
            
            std::cout << "Set custom style successfully." << std::endl;
          }
        if (args.just_png)
          {
            ptr.file_extensions = {"png"};
            ptr.print_options = {""};
          }
      };

      if (args.do_clusters)
        {
          set_single_properties(plotters.back());
        }
      if (args.do_times)
        {
          set_single_properties(time_plotters.back());
        }

      const std::string out_dir = args.out_path + "/plots_" + std::to_string(std::max(plotters.size(), time_plotters.size()));


      if (!boost::filesystem::exists(out_dir))
        {
          if (boost::filesystem::create_directories(out_dir))
            {
              std::cout << "WARNING: Output path '" << out_dir << "' did not exist and was created!" << std::endl;
            }
          else
            {
              std::cout << "ERROR: Output path '" << out_dir << "' did not exist and couldn't be created!" << std::endl;
              continue;
            }
        }

      if (args.do_clusters)
        {
          std::cout << "\n              Plotting " << plotters.back().plotter_name << " from '" << in_path << "' to '" << out_dir << "'\n" << std::endl;
        }
      else if (args.do_times)
        {
          std::cout << "\n              Plotting " << time_plotters.back().plotter_name << " from '" << in_path << "' to '" << out_dir << "'\n" << std::endl;
        }

      auto actually_plot = [&](BasePlotter & plotter, const std::vector<PlotDefs> & plot_reqs)
      {
        plotter.plot_label = args.plot_label;
        const int prev_num_bins = plotter.num_bins;

        for (const auto & el : plot_reqs)
          {
            if (el.bin_size > 0)
              {
                plotter.num_bins = el.bin_size;
              }
            if (el.labelx > 0)
              {
                BasePlotter::labelstart_x = el.labelx;
              }
            if (el.labely > 0)
              {
                BasePlotter::labelstart_y = el.labely;
              }
            plotter.plot(el.name, el.xmin, el.xmax, el.ymin, el.ymax, out_dir, el.pref, el.suf);
          }
        plotter.num_bins = prev_num_bins;
      };

      if (args.do_clusters)
        {
          actually_plot(plotters.back(), args.plot_reqs);
        }
      if (args.do_times)
        {
          actually_plot(time_plotters.back(), args.timeplot_reqs);
        }
    }

  auto set_together_style = [&](auto & plt_list)
  {
    if (args.use_ATLAS_style)
      {
        plt_list.front().Styles[0].line.color = kOrange + 3;
        plt_list.front().Styles[0].line.style = 3;
        plt_list.front().Styles[1].line.color = kRed;
        plt_list.front().Styles[1].line.style = 1;
        plt_list.front().Styles[2].line.color = kRed;
        plt_list.front().Styles[2].line.style = 1;

        plt_list.back().Styles[0].line.color = kCyan - 5;
        plt_list.back().Styles[0].line.style = 4;
        plt_list.back().Styles[1].line.color = kBlue;
        plt_list.back().Styles[1].line.style = 2;
        plt_list.back().Styles[2].line.color = kBlue;
        plt_list.back().Styles[2].line.style = 2;
      }
    else
      {
        plt_list.front().Styles[0].fill.color = kOrange + 3;
        plt_list.front().Styles[1].fill.color = kRed;
        plt_list.front().Styles[2].fill.color = kRed;

        plt_list.back().Styles[0].fill.color = kCyan - 5;
        plt_list.back().Styles[1].fill.color = kBlue;
        plt_list.back().Styles[2].fill.color = kBlue;

        plt_list.front().Styles[0].fill.style = 3007;
        plt_list.front().Styles[1].fill.style = 3004;
        plt_list.front().Styles[2].fill.style = 3004;

        plt_list.back().Styles[0].fill.style = 3006;
        plt_list.back().Styles[1].fill.style = 3005;
        plt_list.back().Styles[2].fill.style = 3005;


        plt_list.front().Styles[0].marker.color = kOrange + 3;
        plt_list.front().Styles[1].marker.color = kRed;
        plt_list.front().Styles[2].marker.color = kRed;
        plt_list.front().Styles[0].marker.style = 20;
        plt_list.front().Styles[1].marker.style = 20;
        plt_list.front().Styles[2].marker.style = 20;

        plt_list.back().Styles[0].marker.color = kCyan - 5;
        plt_list.back().Styles[1].marker.color = kBlue;
        plt_list.back().Styles[2].marker.color = kBlue;
        plt_list.back().Styles[0].marker.style = 21;
        plt_list.back().Styles[1].marker.style = 21;
        plt_list.back().Styles[2].marker.style = 21;

      }
  };

  if (args.do_clusters)
    {
      set_together_style(plotters);
      for (const auto & plotter : plotters)
        {
          together.add_plot(&plotter);
        }
    }
  if (args.do_times)
    {
      set_together_style(time_plotters);
      for (const auto & time_plotter : time_plotters)
        {
          time_together.add_plot(&time_plotter);
        }
    }

  std::cout << "\n              Plotting together.\n" << std::endl;

  auto actually_do_plots = [&](auto & together_plt, auto & plt_list, auto & plot_reqs)
  {
    const int prev_num_bins_front = plt_list.front().num_bins;
    const int prev_num_bins_back = plt_list.back().num_bins;

    for (const auto & el : plot_reqs)
      {
        if (el.bin_size > 0)
          {
            plt_list.front().num_bins = el.bin_size;
            plt_list.back().num_bins = el.bin_size;
          }
        together_plt.plot(el.name, el.xmin, el.xmax, el.ymin, el.ymax, args.out_path, el.pref, el.suf);
      }

    plt_list.front().num_bins = prev_num_bins_front;
    plt_list.back().num_bins = prev_num_bins_back;
  };

  if (args.do_clusters)
    {
      actually_do_plots(together, plotters, args.togetherplot_reqs);
    }
  if (args.do_times)
    {
      actually_do_plots(time_together, time_plotters, args.timetogetherplot_reqs);
    }

  std::cout << "\n              Done!\n" << std::endl;

}


std::unique_ptr<TStyle> CreateAtlasStyle()
{
  std::unique_ptr<TStyle> atlasStyle = std::make_unique<TStyle>("ATLAS", "Atlas style");

  // use plain black on white colors
  Int_t icol = 0; // WHITE
  atlasStyle->SetFrameBorderMode(icol);
  atlasStyle->SetFrameFillColor(icol);
  atlasStyle->SetCanvasBorderMode(icol);
  atlasStyle->SetCanvasColor(icol);
  atlasStyle->SetPadBorderMode(icol);
  atlasStyle->SetPadColor(icol);
  atlasStyle->SetStatColor(icol);
  //atlasStyle->SetFillColor(icol); // don't use: white fill color for *all* objects

  // set the paper & margin sizes
  atlasStyle->SetPaperSize(20, 26);

  // set margin sizes
  atlasStyle->SetPadTopMargin(0.05);
  atlasStyle->SetPadRightMargin(0.05);
  atlasStyle->SetPadBottomMargin(0.16);
  atlasStyle->SetPadLeftMargin(0.16);

  // set title offsets (for axis label)
  atlasStyle->SetTitleXOffset(1.4);
  atlasStyle->SetTitleYOffset(1.4);

  // use large fonts
  //Int_t font=72; // Helvetica italics
  Int_t font = 42; // Helvetica
  Double_t tsize = 0.05;
  atlasStyle->SetTextFont(font);

  atlasStyle->SetTextSize(tsize);
  atlasStyle->SetLabelFont(font, "x");
  atlasStyle->SetTitleFont(font, "x");
  atlasStyle->SetLabelFont(font, "y");
  atlasStyle->SetTitleFont(font, "y");
  atlasStyle->SetLabelFont(font, "z");
  atlasStyle->SetTitleFont(font, "z");

  atlasStyle->SetLabelSize(tsize, "x");
  atlasStyle->SetTitleSize(tsize, "x");
  atlasStyle->SetLabelSize(tsize, "y");
  atlasStyle->SetTitleSize(tsize, "y");
  atlasStyle->SetLabelSize(tsize, "z");
  atlasStyle->SetTitleSize(tsize, "z");

  // use bold lines and markers
  atlasStyle->SetMarkerStyle(20);
  atlasStyle->SetMarkerSize(1.2);
  atlasStyle->SetHistLineWidth(2.);
  atlasStyle->SetLineStyleString(2, "[12 12]"); // postscript dashes

  // get rid of X error bars (as recommended in ATLAS figure guidelines)
  atlasStyle->SetErrorX(0.0001);
  // get rid of error bar caps
  atlasStyle->SetEndErrorSize(0.);

  // do not display any of the standard histogram decorations
  atlasStyle->SetOptTitle(0);
  //atlasStyle->SetOptStat(1111);
  atlasStyle->SetOptStat(0);
  //atlasStyle->SetOptFit(1111);
  atlasStyle->SetOptFit(0);

  // put tick marks on top and RHS of plots
  atlasStyle->SetPadTickX(1);
  atlasStyle->SetPadTickY(1);

  return atlasStyle;

}

int main(int argc, char ** argv)
{

  TH1::AddDirectory(false);
  ProgArgs args;

  if (setup_arguments(args, argc, argv))
    {
      return 1;
    }

  std::unique_ptr<TStyle> atlasStyle = CreateAtlasStyle();

  if (args.use_ATLAS_style)
    {
      gROOT->SetStyle("ATLAS");
      gROOT->ForceStyle();
    }

  do_plots(args);
  //"Maker"

  //do_plots(args, "ClusterSplitter_", "Splitter");
  //"Splitter
  //do_plots(args, "", "");
  //Neither "Splitter" nor "Maker"

  return 0;

}
