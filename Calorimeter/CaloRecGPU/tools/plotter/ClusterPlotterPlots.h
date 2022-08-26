// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
//

#ifndef CALORECGPU_TOOLS_CLUSTERPLOTTERPLOTS_H
#define CALORECGPU_TOOLS_CLUSTERPLOTTERPLOTS_H

#include "CxxUtils/checker_macros.h"

#ifdef ATLAS_CHECK_THREAD_SAFETY

  ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#endif

void ClusterPlotter::populate_plots()
{

  std::cout << "Started populating plots." << std::endl;

#ifndef RETFUNC
#define RETFUNC(...) [&]([[maybe_unused]] const int i){ return double(__VA_ARGS__); }
#endif

  add_plot<H1D_plotter_region>(
          "diff_num_hist",
          RETFUNC(min_cluster_diff[i]),
          RETFUNC(max_cluster_diff[i]),
          true, 0, "Difference in Number of Clusters Per Event", test_name + " Clusters - " + ref_name + " Clusters", (normalize ? "Fraction of Events" : "Events"),
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {

        group->global->Fill( ev.cluster_diff[0] );
        group->t1->Fill( ev.cluster_diff[1] );
        group->t2->Fill( ev.cluster_diff[2] );
        group->t3->Fill( ev.cluster_diff[3] );
      }

  },
  StyleKinds::joined);

  add_plot<H1D_plotter_region>(
          "diff_num_rel_hist",
          RETFUNC(double(min_cluster_diff[i]) / double(max_cluster_number[i])),
          RETFUNC(double(max_cluster_diff[i]) / double(min_cluster_number[i])),
          true, 0, "Relative Difference in Number of Clusters Per Event", "#(){" + test_name + " Clusters - " + ref_name + " Clusters}/" + ref_name + " Clusters", (normalize ? "Fraction of Events" : "Events"),
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {

        group->global->Fill( double(ev.cluster_diff[0])  /
                             double(ev.test_has_more[0] ? ev.min_cluster_number[0] : ev.max_cluster_number[0] ) );
        group->t1->Fill( double(ev.cluster_diff[1])  /
                         double(ev.test_has_more[1] ? ev.min_cluster_number[1] : ev.max_cluster_number[1] ) );
        group->t2->Fill( double(ev.cluster_diff[2])  /
                         double(ev.test_has_more[2] ? ev.min_cluster_number[2] : ev.max_cluster_number[2] ) );
        group->t3->Fill( double(ev.cluster_diff[3])  /
                         double(ev.test_has_more[3] ? ev.min_cluster_number[3] : ev.max_cluster_number[3] ) );
      }

  },
  StyleKinds::joined);

  add_plot<H1D_plotter_region>(
          "frac_cell_diff_reg_hist",
          RETFUNC(0),
          RETFUNC(1),
          false, 0, "Fraction of Cells With Different Clusters Per Region", "Fraction", "Counts",
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {

        group->global->Fill( ev.cell_diff_frac_reg[0] );
        group->t1->Fill( ev.cell_diff_frac_reg[1] );
        group->t2->Fill( ev.cell_diff_frac_reg[2] );
        group->t3->Fill( ev.cell_diff_frac_reg[3] );
      }

  },
  StyleKinds::joined);

  add_plot<H1D_plotter_type>(
          "frac_cell_diff_type_hist",
          RETFUNC(0),
          RETFUNC(1),
          false, 0, "Fraction of Cells With Different Clusters Per Type", "Fraction", "Counts",
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {

        group->global->Fill( ev.cell_diff_frac_type[0] );

        group->t1->Fill( ev.cell_diff_frac_type[1] );

        group->t2->Fill( ev.cell_diff_frac_type[2] );

        group->t3->Fill( ev.cell_diff_frac_type[3] );
      }

  },
  StyleKinds::joined);


  add_plot<joined_plotter_type>("diff_cell_per_cluster", "Differently Assigned Cells Per Cluster", "Number of Differently Assigned Cells", (normalize ? "Average Number of Clusters per Event" : "Clusters"),
                                add_plot<H1D_plotter_type>("diff_cell_per_cluster_ref",
                                                           RETFUNC(0),
                                                           RETFUNC(10000),
                                                           false, 0, std::string("Differently Assigned Cells Per Cluster") + " (" + ref_name + ")", "Number of Differently Assigned Cells", (normalize ? "Average Number of Clusters per Event" : "Clusters"),
                                                           [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        std::vector<int> count[4];

        for (int i = 0; i < 4; ++i)
          {
            count[i].resize(ev.ref_clusters.size(), 0);
          }

        for (int i = 0; i < NCaloCells; ++i)
          {
            const int ref_idx = ev.ref_tag_array[i];
            const int test_idx = ev.test_tag_array[i];
            const float this_SNR = ev.SNR_array[i];
            if (ref_idx >= 0 && ev.r2t_matches[ref_idx] != test_idx)
              {
                operate_on_types(count, this_SNR,
                                 [&](std::vector<int> * vec)
                {
                  (*vec)[ref_idx] += 1;
                });
              }
          }
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            group->global->Fill(count[0][i]);
            group->t1->Fill(count[1][i]);
            group->t2->Fill(count[2][i]);
            group->t3->Fill(count[3][i]);
          }
      }

  },
  StyleKinds::ref),
  ref_name,
  add_plot<H1D_plotter_type>("diff_cell_per_cluster_test",
                             RETFUNC(0),
                             RETFUNC(10000),
                             false, 0, std::string("Differently Assigned Cells Per Cluster") + " (" + test_name + ")", "Number of Differently Assigned  Cells", (normalize ? "Average Number of Clusters per Event" : "Clusters"),
                             [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        std::vector<int> count[4];

        for (int i = 0; i < 4; ++i)
          {
            count[i].resize(ev.test_clusters.size(), 0);
          }

        for (int i = 0; i < NCaloCells; ++i)
          {
            const int ref_idx = ev.ref_tag_array[i];
            const int test_idx = ev.test_tag_array[i];
            const float this_SNR = ev.SNR_array[i];
            if (test_idx >= 0 && ev.t2r_matches[test_idx] != ref_idx)
              {
                operate_on_types(count, this_SNR,
                                 [&](std::vector<int> * vec)
                {
                  (*vec)[test_idx] += 1;
                });
              }
          }
        for (size_t i = 0; i < ev.test_clusters.size(); ++i)
          {
            group->global->Fill(count[0][i]);
            group->t1->Fill(count[1][i]);
            group->t2->Fill(count[2][i]);
            group->t3->Fill(count[3][i]);
          }
      }

  },
  StyleKinds::test),
  test_name);



  add_plot<joined_plotter_type>("rel_diff_cell_per_cluster", "Fraction of Differently Assigned Cells Per Cluster", "Differently Assigned Cells/Cluster Size", "Counts",
                                add_plot<H1D_plotter_type>("rel_diff_cell_per_cluster_ref",
                                                           RETFUNC(0),
                                                           RETFUNC(1),
                                                           false, 0, std::string("Fraction of Differently Assigned Cells Per Cluster") + " (" + ref_name + ")", "Differently Assigned Cells/Cluster Size", "Counts",
                                                           [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        std::vector<int> count[4];

        for (int i = 0; i < 4; ++i)
          {
            count[i].resize(ev.ref_clusters.size(), 0);
          }

        for (int i = 0; i < NCaloCells; ++i)
          {
            const int ref_idx = ev.ref_tag_array[i];
            const int test_idx = ev.test_tag_array[i];
            const float this_SNR = ev.SNR_array[i];
            if (ref_idx >= 0 && ev.r2t_matches[ref_idx] != test_idx)
              {
                operate_on_types(count, this_SNR,
                                 [&](std::vector<int> * vec)
                {
                  (*vec)[ref_idx] += 1;
                });
              }
          }
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            if (ev.ref_clusters[i].num_cells > min_size_cut)
              {
                group->global->Fill(double(count[0][i]) / ev.ref_clusters[i].num_cells);
                group->t1->Fill(double(count[1][i]) / ev.ref_clusters[i].num_cells);
                group->t2->Fill(double(count[2][i]) / ev.ref_clusters[i].num_cells);
                group->t3->Fill(double(count[3][i]) / ev.ref_clusters[i].num_cells);
              }
          }
      }

  },
  StyleKinds::ref),
  ref_name,
  add_plot<H1D_plotter_type>("rel_diff_cell_per_cluster_test",
                             RETFUNC(0),
                             RETFUNC(1),
                             false, 0, std::string("Fraction of Differently Assigned Cells Per Cluster") + " (" + test_name + ")", "Differently Assigned Cells/Cluster Size", "Counts",
                             [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        std::vector<int> count[4];

        for (int i = 0; i < 4; ++i)
          {
            count[i].resize(ev.test_clusters.size(), 0);
          }

        for (int i = 0; i < NCaloCells; ++i)
          {
            const int ref_idx = ev.ref_tag_array[i];
            const int test_idx = ev.test_tag_array[i];
            const float this_SNR = ev.SNR_array[i];
            if (test_idx >= 0 && ev.t2r_matches[test_idx] != ref_idx)
              {
                operate_on_types(count, this_SNR,
                                 [&](std::vector<int> * vec)
                {
                  (*vec)[test_idx] += 1;
                });
              }
          }
        for (size_t i = 0; i < ev.test_clusters.size(); ++i)
          {
            if (ev.test_clusters[i].num_cells > min_size_cut)
              {
                group->global->Fill(double(count[0][i]) / ev.test_clusters[i].num_cells);
                group->t1->Fill(double(count[1][i]) / ev.test_clusters[i].num_cells);
                group->t2->Fill(double(count[2][i]) / ev.test_clusters[i].num_cells);
                group->t3->Fill(double(count[3][i]) / ev.test_clusters[i].num_cells);
              }
          }
      }

  },
  StyleKinds::test),
  test_name);



  add_plot<joined_plotter_type>("diff_cell_per_cluster_cumul", "Fraction of Clusters With More Differently Assigned Cells", "Differently Assigned Cells", "Fraction of Clusters With More Differences",
                                add_plot<H1D_plotter_type>("diff_cell_per_cluster_ref_cumul",
                                                           RETFUNC(0),
                                                           RETFUNC(10000),
                                                           false, -1, std::string("Differently Assigned Cells Per Cluster") + " (" + ref_name + ")", "Differently Assigned Cells", "Fraction of Clusters With More Differences",
                                                           [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        std::vector<int> count[4];

        for (int i = 0; i < 4; ++i)
          {
            count[i].resize(ev.ref_clusters.size(), 0);
          }

        for (int i = 0; i < NCaloCells; ++i)
          {
            const int ref_idx = ev.ref_tag_array[i];
            const int test_idx = ev.test_tag_array[i];
            const float this_SNR = ev.SNR_array[i];
            if (ref_idx >= 0 && ev.r2t_matches[ref_idx] != test_idx)
              {
                operate_on_types(count, this_SNR,
                                 [&](std::vector<int> * vec)
                {
                  (*vec)[ref_idx] += 1;
                });
              }
          }
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            group->global->Fill(count[0][i]);
            group->t1->Fill(count[1][i]);
            group->t2->Fill(count[2][i]);
            group->t3->Fill(count[3][i]);
          }
      }

  },
  StyleKinds::ref),
  ref_name,
  add_plot<H1D_plotter_type>("diff_cell_per_cluster_test_cumul",
                             RETFUNC(0),
                             RETFUNC(10000),
                             false, -1, std::string("Differently Assigned Cells Per Cluster") + " (" + test_name + ")", "Differently Assigned Cells", "Fraction of Clusters With More Differences",
                             [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        std::vector<int> count[4];

        for (int i = 0; i < 4; ++i)
          {
            count[i].resize(ev.test_clusters.size(), 0);
          }

        for (int i = 0; i < NCaloCells; ++i)
          {
            const int ref_idx = ev.ref_tag_array[i];
            const int test_idx = ev.test_tag_array[i];
            const float this_SNR = ev.SNR_array[i];
            if (test_idx >= 0 && ev.t2r_matches[test_idx] != ref_idx)
              {
                operate_on_types(count, this_SNR,
                                 [&](std::vector<int> * vec)
                {
                  (*vec)[test_idx] += 1;
                });
              }
          }
        for (size_t i = 0; i < ev.test_clusters.size(); ++i)
          {
            group->global->Fill(count[0][i]);
            group->t1->Fill(count[1][i]);
            group->t2->Fill(count[2][i]);
            group->t3->Fill(count[3][i]);
          }
      }

  },
  StyleKinds::test),
  test_name);



  add_plot<joined_plotter_type>("rel_diff_cell_per_cluster_cumul", "Fraction of Differently Assigned Cells Per Cluster", "Differently Assigned Cells/Cluster Size", "Fraction of Clusters With Greater Relative Differences",
                                add_plot<H1D_plotter_type>("rel_diff_cell_per_cluster_ref_cumul",
                                                           RETFUNC(0),
                                                           RETFUNC(1),
                                                           false, -1, std::string("Fraction of Differently Assigned Cells Per Cluster") + " (" + ref_name + ")", "Differently Assigned Cells/Cluster Size", "Fraction of Clusters With Greater Relative Differences",
                                                           [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        std::vector<int> count[4];

        for (int i = 0; i < 4; ++i)
          {
            count[i].resize(ev.ref_clusters.size(), 0);
          }

        for (int i = 0; i < NCaloCells; ++i)
          {
            const int ref_idx = ev.ref_tag_array[i];
            const int test_idx = ev.test_tag_array[i];
            const float this_SNR = ev.SNR_array[i];
            if (ref_idx >= 0 && ev.r2t_matches[ref_idx] != test_idx)
              {
                operate_on_types(count, this_SNR,
                                 [&](std::vector<int> * vec)
                {
                  (*vec)[ref_idx] += 1;
                });
              }
          }
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            if (ev.ref_clusters[i].num_cells > min_size_cut)
              {
                group->global->Fill(double(count[0][i]) / ev.ref_clusters[i].num_cells);
                group->t1->Fill(double(count[1][i]) / ev.ref_clusters[i].num_cells);
                group->t2->Fill(double(count[2][i]) / ev.ref_clusters[i].num_cells);
                group->t3->Fill(double(count[3][i]) / ev.ref_clusters[i].num_cells);
              }
          }
      }

  },
  StyleKinds::ref),
  ref_name,
  add_plot<H1D_plotter_type>("rel_diff_cell_per_cluster_test_cumul",
                             RETFUNC(0),
                             RETFUNC(1),
                             false, -1, std::string("Fraction of Differently Assigned Cells Per Cluster") + " (" + test_name + ")", "Differently Assigned Cells/Cluster Size", "Fraction of Clusters With Greater Relative Differences",
                             [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        std::vector<int> count[4];

        for (int i = 0; i < 4; ++i)
          {
            count[i].resize(ev.test_clusters.size(), 0);
          }

        for (int i = 0; i < NCaloCells; ++i)
          {
            const int ref_idx = ev.ref_tag_array[i];
            const int test_idx = ev.test_tag_array[i];
            const float this_SNR = ev.SNR_array[i];
            if (test_idx >= 0 && ev.t2r_matches[test_idx] != ref_idx)
              {
                operate_on_types(count, this_SNR,
                                 [&](std::vector<int> * vec)
                {
                  (*vec)[test_idx] += 1;
                });
              }
          }
        for (size_t i = 0; i < ev.test_clusters.size(); ++i)
          {
            if (ev.test_clusters[i].num_cells > min_size_cut)
              {
                group->global->Fill(double(count[0][i]) / ev.test_clusters[i].num_cells);
                group->t1->Fill(double(count[1][i]) / ev.test_clusters[i].num_cells);
                group->t2->Fill(double(count[2][i]) / ev.test_clusters[i].num_cells);
                group->t3->Fill(double(count[3][i]) / ev.test_clusters[i].num_cells);
              }
          }
      }

  },
  StyleKinds::test),
  test_name);



  add_plot<H2D_plotter_type>(
          "diff_cell_vs_delta_phi",
          RETFUNC(0),
          RETFUNC(10000),
          RETFUNC(min_delta_phi[i]),
          RETFUNC(max_delta_phi[i]),
          true, "Number of Differently Assigned Cells versus #Delta #phi", "Differently Assigned Cells", "#Delta #phi",
          [&](hist_group_2D * group)
  {

    for (const auto & ev : events)
      {
        std::vector<int> count[4];

        for (int i = 0; i < 4; ++i)
          {
            count[i].resize(ev.test_clusters.size(), 0);
          }

        for (int i = 0; i < NCaloCells; ++i)
          {
            const int ref_idx = ev.ref_tag_array[i];
            const int test_idx = ev.test_tag_array[i];
            const float this_SNR = ev.SNR_array[i];
            if (test_idx >= 0 && ev.t2r_matches[test_idx] != ref_idx)
              {
                operate_on_types(count, this_SNR,
                                 [&](std::vector<int> * vec)
                {
                  (*vec)[test_idx] += 1;
                });
              }
            else if (ref_idx >= 0 && ev.r2t_matches[ref_idx] >= 0 && test_idx != ev.r2t_matches[ref_idx])
              {
                operate_on_types(count, this_SNR,
                                 [&](std::vector<int> * vec)
                {
                  (*vec)[ev.r2t_matches[ref_idx]] += 1;
                });
              }
          }
        for (size_t i = 0; i < ev.test_clusters.size(); ++i)
          {
            const int this_match = ev.t2r_matches[i];
            if (this_match >= 0)
              {
                const double dp = minDiffPhi(ev.test_clusters[i].phi, ev.ref_clusters[this_match].phi);
                group->global->Fill(count[0][i], dp);
                group->t1->Fill(count[1][i], dp);
                group->t2->Fill(count[2][i], dp);
                group->t3->Fill(count[3][i], dp);
              }
          }
      }
  },
  StyleKinds::joined);

  /*
  add_plot<H1D_plotter_type>(
           "diff_cell_per_cluster_alt",
           RETFUNC(25000),
           RETFUNC(-25000),
           true, 0, "Clusters Cell Differences", test_name + " Cells - " + ref_name + " Cells", "Counts",
           [&](hist_group_1D *group)
           {
             for (const auto & ev : events)
         {

           std::vector<int> num_cells[2][4];
           for (int j = 0; j < 4; ++j)
             {
               num_cells[0][j].resize(ev.ref_clusters.size());
             }
           for (int j = 0; j < 4; ++j)
             {
               num_cells[1][j].resize(ev.test_clusters.size());
             }
           for (int i = 0; i < NCaloCells; ++i)
             {
               const int ref_idx = ev.ref_tag_array[i];
               const int test_idx = ev.test_tag_array[i];
               const float this_SNR = ev.SNR_array[i];
               if (ref_idx >= 0)
           {
             operate_on_types(num_cells[0], this_SNR, [&](std::vector<int> * vec){ (*vec)[ref_idx] += 1; });
           }
               if (test_idx >= 0)
           {
             operate_on_types(num_cells[1], this_SNR, [&](std::vector<int> * vec){ (*vec)[test_idx] += 1; });
           }
             }
           for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
             {
               const int this_match = ev.r2t_matches[i];
               if (this_match >= 0)
           {
             const int this_cluster = i;
             const int match_cluster = this_match;
             group->global->Fill(num_cells[1][0][match_cluster] - num_cells[0][0][this_cluster]);
             group->t1->Fill(num_cells[1][1][match_cluster] - num_cells[0][1][this_cluster]);
             group->t2->Fill(num_cells[1][2][match_cluster] - num_cells[0][2][this_cluster]);
             group->t3->Fill(num_cells[1][3][match_cluster] - num_cells[0][3][this_cluster]);

           }
             }
         }

           },
           StyleKinds::joined);
  */
  add_plot<H1D_plotter_region>(
          "delta_eta_hist",
          RETFUNC(min_delta_eta[i]),
          RETFUNC(max_delta_eta[i]),
          true, 0, "Matched Clusters #Delta #eta", "#eta_{Cluster}^{(" + test_name + ")} - #eta_{Cluster}^{(" + ref_name + ")}", (normalize ? "Average Number of Clusters per Event" : "Clusters"),
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.r2t_matches.size(); ++i)
          {
            const int this_match = ev.r2t_matches[i];
            if (this_match >= 0)
              {
                const ClusterData * this_cluster = &(ev.ref_clusters[i]);
                const ClusterData * match_cluster = &(ev.test_clusters[this_match]);
                const double delta_eta = (match_cluster->eta - this_cluster->eta);
                fill_regions(group, match_cluster->eta, delta_eta);
              }
          }
      }

  },
  StyleKinds::joined);

  add_plot<H1D_plotter_region>(
          "delta_phi_hist",
          RETFUNC(min_delta_phi[i]),
          RETFUNC(max_delta_phi[i]),
          true, 0, "Matched Clusters #Delta #phi", "#Delta #phi", (normalize ? "Average Number of Clusters per Event" : "Clusters"),
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.r2t_matches.size(); ++i)
          {
            const int this_match = ev.r2t_matches[i];
            if (this_match >= 0)
              {
                const ClusterData * this_cluster = &(ev.ref_clusters[i]);
                const ClusterData * match_cluster = &(ev.test_clusters[this_match]);
                const double delta_phi = minDiffPhi(match_cluster->phi, this_cluster->phi);
                fill_regions(group, match_cluster->eta, delta_phi);
              }
          }
      }

  },
  StyleKinds::joined);


  add_plot<H1D_plotter_region>(
          "delta_phi_vs_cpu_size_hist",
          RETFUNC(min_delta_phi[i]),
          RETFUNC(max_delta_phi[i]),
          true, 0, "Matched Clusters #Delta #phi/CPU Size", "#Delta #phi/CPU Cluster Size", (normalize ? "Average Number of Clusters per Event" : "Clusters"),
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.r2t_matches.size(); ++i)
          {
            const int this_match = ev.r2t_matches[i];
            if (this_match >= 0)
              {
                const ClusterData * this_cluster = &(ev.ref_clusters[i]);
                const ClusterData * match_cluster = &(ev.test_clusters[this_match]);
                const double delta_phi = minDiffPhi(match_cluster->phi, this_cluster->phi) / this_cluster->num_cells;
                fill_regions(group, match_cluster->eta, delta_phi);
              }
          }
      }

  },
  StyleKinds::joined);


  add_plot<H2D_plotter_region>(
          "delta_phi_vs_cpu_size_hist2d",
          RETFUNC(min_delta_phi[i]),
          RETFUNC(max_delta_phi[i]),
          RETFUNC(0),
          RETFUNC(max_vals[i].num_cells),
          true, "Matched Clusters #Delta #phi versus CPU Size", "#Delta #phi", "CPU Cluster Size",
          [&](hist_group_2D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.r2t_matches.size(); ++i)
          {
            const int this_match = ev.r2t_matches[i];
            if (this_match >= 0)
              {
                const ClusterData * this_cluster = &(ev.ref_clusters[i]);
                const ClusterData * match_cluster = &(ev.test_clusters[this_match]);
                const double delta_phi = minDiffPhi(match_cluster->phi, this_cluster->phi);
                fill_regions(group, match_cluster->eta, delta_phi, this_cluster->num_cells);
              }
          }
      }

  },
  StyleKinds::joined);


  add_plot<H1D_plotter_region>(
          "delta_phi_vs_gpu_size_hist",
          RETFUNC(min_delta_phi[i]),
          RETFUNC(max_delta_phi[i]),
          true, 0, "Matched Clusters #Delta #phi/GPU Size", "#Delta #phi/GPU Cluster Size", (normalize ? "Average Number of Clusters per Event" : "Clusters"),
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.r2t_matches.size(); ++i)
          {
            const int this_match = ev.r2t_matches[i];
            if (this_match >= 0)
              {
                const ClusterData * this_cluster = &(ev.ref_clusters[i]);
                const ClusterData * match_cluster = &(ev.test_clusters[this_match]);
                const double delta_phi = minDiffPhi(match_cluster->phi, this_cluster->phi) / match_cluster->num_cells;
                fill_regions(group, match_cluster->eta, delta_phi);
              }
          }
      }

  },
  StyleKinds::joined);



  add_plot<H2D_plotter_region>(
          "delta_phi_vs_gpu_size_hist2d",
          RETFUNC(min_delta_phi[i]),
          RETFUNC(max_delta_phi[i]),
          RETFUNC(0),
          RETFUNC(max_vals[i].num_cells),
          true, "Matched Clusters #Delta #phi versus GPU Size", "#Delta #phi", "GPU Cluster Size",
          [&](hist_group_2D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.r2t_matches.size(); ++i)
          {
            const int this_match = ev.r2t_matches[i];
            if (this_match >= 0)
              {
                const ClusterData * this_cluster = &(ev.ref_clusters[i]);
                const ClusterData * match_cluster = &(ev.test_clusters[this_match]);
                const double delta_phi = minDiffPhi(match_cluster->phi, this_cluster->phi);
                fill_regions(group, match_cluster->eta, delta_phi, match_cluster->num_cells);
              }
          }
      }

  },
  StyleKinds::joined);

  /*************************************
   *           DELTA R PLOTS           *
   *************************************/
  add_plot<H1D_plotter_region>(
          "delta_R_hist",
          RETFUNC(min_delta_R[i]),
          RETFUNC(max_delta_R[i]),
          false, 0, "Matched Clusters Distance", "#Delta R", (normalize ? "Average Number of Clusters per Event" : "Clusters"),
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const double dr = ev.delta_R_array[i];
            if (dr >= 0)
              {
                fill_regions(group, ev.ref_clusters[i].eta, dr);
              }
          }
      }

  },
  StyleKinds::joined);

  add_plot<H1D_plotter_region>(
          "delta_R_hist_cumul",
          RETFUNC(min_delta_R[i]),
          RETFUNC(max_delta_R[i]),
          false, -1, "Fraction of Matched Clusters at Distance", "#Delta R", "Fraction of More Distant Clusters",
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const double dr = ev.delta_R_array[i];
            if (dr >= 0)
              {
                fill_regions(group, ev.ref_clusters[i].eta, dr);
              }
          }
      }

  },
  StyleKinds::joined);


  add_plot<H2D_plotter_region>(
          "delta_R_vs_E",
          RETFUNC(min_vals[i].energy),
          RETFUNC(max_vals[i].energy),
          RETFUNC(min_delta_R[i]),
          RETFUNC(max_delta_R[i]),
          true, "Matched Clusters Distance versus Energy", "E^{(" + ref_name + ")} [GeV]", "#Delta R",
          [&](hist_group_2D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const double dr = ev.delta_R_array[i];
            if (dr >= 0)
              {
                fill_regions(group, ev.ref_clusters[i].eta, ev.ref_clusters[i].energy, dr);
              }
          }
      }

  },
  StyleKinds::joined);

  add_plot<H2D_plotter_region>(
          "delta_R_vs_Et",
          RETFUNC(min_vals[i].transverse_energy),
          RETFUNC(max_vals[i].transverse_energy),
          RETFUNC(min_delta_R[i]),
          RETFUNC(max_delta_R[i]),
          true, "Matched Clusters Distance versus Transverse Energy", "E_{T}^{(" + ref_name + ")} [GeV]", "#Delta R",
          [&](hist_group_2D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const double dr = ev.delta_R_array[i];
            if (dr >= 0)
              {
                fill_regions(group, ev.ref_clusters[i].eta, ev.ref_clusters[i].transverse_energy, dr);
              }
          }
      }

  },
  StyleKinds::joined);

  add_plot<H2D_plotter_region>(
          "delta_R_vs_Eta",
          RETFUNC(min_vals[i].eta),
          RETFUNC(max_vals[i].eta),
          RETFUNC(min_delta_R[i]),
          RETFUNC(max_delta_R[i]),
          true, "Matched Clusters Distance versus #eta", "#eta^{(" + ref_name + ")}", "#Delta R",
          [&](hist_group_2D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const double dr = ev.delta_R_array[i];
            if (dr >= 0)
              {
                fill_regions(group, ev.ref_clusters[i].eta, ev.ref_clusters[i].eta, dr);
              }
          }
      }

  },
  StyleKinds::joined);

  add_plot<H2D_plotter_region>(
          "delta_R_vs_Phi",
          RETFUNC(min_vals[i].phi),
          RETFUNC(max_vals[i].phi),
          RETFUNC(min_delta_R[i]),
          RETFUNC(max_delta_R[i]),
          true, "Matched Clusters Distance versus #phi", "#phi^{(" + ref_name + ")}", "#Delta R",
          [&](hist_group_2D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const double dr = ev.delta_R_array[i];
            if (dr >= 0)
              {
                fill_regions(group, ev.ref_clusters[i].eta, ev.ref_clusters[i].phi, dr);
              }
          }
      }

  },
  StyleKinds::joined);

  add_plot<H2D_plotter_region>(
          "delta_R_vs_size",
          RETFUNC(min_vals[i].num_cells),
          RETFUNC(max_vals[i].num_cells),
          RETFUNC(min_delta_R[i]),
          RETFUNC(max_delta_R[i]),
          false, "Matched Clusters Distance versus Cluster Size", "Cluster Size^{(" + ref_name + ")} [# Cells]", "#Delta R",
          [&](hist_group_2D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const double dr = ev.delta_R_array[i];
            if (dr >= 0)
              {
                fill_regions(group, ev.ref_clusters[i].eta, ev.ref_clusters[i].num_cells, dr);
              }
          }
      }

  },
  StyleKinds::joined);

  add_plot<H2D_plotter_type>(
          "delta_cells_vs_delta_R",
          RETFUNC(min_delta_R[i]),
          RETFUNC(max_delta_R[i]),
          RETFUNC(-2500),
          RETFUNC(2500),
          true, "Matched Clusters Distance versus Difference in Number of Cells", "#Delta R", "#Delta Cells",
          [&](hist_group_2D * group)
  {
    for (const auto & ev : events)
      {
        std::vector<int> num_cells[2][4];
        for (int j = 0; j < 4; ++j)
          {
            num_cells[0][j].resize(ev.ref_clusters.size());
          }
        for (int j = 0; j < 4; ++j)
          {
            num_cells[1][j].resize(ev.test_clusters.size());
          }
        for (int i = 0; i < NCaloCells; ++i)
          {
            const int ref_idx = ev.ref_tag_array[i];
            const int test_idx = ev.test_tag_array[i];
            const float this_SNR = ev.SNR_array[i];
            if (ref_idx >= 0)
              {
                operate_on_types(num_cells[0], this_SNR, [&](std::vector<int> * vec)
                {
                  (*vec)[ref_idx] += 1;
                });
              }
            if (test_idx >= 0)
              {
                operate_on_types(num_cells[1], this_SNR, [&](std::vector<int> * vec)
                {
                  (*vec)[test_idx] += 1;
                });
              }
          }
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const int this_match = ev.r2t_matches[i];
            if (this_match >= 0)
              {
                const double dr = ev.delta_R_array[i];
                const int this_cluster = i;
                const int match_cluster = this_match;
                group->global->Fill(dr, num_cells[1][0][match_cluster] - num_cells[0][0][this_cluster]);
                group->t1->Fill(dr, num_cells[1][1][match_cluster] - num_cells[0][1][this_cluster]);
                group->t2->Fill(dr, num_cells[1][2][match_cluster] - num_cells[0][2][this_cluster]);
                group->t3->Fill(dr, num_cells[1][3][match_cluster] - num_cells[0][3][this_cluster]);

              }
          }
      }

  },
  StyleKinds::joined);

  /*************************************
   *      DELTA (ET) (REL) PLOTS       *
   *************************************/


  add_plot<H1D_plotter_region>(
          "delta_E_hist",
          RETFUNC(min_vals[i].energy),
          RETFUNC(max_vals[i].energy),
          true, 0, "Energy Difference", "#font[52]{E}^{(" + test_name + ")} - #font[52]{E}^{(" + ref_name + ")} [GeV]", (normalize ? "Average Number of Clusters per Event" : "Clusters"),
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const int match_cluster = ev.r2t_matches[i];
            if (match_cluster >= 0)
              {
                const double de = ev.test_clusters[match_cluster].energy - ev.ref_clusters[i].energy;
                fill_regions(group, ev.ref_clusters[i].eta, de);
              }
          }
      }

  },
  StyleKinds::joined);

  add_plot<H1D_plotter_region>(
          "delta_Et_hist",
          RETFUNC(min_vals[i].transverse_energy),
          RETFUNC(max_vals[i].transverse_energy),
          true, 0, "Transverse Energy Difference", "#font[52]{E}_{T}^{(" + test_name + ")} - #font[52]{E}_{T}^{(" + ref_name + ")} [GeV]", (normalize ? "Average Number of Clusters per Event" : "Clusters"),
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const int match_cluster = ev.r2t_matches[i];
            if (match_cluster >= 0)
              {
                const double de = ev.test_clusters[match_cluster].transverse_energy - ev.ref_clusters[i].transverse_energy;
                fill_regions(group, ev.ref_clusters[i].eta, de);
              }
          }
      }

  },
  StyleKinds::joined);

  add_plot<H1D_plotter_region>(
          "delta_E_rel_hist",
          RETFUNC(min_delta_E_rel[i]),
          RETFUNC(max_delta_E_rel[i]),
          true, 0, "Relative Energy Difference", "#(){E^{(" + test_name + ")} - E^{(" + ref_name + ")}}/E^{(" + ref_name + ")}", "Counts",
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const int match_cluster = ev.r2t_matches[i];
            if (match_cluster >= 0)
              {
                const double de = (ev.test_clusters[match_cluster].energy - ev.ref_clusters[i].energy);
                const double refe = ev.ref_clusters[i].energy;
                if (std::abs(refe) < min_energy_cut)
                  {
                    continue;
                  }
                fill_regions(group, ev.ref_clusters[i].eta, de / refe);
              }
          }
      }

  },
  StyleKinds::joined);

  add_plot<H1D_plotter_region>(
          "delta_Et_rel_hist",
          RETFUNC(min_delta_Et_rel[i]),
          RETFUNC(max_delta_Et_rel[i]),
          true, 0, "Relative Transverse Energy Difference", "#(){E_{T}^{(" + test_name + ")} - E_{T}^{(" + ref_name + ")}}/E_{T}^{(" + ref_name + ")}", "Counts",
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const double det = ev.delta_Et_rel_array[i];
            if (std::abs(ev.ref_clusters[i].transverse_energy) < min_energy_cut)
              {
                continue;
              }
            if (det >= -1)
              {
                fill_regions(group, ev.ref_clusters[i].eta, det);
              }
          }
      }

  },
  StyleKinds::joined);

  add_plot<H2D_plotter_region>(
          "delta_Et_rel_vs_Et",
          RETFUNC(min_vals[i].transverse_energy),
          RETFUNC(max_vals[i].transverse_energy),
          RETFUNC(min_delta_Et_rel[i]),
          RETFUNC(max_delta_Et_rel[i]),
          true, "Relative Transverse Energy Difference versus Transverse Energy", "E_{T}^{(" + ref_name + ")} [GeV]", "#(){E_{T}^{(" + test_name + ")} - E_{T}^{(" + ref_name + ")}}/E_{T}^{(" + ref_name + ")}",
          [&](hist_group_2D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const double det = ev.delta_Et_rel_array[i];
            if (std::abs(ev.ref_clusters[i].transverse_energy) < min_energy_cut)
              {
                continue;
              }
            if (det >= -1)
              {
                fill_regions(group, ev.ref_clusters[i].eta, ev.ref_clusters[i].transverse_energy, det);
              }
          }
      }

  },
  StyleKinds::joined);


  add_plot<H2D_plotter_region>(
          "Et1_vs_Et2",
          RETFUNC(min_vals[i].transverse_energy),
          RETFUNC(max_vals[i].transverse_energy),
          RETFUNC(min_vals[i].transverse_energy),
          RETFUNC(max_vals[i].transverse_energy),
          true, "Transverse Energy Comparison", "E_{T}^{(" + ref_name + ")} [GeV]", "E_{T}^{(" + test_name + ")} [GeV]",
          [&](hist_group_2D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.r2t_matches.size(); ++i)
          {
            const int this_match = ev.r2t_matches[i];
            if (this_match >= 0)
              {
                fill_regions(group, ev.ref_clusters[i].eta, ev.ref_clusters[i].transverse_energy, ev.test_clusters[this_match].transverse_energy);
              }
          }
      }

  },
  StyleKinds::joined);


  add_plot<H2D_plotter_region>(
          "E1_vs_E2",
          RETFUNC(min_vals[i].energy),
          RETFUNC(max_vals[i].energy),
          RETFUNC(min_vals[i].energy),
          RETFUNC(max_vals[i].energy),
          true, "Energy Comparison", "E^{(" + ref_name + ")} [GeV]", "E^{(" + test_name + ")} [GeV]",
          [&](hist_group_2D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.r2t_matches.size(); ++i)
          {
            const int this_match = ev.r2t_matches[i];
            if (this_match >= 0)
              {
                fill_regions(group, ev.ref_clusters[i].eta, ev.ref_clusters[i].energy, ev.test_clusters[this_match].energy);
              }
          }
      }

  },
  StyleKinds::joined);


  /*************************************
   *        SEPARATE HISTOGRAMS        *
   *************************************/
  add_plot<joined_plotter_region>(
          "cluster_eta_hist", "Cluster #eta", "#eta", "Counts",
          add_plot<H1D_plotter_region>(
                  "cluster_eta_hist_ref",
                  RETFUNC(min_vals[i].eta),
                  RETFUNC(max_vals[i].eta),
                  true, 0, std::string("Cluster #eta") + " (" + ref_name + ")", "#eta", "Counts",
                  [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (const auto & refc : ev.ref_clusters)
          {
            fill_regions(group, refc.eta, refc.eta);
          }
      }
  },
  StyleKinds::ref),
  ref_name,
  add_plot<H1D_plotter_region>(
          "cluster_eta_hist_test",
          RETFUNC(min_vals[i].eta),
          RETFUNC(max_vals[i].eta),
          true, 0, std::string("Cluster #eta") + " (" + ref_name + ")", "#eta", "Counts",
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (const auto & testc : ev.test_clusters)
          {
            fill_regions(group, testc.eta, testc.eta);
          }
      }

  },
  StyleKinds::test),
  test_name);


  add_plot<joined_plotter_region>(
          "cluster_phi_hist", "Cluster #phi", "#phi", "Counts",
          add_plot<H1D_plotter_region>(
                  "cluster_phi_hist_ref",
                  RETFUNC(min_vals[i].phi),
                  RETFUNC(max_vals[i].phi),
                  true, 0, std::string("Cluster #phi") + " (" + ref_name + ")", "#phi", "Counts",
                  [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (const auto & refc : ev.ref_clusters)
          {
            fill_regions(group, refc.eta, refc.phi);
          }
      }
  },
  StyleKinds::ref),
  ref_name,
  add_plot<H1D_plotter_region>(
          "cluster_phi_hist_test",
          RETFUNC(min_vals[i].phi),
          RETFUNC(max_vals[i].phi),
          true, 0, std::string("Cluster #phi") + " (" + ref_name + ")", "#phi", "Counts",
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (const auto & testc : ev.test_clusters)
          {
            fill_regions(group, testc.eta, testc.phi);
          }
      }

  },
  StyleKinds::test),
  test_name);

  add_plot<joined_plotter_region>(
          "Et_hist", "Cluster Transverse Energy", "#font[52]{E}_{T} [GeV]", (normalize ? "Average Number of Clusters per Event" : "Clusters"),
          add_plot<H1D_plotter_region>(
                  "Et_hist_ref",
                  RETFUNC(min_vals[i].transverse_energy),
                  RETFUNC(max_vals[i].transverse_energy),
                  true, 0, std::string("Cluster Transverse Energy") + " (" + ref_name + ")", "#font[52]{E}_{T} [GeV]", (normalize ? "Average Number of Clusters per Event" : "Clusters"),
                  [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (const auto & refc : ev.ref_clusters)
          {
            fill_regions(group, refc.eta, refc.transverse_energy);
          }
      }
  },
  StyleKinds::ref),
  ref_name,
  add_plot<H1D_plotter_region>(
          "Et_hist_test",
          RETFUNC(min_vals[i].transverse_energy),
          RETFUNC(max_vals[i].transverse_energy),
          true, 0, std::string("Cluster Transverse Energy") + " (" + test_name + ")", "E_{T} [GeV]", (normalize ? "Average Number of Clusters per Event" : "Clusters"),
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (const auto & testc : ev.test_clusters)
          {
            fill_regions(group, testc.eta, testc.transverse_energy);
          }
      }

  },
  StyleKinds::test),
  test_name);


  add_plot<joined_plotter_region>(
          "E_hist", "Cluster Energy", "#font[52]{E} [GeV]", (normalize ? "Average Number of Clusters per Event" : "Clusters"),
          add_plot<H1D_plotter_region>(
                  "E_hist_ref",
                  RETFUNC(min_vals[i].energy),
                  RETFUNC(max_vals[i].energy),
                  true, 0, std::string("Cluster Energy") + " (" + ref_name + ")", "#font[52]{E} [GeV]", (normalize ? "Average Number of Clusters per Event" : "Clusters"),
                  [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (const auto & refc : ev.ref_clusters)
          {
            fill_regions(group, refc.eta, refc.energy);
          }
      }

  },
  StyleKinds::ref),
  ref_name,
  add_plot<H1D_plotter_region>(
          "E_hist_test",
          RETFUNC(min_vals[i].energy),
          RETFUNC(max_vals[i].energy),
          true, 0, std::string("Cluster Energy") + " (" + test_name + ")", "#font[52]{E} [GeV]", (normalize ? "Average Number of Clusters per Event" : "Clusters"),
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (const auto & testc : ev.test_clusters)
          {
            fill_regions(group, testc.eta, testc.energy);
          }
      }

  },
  StyleKinds::test),
  test_name);

  add_plot<joined_plotter_region>(
          "cluster_size_hist", "Cluster Size", "Cluster Size [# Cells]", "Counts",
          add_plot<H1D_plotter_region>(
                  "cluster_size_hist_ref",
                  RETFUNC(min_vals[i].num_cells),
                  RETFUNC(max_vals[i].num_cells),
                  false, 0, std::string("Cluster Size") + " (" + ref_name + ")", "Cluster Size [# Cells]", "Counts",
                  [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (const auto & refc : ev.ref_clusters)
          {
            fill_regions(group, refc.eta, refc.num_cells);
          }
      }

  },
  StyleKinds::ref),
  ref_name,
  add_plot<H1D_plotter_region>(
          "cluster_size_hist_test",
          RETFUNC(min_vals[i].num_cells),
          RETFUNC(max_vals[i].num_cells),
          false, 0, std::string("Cluster Size") + " (" + test_name + ")", "Cluster Size [# Cells]", "Counts",
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (const auto & testc : ev.test_clusters)
          {
            fill_regions(group, testc.eta, testc.num_cells);
          }
      }

  },
  StyleKinds::test),
  test_name);
  add_plot<joined_plotter_region>(
          "cluster_size_hist_cumul", "Cluster Size", "Cluster Size [# Cells]", "Fraction of Larger Clusters",
          add_plot<H1D_plotter_region>(
                  "cluster_size_hist_ref_cumul",
                  RETFUNC(min_vals[i].num_cells),
                  RETFUNC(max_vals[i].num_cells),
                  false, -1, std::string("Cluster Size") + " (" + ref_name + ")", "Cluster Size [# Cells]", "Fraction of Larger Clusters",
                  [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (const auto & refc : ev.ref_clusters)
          {
            fill_regions(group, refc.eta, refc.num_cells);
          }
      }

  },
  StyleKinds::ref),
  ref_name,
  add_plot<H1D_plotter_region>(
          "cluster_size_hist_test_cumul",
          RETFUNC(min_vals[i].num_cells),
          RETFUNC(max_vals[i].num_cells),
          false, -1, std::string("Cluster Size") + " (" + test_name + ")", "Cluster Size [# Cells]", "Fraction of Larger Clusters",
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (const auto & testc : ev.test_clusters)
          {
            fill_regions(group, testc.eta, testc.num_cells);
          }
      }

  },
  StyleKinds::test),
  test_name);

  add_plot<joined_plotter_region>(
          "cluster_number_hist", "Number of Clusters Per Event", "Number of Clusters Per Event", (normalize ? "Fraction of Events" : "Events"),
          add_plot<H1D_plotter_region>(
                  "cluster_number_hist_ref",
                  RETFUNC(min_cluster_number[i]),
                  RETFUNC(max_cluster_number[i]),
                  false, 0, std::string("Number of Clusters Per Event") + " (" + ref_name + ")", "Number of Clusters Per Event", (normalize ? "Fraction of Events" : "Events"),
                  [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {

        int ref_counter[4] = {0, 0, 0, 0};

        for (const auto & refc : ev.ref_clusters)
          {
            operate_on_regions(ref_counter, refc.eta, [](int * val)
            {
              *val = *val + 1;
            } );
          }

        group->global->Fill(ref_counter[0]);
        group->t1->Fill(ref_counter[1]);
        group->t2->Fill(ref_counter[2]);
        group->t3->Fill(ref_counter[3]);

      }

  },
  StyleKinds::ref),
  ref_name,
  add_plot<H1D_plotter_region>(
          "cluster_number_hist_test",
          RETFUNC(min_cluster_number[i]),
          RETFUNC(max_cluster_number[i]),
          false, 0, std::string("Number of Clusters Per Event") + " (" + test_name + ")", "Number of Clusters Per Event", (normalize ? "Fraction of Events" : "Events"),
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {

        int test_counter[4] = {0, 0, 0, 0};
        for (const auto & testc : ev.test_clusters)
          {
            operate_on_regions(test_counter, testc.eta, [](int * val)
            {
              *val = *val + 1;
            } );
          }
        group->global->Fill(test_counter[0]);
        group->t1->Fill(test_counter[1]);
        group->t2->Fill(test_counter[2]);
        group->t3->Fill(test_counter[3]);
      }

  },
  StyleKinds::test),
  test_name);

  add_plot<joined_plotter_region>(
          "term_cell_E_perc_hist", "Percentage of Cluster Energy From Terminal Cells", "%", "Counts",
          add_plot<H1D_plotter_region>(
                  "term_cell_E_perc_hist_ref",
                  RETFUNC(0),
                  RETFUNC(100.),
                  false, 0, std::string("Percentage of Cluster Energy From Terminal Cells") + " (" + ref_name + ")", "%", "Counts",
                  [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        std::vector<double> ref_termE(ev.ref_clusters.size(), 0.);
        for (int i = 0; i < NCaloCells; ++i)
          {
            const double this_energy = ev.energy_array[i];
            const double this_absenergy = std::abs(this_energy);
            const float this_SNR = ev.SNR_array[i];
            const int ref_idx = ev.ref_tag_array[i];
            if (this_SNR > SNR_thresholds[0] && this_SNR <= SNR_thresholds[1])
              {
                if (ref_idx >= 0)
                  {
                    ref_termE[ref_idx] += this_absenergy;
                  }
              }
          }
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const double res = 100. * ref_termE[i] / ev.ref_clusters[i].abs_energy;
            fill_regions( group, ev.ref_clusters[i].eta, res );
          }
      }

  },
  StyleKinds::ref),
  ref_name,
  add_plot<H1D_plotter_region>(
          "term_cell_E_perc_hist_test",
          RETFUNC(0),
          RETFUNC(100.),
          false, 0, std::string("Percentage of Cluster Energy From Terminal Cells") + " (" + test_name + ")", "%", "Counts",
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        std::vector<double> test_termE(ev.test_clusters.size(), 0.);
        for (int i = 0; i < NCaloCells; ++i)
          {
            const double this_energy = ev.energy_array[i];
            const double this_absenergy = std::abs(this_energy);
            const float this_SNR = ev.SNR_array[i];
            const int test_idx = ev.test_tag_array[i];
            if (this_SNR > SNR_thresholds[0] && this_SNR <= SNR_thresholds[1])
              {
                if (test_idx >= 0)
                  {
                    test_termE[test_idx] += this_absenergy;
                  }
              }
          }
        for (size_t i = 0; i < ev.test_clusters.size(); ++i)
          {
            const double res = 100. * test_termE[i] / ev.test_clusters[i].abs_energy;
            fill_regions( group, ev.test_clusters[i].eta, res );
          }
      }

  },
  StyleKinds::test),
  test_name);

  add_plot<H2D_plotter_region>(
          "term_cell_E_perc_vs_E_ref",
          RETFUNC(min_vals[i].abs_energy),
          RETFUNC(max_vals[i].abs_energy),
          RETFUNC(0.),
          RETFUNC(100.),
          false, "Percentage of Cluster Energy From Terminal Cells versus Cluster Energy (" + ref_name + ")", "#||{E} [GeV]", "%",
          [&](hist_group_2D * group)
  {
    for (const auto & ev : events)
      {
        std::vector<double> ref_termE(ev.ref_clusters.size(), 0.);
        for (int i = 0; i < NCaloCells; ++i)
          {
            const double this_energy = ev.energy_array[i];
            const double this_absenergy = std::abs(this_energy);
            const float this_SNR = ev.SNR_array[i];
            const int ref_idx = ev.ref_tag_array[i];
            if (this_SNR > SNR_thresholds[0] && this_SNR <= SNR_thresholds[1])
              {
                if (ref_idx >= 0)
                  {
                    ref_termE[ref_idx] += this_absenergy;
                  }
              }
          }
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const double res = 100. * ref_termE[i] / ev.ref_clusters[i].abs_energy;
            fill_regions( group, ev.ref_clusters[i].eta, ev.ref_clusters[i].abs_energy, res );
          }
      }

  },
  StyleKinds::ref);


  add_plot<H2D_plotter_region>(
          "term_cell_E_perc_vs_E_test",
          RETFUNC(min_vals[i].abs_energy),
          RETFUNC(max_vals[i].abs_energy),
          RETFUNC(0.),
          RETFUNC(100.),
          false, "Percentage of Cluster Energy From Terminal Cells versus Cluster Energy (" + test_name + ")", "#||{E} [GeV]", "%",
          [&](hist_group_2D * group)
  {
    for (const auto & ev : events)
      {
        std::vector<double> test_termE(ev.test_clusters.size(), 0.);
        for (int i = 0; i < NCaloCells; ++i)
          {
            const double this_energy = ev.energy_array[i];
            const double this_absenergy = std::abs(this_energy);
            const float this_SNR = ev.SNR_array[i];
            const int test_idx = ev.test_tag_array[i];
            if (this_SNR > SNR_thresholds[0] && this_SNR <= SNR_thresholds[1])
              {
                if (test_idx >= 0)
                  {
                    test_termE[test_idx] += this_absenergy;
                  }
              }
          }
        for (size_t i = 0; i < ev.test_clusters.size(); ++i)
          {
            const double res = 100. * test_termE[i] / ev.test_clusters[i].abs_energy;
            fill_regions( group, ev.test_clusters[i].eta, ev.test_clusters[i].abs_energy, res );
          }
      }

  },
  StyleKinds::test);

  /*************************************
   *          UNMATCHED STUFF          *
   *************************************/

  add_plot<joined_plotter_region>(
          "unmatched_number_hist", "Number of Unmatched Clusters per Event", "Unmatched Clusters Per Event", (normalize ? "Fraction of Events" : "Events"),
          add_plot<H1D_plotter_region>(
                  "unmatched_number_hist_ref",
                  RETFUNC(min_unmatched_number[i]),
                  RETFUNC(max_unmatched_number[i]),
                  false, 0, std::string("Number of Unmatched Clusters per Event") + " (" + ref_name + ")", "Unmatched Clusters Per Event", (normalize ? "Fraction of Events" : "Events"),
                  [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {

        int ref_counter[4] = {0, 0, 0, 0};

        for (const auto refc : ev.ref_unmatched)
          {
            operate_on_regions(ref_counter, ev.ref_clusters[refc].eta, [](int * val)
            {
              *val = *val + 1;
            } );
          }

        group->global->Fill(ref_counter[0]);
        group->t1->Fill(ref_counter[1]);
        group->t2->Fill(ref_counter[2]);
        group->t3->Fill(ref_counter[3]);

      }
    group->global->GetXaxis()->SetNdivisions(10, 0, 0);

  },
  StyleKinds::ref),
  ref_name,
  add_plot<H1D_plotter_region>(
          "unmatched_number_hist_test",
          RETFUNC(min_unmatched_number[i]),
          RETFUNC(max_unmatched_number[i]),
          false, 0, std::string("Number of Unmatched Clusters per Event") + " (" + test_name + ")", "Unmatched Clusters Per Event", (normalize ? "Fraction of Events" : "Events"),
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {

        int test_counter[4] = {0, 0, 0, 0};
        for (const auto testc : ev.test_unmatched)
          {
            operate_on_regions(test_counter, ev.test_clusters[testc].eta, [](int * val)
            {
              *val = *val + 1;
            } );
          }
        group->global->Fill(test_counter[0]);
        group->t1->Fill(test_counter[1]);
        group->t2->Fill(test_counter[2]);
        group->t3->Fill(test_counter[3]);
      }
    group->global->GetXaxis()->SetNdivisions(10, 0, 0);

  },
  StyleKinds::test),
  test_name);


  add_plot<joined_plotter_region>(
          "unmatched_perc_hist", "Percentage of Unmatched Clusters per Event", "%", "Counts",
          add_plot<H1D_plotter_region>(
                  "unmatched_perc_hist_ref",
                  RETFUNC(0),
                  RETFUNC(150.*double(max_unmatched_number[i]) / double(min_cluster_number[i])),
                  false, 0, std::string("Percentage of Unmatched Clusters per Event") + " (" + ref_name + ")", "%", "Counts",
                  [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {

        int ref_counter[4] = {0, 0, 0, 0};
        for (const auto refc : ev.ref_unmatched)
          {
            operate_on_regions(ref_counter, ev.ref_clusters[refc].eta, [](int * val)
            {
              *val = *val + 1;
            } );
          }
        group->global->Fill(100.*double(ref_counter[0]) /
                            double(ev.test_has_more[0] ? ev.min_cluster_number[0] : ev.max_cluster_number[0] ) );
        group->t1->Fill(100.*double(ref_counter[1]) /
                        double(ev.test_has_more[1] ? ev.min_cluster_number[1] : ev.max_cluster_number[1] ) );
        group->t2->Fill(100.*double(ref_counter[2]) /
                        double(ev.test_has_more[2] ? ev.min_cluster_number[2] : ev.max_cluster_number[2] ) );
        group->t3->Fill(100.*double(ref_counter[3]) /
                        double(ev.test_has_more[3] ? ev.min_cluster_number[3] : ev.max_cluster_number[3] ) );
      }

  },
  StyleKinds::ref),
  ref_name,
  add_plot<H1D_plotter_region>(
          "unmatched_perc_hist_test",
          RETFUNC(0),
          RETFUNC(150.*double(max_unmatched_number[i]) / double(min_cluster_number[i])),
          false, 0, std::string("Percentage of Unmatched Clusters per Event") + " (" + test_name + ")", "%", "Counts",
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {

        int test_counter[4] = {0, 0, 0, 0};
        for (const auto testc : ev.test_unmatched)
          {
            operate_on_regions(test_counter, ev.test_clusters[testc].eta, [](int * val)
            {
              *val = *val + 1;
            } );
          }

        group->global->Fill(100.*double(test_counter[0]) /
                            double(ev.test_has_more[0] ? ev.max_cluster_number[0] : ev.min_cluster_number[0] ) );
        group->t1->Fill(100.*double(test_counter[1]) /
                        double(ev.test_has_more[1] ? ev.max_cluster_number[1] : ev.min_cluster_number[1] ) );
        group->t2->Fill(100.*double(test_counter[2]) /
                        double(ev.test_has_more[2] ? ev.max_cluster_number[2] : ev.min_cluster_number[2] ) );
        group->t3->Fill(100.*double(test_counter[3]) /
                        double(ev.test_has_more[3] ? ev.max_cluster_number[3] : ev.min_cluster_number[3] ) );
      }

  },
  StyleKinds::test),
  test_name);

  add_plot<joined_plotter_region>(
          "unmatched_E_hist", "Unmatched Cluster Energy", "#font[52]{E} [GeV]", "Unmatched Clusters",
          add_plot<H1D_plotter_region>(
                  "unmatched_E_hist_ref",
                  RETFUNC(unmatched_min_vals[i].energy),
                  RETFUNC(unmatched_max_vals[i].energy),
                  true, 0, std::string("Unmatched Cluster Energy") + " (" + ref_name + ")", "#font[52]{E} [GeV]", "Unmatched Clusters",
                  [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (const auto refc : ev.ref_unmatched)
          {
            fill_regions(group, ev.ref_clusters[refc].eta, ev.ref_clusters[refc].energy);
          }
      }

  },
  StyleKinds::ref),
  ref_name,
  add_plot<H1D_plotter_region>(
          "unmatched_E_hist_test",
          RETFUNC(unmatched_min_vals[i].energy),
          RETFUNC(unmatched_max_vals[i].energy),
          true, 0, std::string("Unmatched Cluster Energy") + " (" + test_name + ")", "#font[52]{E} [GeV]", "Unmatched Clusters",
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (const auto testc : ev.test_unmatched)
          {
            fill_regions(group, ev.test_clusters[testc].eta, ev.test_clusters[testc].energy);
          }
      }

  },
  StyleKinds::test),
  test_name);

  add_plot<joined_plotter_region>(
          "unmatched_Et_hist", "Unmatched Cluster Transverse Energy", "#font[52]{E}_{T} [GeV]", "Unmatched Clusters",
          add_plot<H1D_plotter_region>(
                  "unmatched_Et_hist_ref",
                  RETFUNC(unmatched_min_vals[i].transverse_energy),
                  RETFUNC(unmatched_max_vals[i].transverse_energy),
                  true, 0, std::string("Unmatched Cluster Transverse Energy") + " (" + ref_name + ")", "#font[52]{E}_{T} [GeV]", "Unmatched Clusters",
                  [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (const auto refc : ev.ref_unmatched)
          {
            fill_regions(group, ev.ref_clusters[refc].eta, ev.ref_clusters[refc].transverse_energy);
          }
      }

  },
  StyleKinds::ref),
  ref_name,
  add_plot<H1D_plotter_region>(
          "unmatched_Et_hist_test",
          RETFUNC(unmatched_min_vals[i].transverse_energy),
          RETFUNC(unmatched_max_vals[i].transverse_energy),
          true, 0, std::string("Unmatched Cluster Transverse Energy") + " (" + test_name + ")", "#font[52]{E}_{T} [GeV]", "Unmatched Clusters",
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (const auto testc : ev.test_unmatched)
          {
            fill_regions(group, ev.test_clusters[testc].eta, ev.test_clusters[testc].transverse_energy);
          }
      }

  },
  StyleKinds::test),
  test_name);


  add_plot<joined_plotter_region>(
          "unmatched_sizes_hist", "Unmatched Cluster Sizes", "Unmatched Cluster Size [Cells]", "Unmatched Clusters",
          add_plot<H1D_plotter_region>(
                  "unmatched_sizes_hist_ref",
                  RETFUNC(unmatched_min_vals[i].num_cells),
                  RETFUNC(unmatched_max_vals[i].num_cells),
                  false, 0, std::string("Unmatched Cluster Sizes") + " (" + ref_name + ")", "Unmatched Cluster Size [Cells]", "Unmatched Clusters",
                  [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (const auto refc : ev.ref_unmatched)
          {
            fill_regions(group, ev.ref_clusters[refc].eta, ev.ref_clusters[refc].num_cells);
          }
      }

  },
  StyleKinds::ref),
  ref_name,
  add_plot<H1D_plotter_region>(
          "unmatched_sizes_hist_test",
          RETFUNC(unmatched_min_vals[i].num_cells),
          RETFUNC(unmatched_max_vals[i].num_cells),
          false, 0, std::string("Unmatched Cluster Sizes") + " (" + test_name + ")", "Unmatched Cluster Size [Cells]", "Unmatched Clusters",
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (const auto testc : ev.test_unmatched)
          {
            fill_regions(group, ev.test_clusters[testc].eta, ev.test_clusters[testc].num_cells);
          }
      }

  },
  StyleKinds::test),
  test_name);
  add_plot<joined_plotter_region>(
          "unmatched_sizes_hist_cumul", "Unmatched Cluster Sizes", "Cluster Size [# Cells]", "Fraction of Larger Clusters",
          add_plot<H1D_plotter_region>(
                  "unmatched_sizes_hist_ref_cumul",
                  RETFUNC(unmatched_min_vals[i].num_cells),
                  RETFUNC(unmatched_max_vals[i].num_cells),
                  false, -1, std::string("Unmatched Cluster Sizes") + " (" + ref_name + ")", "Cluster Size [# Cells]", "Fraction of Larger Clusters",
                  [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (const auto refc : ev.ref_unmatched)
          {
            fill_regions(group, ev.ref_clusters[refc].eta, ev.ref_clusters[refc].num_cells);
          }
      }

  },
  StyleKinds::ref),
  ref_name,
  add_plot<H1D_plotter_region>(
          "unmatched_sizes_hist_test_cumul",
          RETFUNC(unmatched_min_vals[i].num_cells),
          RETFUNC(unmatched_max_vals[i].num_cells),
          false, -1, std::string("Unmatched Cluster Sizes") + " (" + test_name + ")", "Cluster Size [# Cells]", "Fraction of Larger Clusters",
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (const auto testc : ev.test_unmatched)
          {
            fill_regions(group, ev.test_clusters[testc].eta, ev.test_clusters[testc].num_cells);
          }
      }

  },
  StyleKinds::test),
  test_name);

  add_plot<joined_plotter_region>(
          "unmatched_eta_hist", "Unmatched Cluster #eta", "#eta", "Unmatched Clusters",
          add_plot<H1D_plotter_region>(
                  "unmatched_eta_hist_ref",
                  RETFUNC(unmatched_min_vals[i].eta),
                  RETFUNC(unmatched_max_vals[i].eta),
                  true, 0, std::string("Unmatched Cluster #eta") + " (" + ref_name + ")", "#eta", "Unmatched Clusters",
                  [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (const auto refc : ev.ref_unmatched)
          {
            fill_regions(group, ev.ref_clusters[refc].eta, ev.ref_clusters[refc].eta);
          }
      }

  },
  StyleKinds::ref),
  ref_name,
  add_plot<H1D_plotter_region>(
          "unmatched_eta_hist_test",
          RETFUNC(unmatched_min_vals[i].eta),
          RETFUNC(unmatched_max_vals[i].eta),
          true, 0, std::string("Unmatched Cluster #eta") + " (" + test_name + ")", "#eta", "Unmatched Clusters",
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (const auto testc : ev.test_unmatched)
          {
            fill_regions(group, ev.test_clusters[testc].eta, ev.test_clusters[testc].eta);
          }
      }

  },
  StyleKinds::test),
  test_name);

  add_plot<joined_plotter_region>(
          "unmatched_phi_hist", "Unmatched Cluster #phi", "#phi", "Unmatched Clusters",
          add_plot<H1D_plotter_region>(
                  "unmatched_phi_hist_ref",
                  RETFUNC(unmatched_min_vals[i].phi),
                  RETFUNC(unmatched_max_vals[i].phi),
                  true, 0, std::string("Unmatched Cluster #phi") + " (" + ref_name + ")", "#phi", "Unmatched Clusters",
                  [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (const auto refc : ev.ref_unmatched)
          {
            fill_regions(group, ev.ref_clusters[refc].eta, ev.ref_clusters[refc].phi);
          }
      }

  },
  StyleKinds::ref),
  ref_name,
  add_plot<H1D_plotter_region>(
          "unmatched_phi_hist_test",
          RETFUNC(unmatched_min_vals[i].phi),
          RETFUNC(unmatched_max_vals[i].phi),
          true, 0, std::string("Unmatched Cluster #phi") + " (" + test_name + ")", "#phi", "Unmatched Clusters",
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (const auto testc : ev.test_unmatched)
          {
            fill_regions(group, ev.test_clusters[testc].eta, ev.test_clusters[testc].phi);
          }
      }

  },
  StyleKinds::test),
  test_name);

  /*************************************
   *         SAME CELL CLUSTERS        *
   *************************************/
  add_plot<H1D_plotter_type>(
          "equal_cells_delta_R",
          RETFUNC(min_delta_R[i]),
          RETFUNC(max_delta_R[i]),
          false, 0, "Same Cell Clusters Distance", "#Delta R", "Counts",
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const double dr = ev.delta_R_array[i];
            if (dr >= 0)
              {
                if (ev.cluster_same_cells[0][i])
                  {
                    group->global->Fill(dr);
                    //All cells match
                  }
                if (ev.cluster_same_cells[1][i])
                  {
                    group->t1->Fill(dr);
                    //Seed cells match
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i])
                  {
                    group->t2->Fill(dr);
                    //Seed and grow cells match
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i] && ev.cluster_same_cells[3][i])
                  {
                    group->t3->Fill(dr);
                    //Seed, grow and terminal cells match
                  }
              }
          }
      }

  },
  StyleKinds::joined);


  add_plot<H1D_plotter_type>(
          "equal_cells_delta_R_cumul",
          RETFUNC(min_delta_R[i]),
          RETFUNC(max_delta_R[i]),
          false, -1, "Fraction of Same Cell Clusters at a Distance", "#Delta R", "Fraction of More Distant Clusters",
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const double dr = ev.delta_R_array[i];
            if (dr >= 0)
              {
                if (ev.cluster_same_cells[0][i])
                  {
                    group->global->Fill(dr);
                    //All cells match
                  }
                if (ev.cluster_same_cells[1][i])
                  {
                    group->t1->Fill(dr);
                    //Seed cells match
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i])
                  {
                    group->t2->Fill(dr);
                    //Seed and grow cells match
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i] && ev.cluster_same_cells[3][i])
                  {
                    group->t3->Fill(dr);
                    //Seed, grow and terminal cells match
                  }
              }
          }
      }

  },
  StyleKinds::joined);


  add_plot<H1D_plotter_type>(
          "equal_cells_delta_eta",
          RETFUNC(min_delta_eta[i]),
          RETFUNC(max_delta_eta[i]),
          true, 0, "Same Cell Clusters #Delta #eta", "#eta^{(" + test_name + ")} - #eta^{(" + ref_name + ")}", "Counts",
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const int match_cluster = ev.r2t_matches[i];
            if (match_cluster >= 0)
              {
                const double delta_eta = ev.test_clusters[match_cluster].eta - ev.ref_clusters[i].eta;
                if (ev.cluster_same_cells[0][i])
                  {
                    group->global->Fill(delta_eta);
                  }
                if (ev.cluster_same_cells[1][i])
                  {
                    group->t1->Fill(delta_eta);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i])
                  {
                    group->t2->Fill(delta_eta);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i] && ev.cluster_same_cells[3][i])
                  {
                    group->t3->Fill(delta_eta);
                  }
              }
          }
      }

  },
  StyleKinds::joined);


  add_plot<H1D_plotter_type>(
          "equal_cells_delta_phi",
          RETFUNC(min_delta_phi[i]),
          RETFUNC(max_delta_phi[i]),
          true, 0, "Same Cell Clusters #Delta #phi", "#phi^{(" + test_name + ")} - #phi^{(" + ref_name + ")}", "Counts",
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const int match_cluster = ev.r2t_matches[i];
            if (match_cluster >= 0)
              {
                const double delta_phi = minDiffPhi(ev.test_clusters[match_cluster].phi, ev.ref_clusters[i].phi);
                if (ev.cluster_same_cells[0][i])
                  {
                    group->global->Fill(delta_phi);
                  }
                if (ev.cluster_same_cells[1][i])
                  {
                    group->t1->Fill(delta_phi);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i])
                  {
                    group->t2->Fill(delta_phi);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i] && ev.cluster_same_cells[3][i])
                  {
                    group->t3->Fill(delta_phi);
                  }
              }
          }
      }

  },
  StyleKinds::joined);


  add_plot<H1D_plotter_type>(
          "equal_cells_delta_E",
          RETFUNC(min_delta_E[i]),
          RETFUNC(max_delta_E[i]),
          true, 0, "Same Cell Clusters #Delta E", "E^{(" + test_name + ")} - E^{(" + ref_name + ")} [GeV]", "Counts",
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const int match_cluster = ev.r2t_matches[i];
            if (match_cluster >= 0)
              {
                const double de = ev.test_clusters[match_cluster].energy - ev.ref_clusters[i].energy;
                if (ev.cluster_same_cells[0][i])
                  {
                    group->global->Fill(de);
                  }
                if (ev.cluster_same_cells[1][i])
                  {
                    group->t1->Fill(de);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i])
                  {
                    group->t2->Fill(de);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i] && ev.cluster_same_cells[3][i])
                  {
                    group->t3->Fill(de);
                  }
              }
          }
      }

  },
  StyleKinds::joined);


  add_plot<H1D_plotter_type>(
          "equal_cells_delta_E_rel",
          RETFUNC(min_delta_E_rel[i]),
          RETFUNC(max_delta_E_rel[i]),
          true, 0, "Same Cell Clusters #Delta E/E", "#(){E^{(" + test_name + ")} - E^{(" + ref_name + ")}}/#(){E^{(" + ref_name + ")}}", "Counts",
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const int match_cluster = ev.r2t_matches[i];
            if (match_cluster >= 0)
              {
                const double de = (ev.test_clusters[match_cluster].energy - ev.ref_clusters[i].energy) / std::abs(ev.ref_clusters[i].energy);
                if (std::abs(ev.ref_clusters[i].energy) < min_energy_cut)
                  {
                    continue;
                  }
                if (ev.cluster_same_cells[0][i])
                  {
                    group->global->Fill(de);
                  }
                if (ev.cluster_same_cells[1][i])
                  {
                    group->t1->Fill(de);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i])
                  {
                    group->t2->Fill(de);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i] && ev.cluster_same_cells[3][i])
                  {
                    group->t3->Fill(de);
                  }
              }
          }
      }

  },
  StyleKinds::joined);

  add_plot<H1D_plotter_type>(
          "equal_cells_delta_Et",
          RETFUNC(min_delta_Et[i]),
          RETFUNC(max_delta_Et[i]),
          true, 0, "Same Cell Clusters #Delta E_{T}", "E_{T}^{(" + test_name + ")} - E_{T}^{(" + ref_name + ")} [GeV]", "Counts",
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const int match_cluster = ev.r2t_matches[i];
            if (match_cluster >= 0)
              {
                const double de = ev.test_clusters[match_cluster].transverse_energy - ev.ref_clusters[i].transverse_energy;
                if (ev.cluster_same_cells[0][i])
                  {
                    group->global->Fill(de);
                  }
                if (ev.cluster_same_cells[1][i])
                  {
                    group->t1->Fill(de);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i])
                  {
                    group->t2->Fill(de);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i] && ev.cluster_same_cells[3][i])
                  {
                    group->t3->Fill(de);
                  }
              }
          }
      }

  },
  StyleKinds::joined);


  add_plot<H1D_plotter_type>(
          "equal_cells_delta_Et_rel",
          RETFUNC(min_delta_Et_rel[i]),
          RETFUNC(max_delta_Et_rel[i]),
          true, 0, "Same Cell Clusters #Delta E_{T}/E_{T}", "#(){E_T^{(" + test_name + ")} - E_T^{(" + ref_name + ")}}/#(){E_T^{(" + ref_name + ")}}", "Counts",
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const int match_cluster = ev.r2t_matches[i];
            if (match_cluster >= 0)
              {
                const double de = (ev.test_clusters[match_cluster].transverse_energy - ev.ref_clusters[i].transverse_energy) / std::abs(ev.ref_clusters[i].transverse_energy);
                if (std::abs(ev.ref_clusters[i].transverse_energy) < min_energy_cut)
                  {
                    continue;
                  }
                if (ev.cluster_same_cells[0][i])
                  {
                    group->global->Fill(de);
                  }
                if (ev.cluster_same_cells[1][i])
                  {
                    group->t1->Fill(de);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i])
                  {
                    group->t2->Fill(de);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i] && ev.cluster_same_cells[3][i])
                  {
                    group->t3->Fill(de);
                  }
              }
          }
      }

  },
  StyleKinds::joined);

  /*************************************
   *     POST-PROCESS CALCULATIONS     *
   *************************************/
  add_plot<joined_plotter_type>(
          "post_calc_delta_R", "Post-Process Calculated Clusters Distance", "#Delta R", "Counts",
          add_plot<H1D_plotter_type>(
                  "post_calc_delta_R_ref",
                  RETFUNC(min_delta_R[i]),
                  RETFUNC(max_delta_R[i]),
                  false, 0, std::string("Post-Process Calculated Clusters Distance") + " (" + ref_name + ")", "#Delta R", "Counts",
                  [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const int match_cluster = ev.r2t_matches[i];
            if (match_cluster >= 0)
              {
                const double delta_ref = ev.ref_clusters[i].delta_R_post();
                if (ev.cluster_same_cells[0][i])
                  {
                    group->global->Fill(delta_ref);
                  }
                if (ev.cluster_same_cells[1][i])
                  {
                    group->t1->Fill(delta_ref);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i])
                  {
                    group->t2->Fill(delta_ref);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i] && ev.cluster_same_cells[3][i])
                  {
                    group->t3->Fill(delta_ref);
                  }
              }
          }
      }

  },
  StyleKinds::ref),
  ref_name,
  add_plot<H1D_plotter_type>(
          "post_calc_delta_R_test",
          RETFUNC(min_delta_R[i]),
          RETFUNC(max_delta_R[i]),
          false, 0, std::string("Post-Process Calculated Clusters Distance") + " (" + test_name + ")", "#Delta R", "Counts",
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const int match_cluster = ev.r2t_matches[i];
            if (match_cluster >= 0)
              {
                const double delta_test = ev.test_clusters[match_cluster].delta_R_post();
                if (ev.cluster_same_cells[0][i])
                  {
                    group->global->Fill(delta_test);
                  }
                if (ev.cluster_same_cells[1][i])
                  {
                    group->t1->Fill(delta_test);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i])
                  {
                    group->t2->Fill(delta_test);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i] && ev.cluster_same_cells[3][i])
                  {
                    group->t3->Fill(delta_test);
                  }
              }
          }
      }

  },
  StyleKinds::test),
  test_name);


  add_plot<joined_plotter_type>(
          "post_calc_delta_eta", "Post-Processed Calculated #Delta #eta", "#eta^{(Impl)} - #eta^{(Post)}", "Counts",
          add_plot<H1D_plotter_type>(
                  "post_calc_delta_eta_ref",
                  RETFUNC(min_delta_eta[i]),
                  RETFUNC(max_delta_eta[i]),
                  true, 0, std::string("Post-Processed Calculated #Delta #eta") + " (" + ref_name + ")", "#eta^{(Impl)} - #eta^{(Post)}", "Counts",
                  [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const int match_cluster = ev.r2t_matches[i];
            if (match_cluster >= 0)
              {
                const double delta_ref = ev.ref_clusters[i].eta - ev.ref_clusters[i].eta_post;
                if (ev.cluster_same_cells[0][i])
                  {
                    group->global->Fill(delta_ref);
                  }
                if (ev.cluster_same_cells[1][i])
                  {
                    group->t1->Fill(delta_ref);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i])
                  {
                    group->t2->Fill(delta_ref);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i] && ev.cluster_same_cells[3][i])
                  {
                    group->t3->Fill(delta_ref);
                  }
              }
          }
      }

  },
  StyleKinds::ref),
  ref_name,
  add_plot<H1D_plotter_type>(
          "post_calc_delta_eta_test",
          RETFUNC(min_delta_eta[i]),
          RETFUNC(max_delta_eta[i]),
          true, 0, std::string("Post-Processed Calculated #Delta #eta") + " (" + test_name + ")", "#eta^{(Impl)} - #eta^{(Post)}", "Counts",
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const int match_cluster = ev.r2t_matches[i];
            if (match_cluster >= 0)
              {
                const double delta_test = ev.test_clusters[match_cluster].eta - ev.ref_clusters[i].eta_post;
                if (ev.cluster_same_cells[0][i])
                  {
                    group->global->Fill(delta_test);
                  }
                if (ev.cluster_same_cells[1][i])
                  {
                    group->t1->Fill(delta_test);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i])
                  {
                    group->t2->Fill(delta_test);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i] && ev.cluster_same_cells[3][i])
                  {
                    group->t3->Fill(delta_test);
                  }
              }
          }
      }

  },
  StyleKinds::test),
  test_name);


  add_plot<joined_plotter_type>(
          "post_calc_delta_phi", "Post-Processed Calculated #Delta #phi", "#phi^{(Impl)} - #phi^{(Post)}", "Counts",
          add_plot<H1D_plotter_type>(
                  "post_calc_delta_phi_ref",
                  RETFUNC(min_delta_phi[i]),
                  RETFUNC(max_delta_phi[i]),
                  true, 0, std::string("Post-Processed Calculated #Delta #phi") + " (" + ref_name + ")", "#phi^{(Impl)} - #phi^{(Post)}", "Counts",
                  [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const int match_cluster = ev.r2t_matches[i];
            if (match_cluster >= 0)
              {
                const double delta_ref = minDiffPhi(ev.ref_clusters[i].phi,
                                                    ev.ref_clusters[i].phi_post);
                if (ev.cluster_same_cells[0][i])
                  {
                    group->global->Fill(delta_ref);
                  }
                if (ev.cluster_same_cells[1][i])
                  {
                    group->t1->Fill(delta_ref);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i])
                  {
                    group->t2->Fill(delta_ref);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i] && ev.cluster_same_cells[3][i])
                  {
                    group->t3->Fill(delta_ref);
                  }
              }
          }
      }

  },
  StyleKinds::ref),
  ref_name,
  add_plot<H1D_plotter_type>(
          "post_calc_delta_phi_test",
          RETFUNC(min_delta_phi[i]),
          RETFUNC(max_delta_phi[i]),
          true, 0, std::string("Post-Processed Calculated #Delta #phi") + " (" + test_name + ")", "#phi^{(Impl)} - #phi^{(Post)}", "Counts",
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const int match_cluster = ev.r2t_matches[i];
            if (match_cluster >= 0)
              {
                const double delta_test = minDiffPhi(ev.test_clusters[match_cluster].phi,
                                                     ev.test_clusters[match_cluster].phi_post);
                if (ev.cluster_same_cells[0][i])
                  {
                    group->global->Fill(delta_test);
                  }
                if (ev.cluster_same_cells[1][i])
                  {
                    group->t1->Fill(delta_test);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i])
                  {
                    group->t2->Fill(delta_test);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i] && ev.cluster_same_cells[3][i])
                  {
                    group->t3->Fill(delta_test);
                  }
              }
          }
      }

  },
  StyleKinds::test),
  test_name);


  add_plot<joined_plotter_type>(
          "post_calc_delta_E", "Post-Processed Calculated #Delta E", "E^{(Impl)} - E^{(Post)} [GeV]", "Counts",
          add_plot<H1D_plotter_type>(
                  "post_calc_delta_E_ref",
                  RETFUNC(min_delta_E[i]),
                  RETFUNC(max_delta_E[i]),
                  true, 0, std::string("Post-Processed Calculated #Delta E") + " (" + ref_name + ")", "E^{(Impl)} - E^{(Post)} [GeV]", "Counts",
                  [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const int match_cluster = ev.r2t_matches[i];
            if (match_cluster >= 0)
              {
                const double delta_ref = ev.ref_clusters[i].energy - ev.ref_clusters[i].energy_post;
                if (ev.cluster_same_cells[0][i])
                  {
                    group->global->Fill(delta_ref);
                  }
                if (ev.cluster_same_cells[1][i])
                  {
                    group->t1->Fill(delta_ref);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i])
                  {
                    group->t2->Fill(delta_ref);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i] && ev.cluster_same_cells[3][i])
                  {
                    group->t3->Fill(delta_ref);
                  }
              }
          }
      }

  },
  StyleKinds::ref),
  ref_name,
  add_plot<H1D_plotter_type>(
          "post_calc_delta_E_test",
          RETFUNC(min_delta_E[i]),
          RETFUNC(max_delta_E[i]),
          true, 0, std::string("Post-Processed Calculated #Delta E") + " (" + test_name + ")", "E^{(Impl)} - E^{(Post)} [GeV]", "Counts",
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const int match_cluster = ev.r2t_matches[i];
            if (match_cluster >= 0)
              {
                const double delta_test = ev.test_clusters[match_cluster].energy - ev.test_clusters[match_cluster].energy_post;
                if (ev.cluster_same_cells[0][i])
                  {
                    group->global->Fill(delta_test);
                  }
                if (ev.cluster_same_cells[1][i])
                  {
                    group->t1->Fill(delta_test);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i])
                  {
                    group->t2->Fill(delta_test);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i] && ev.cluster_same_cells[3][i])
                  {
                    group->t3->Fill(delta_test);
                  }
              }
          }
      }

  },
  StyleKinds::test),
  test_name);


  add_plot<joined_plotter_type>(
          "post_calc_delta_E_rel", "Post-Processed Calculated #Delta E / E", "#(){E^{(Impl)} - E^{(Post)}}/#(){E^{(Post)}} ", "Counts",
          add_plot<H1D_plotter_type>(
                  "post_calc_delta_E_rel_ref",
                  RETFUNC(min_delta_E_rel[i]),
                  RETFUNC(max_delta_E_rel[i]),
                  true, 0, std::string("Post-Processed Calculated #Delta E / E") + " (" + ref_name + ")", "#(){E^{(Impl)} - E^{(Post)}}/#(){E^{(Post)}} ", "Counts",
                  [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const int match_cluster = ev.r2t_matches[i];
            if (match_cluster >= 0)
              {
                const double delta_ref = (ev.ref_clusters[i].energy - ev.ref_clusters[i].energy_post) / ev.ref_clusters[i].energy_post;
                if (std::abs(ev.ref_clusters[i].energy_post) < min_energy_cut ||
                    std::abs(ev.test_clusters[match_cluster].energy_post) < min_energy_cut)
                  {
                    continue;
                  }
                if (ev.cluster_same_cells[0][i])
                  {
                    group->global->Fill(delta_ref);
                  }
                if (ev.cluster_same_cells[1][i])
                  {
                    group->t1->Fill(delta_ref);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i])
                  {
                    group->t2->Fill(delta_ref);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i] && ev.cluster_same_cells[3][i])
                  {
                    group->t3->Fill(delta_ref);
                  }
              }
          }
      }

  },
  StyleKinds::ref),
  ref_name,
  add_plot<H1D_plotter_type>(
          "post_calc_delta_E_rel_test",
          RETFUNC(min_delta_E_rel[i]),
          RETFUNC(max_delta_E_rel[i]),
          true, 0, std::string("Post-Processed Calculated #Delta E / E") + " (" + test_name + ")", "#(){E^{(Impl)} - E^{(Post)}}/#(){E^{(Post)}} ", "Counts",
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const int match_cluster = ev.r2t_matches[i];
            if (match_cluster >= 0)
              {
                const double delta_test = (ev.test_clusters[match_cluster].energy - ev.test_clusters[match_cluster].energy_post) / ev.test_clusters[match_cluster].energy_post;
                if (std::abs(ev.ref_clusters[i].energy_post) < min_energy_cut ||
                    std::abs(ev.test_clusters[match_cluster].energy_post) < min_energy_cut)
                  {
                    continue;
                  }
                if (ev.cluster_same_cells[0][i])
                  {
                    group->global->Fill(delta_test);
                  }
                if (ev.cluster_same_cells[1][i])
                  {
                    group->t1->Fill(delta_test);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i])
                  {
                    group->t2->Fill(delta_test);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i] && ev.cluster_same_cells[3][i])
                  {
                    group->t3->Fill(delta_test);
                  }
              }
          }
      }

  },
  StyleKinds::test),
  test_name);

  add_plot<H1D_plotter_type>(
          "delta_post_E",
          RETFUNC(min_delta_E[i]),
          RETFUNC(max_delta_E[i]),
          true, 0, "Post-Processed Calculated #Delta E", "E^{(" + test_name + ", Post)} - E^{(" + ref_name + ", Post)} [GeV]", "Counts",
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const int match_cluster = ev.r2t_matches[i];
            if (match_cluster >= 0)
              {
                const double delta = ev.test_clusters[match_cluster].energy_post - ev.ref_clusters[i].energy_post;
                if (ev.cluster_same_cells[0][i])
                  {
                    group->global->Fill(delta);
                  }
                if (ev.cluster_same_cells[1][i])
                  {
                    group->t1->Fill(delta);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i])
                  {
                    group->t2->Fill(delta);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i] && ev.cluster_same_cells[3][i])
                  {
                    group->t3->Fill(delta);
                  }
              }
          }
      }

  },
  StyleKinds::joined);


  add_plot<H1D_plotter_type>(
          "delta_post_eta",
          RETFUNC(min_delta_E[i]),
          RETFUNC(max_delta_E[i]),
          true, 0, "Post-Processed Calculated #Delta #eta", "#eta^{(" + test_name + ", Post)} - #eta^{(" + ref_name + ", Post)}", "Counts",
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const int match_cluster = ev.r2t_matches[i];
            if (match_cluster >= 0)
              {
                const double delta = ev.test_clusters[match_cluster].eta_post - ev.ref_clusters[i].eta_post;
                if (ev.cluster_same_cells[0][i])
                  {
                    group->global->Fill(delta);
                  }
                if (ev.cluster_same_cells[1][i])
                  {
                    group->t1->Fill(delta);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i])
                  {
                    group->t2->Fill(delta);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i] && ev.cluster_same_cells[3][i])
                  {
                    group->t3->Fill(delta);
                  }
              }
          }
      }

  },
  StyleKinds::joined);

  add_plot<H1D_plotter_type>(
          "delta_post_phi",
          RETFUNC(min_delta_E[i]),
          RETFUNC(max_delta_E[i]),
          true, 0, "Post-Processed Calculated #Delta #phi", "#phi^{(" + test_name + ", Post)} - #phi^{(" + ref_name + ", Post)}", "Counts",
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const int match_cluster = ev.r2t_matches[i];
            if (match_cluster >= 0)
              {
                const double delta = minDiffPhi(ev.test_clusters[match_cluster].phi_post, ev.ref_clusters[i].phi_post);
                if (ev.cluster_same_cells[0][i])
                  {
                    group->global->Fill(delta);
                  }
                if (ev.cluster_same_cells[1][i])
                  {
                    group->t1->Fill(delta);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i])
                  {
                    group->t2->Fill(delta);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i] && ev.cluster_same_cells[3][i])
                  {
                    group->t3->Fill(delta);
                  }
              }
          }
      }

  },
  StyleKinds::joined);

  add_plot<H1D_plotter_type>(
          "delta_post_E_rel",
          RETFUNC(1),
          RETFUNC(-1),
          true, 0, "Post-Processed Calculated #Delta E / E", "2 #times #(){E^{(" + test_name + ", Post)} - E^{(" + ref_name + ", Post)}}/#(){E^{(" + test_name + ", Post)} + E^{(" + ref_name + ", Post)}}", "Counts",
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (size_t i = 0; i < ev.ref_clusters.size(); ++i)
          {
            const int match_cluster = ev.r2t_matches[i];
            if (match_cluster >= 0)
              {
                const double E_ref = ev.ref_clusters[i].energy_post;
                const double E_test = ev.test_clusters[i].energy_post;
                const double delta = 2 * (E_test - E_ref) / (E_test + E_ref);
                if (std::abs(E_ref + E_test) < 2 * min_energy_cut)
                  {
                    continue;
                  }
                if (ev.cluster_same_cells[0][i])
                  {
                    group->global->Fill(delta);
                  }
                if (ev.cluster_same_cells[1][i])
                  {
                    group->t1->Fill(delta);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i])
                  {
                    group->t2->Fill(delta);
                  }
                if (ev.cluster_same_cells[1][i] && ev.cluster_same_cells[2][i] && ev.cluster_same_cells[3][i])
                  {
                    group->t3->Fill(delta);
                  }
              }
          }
      }

  },
  StyleKinds::joined);

  add_plot<H1D_plotter_region>(
          "cell_E_dist",
          RETFUNC(-5000),
          RETFUNC(5000),
          true, 0, "Cell Energy", "Cell Energy [GeV]", (normalize ? "Average Cells per Event" : "Cells"),
          [&](hist_group_1D * group)
  {
    for (const auto & ev : events)
      {
        for (int i = 0; i < NCaloCells; ++i)
          {
            fill_regions(group, geometry->eta[i], ev.energy_array[i] * 1e-3);
          }
      }

  },
  StyleKinds::joined);

  std::cout << "Finished populating plots." << std::endl;
}

#undef RETFUNC

#endif //CALORECGPU_TOOLS_CLUSTERPLOTTERPLOTS_H