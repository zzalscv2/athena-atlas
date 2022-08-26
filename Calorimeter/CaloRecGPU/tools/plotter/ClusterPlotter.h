// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
//

#ifndef CALORECGPU_TOOLS_CLUSTERPLOTTER_H
#define CALORECGPU_TOOLS_CLUSTERPLOTTER_H

#include <vector>
#include <string>
#include <set>
#include <limits>
#include <utility>

#include <functional>

#include <sstream>
#include <numeric>
#include <iomanip>

#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "CaloRecGPU/StandaloneDataIO.h"
#include "PlotterAuxDefines.h"

#include "CxxUtils/checker_macros.h"

using namespace CaloRecGPU;

#ifdef ATLAS_CHECK_THREAD_SAFETY

  ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#endif


constexpr double default_min_similarity = 0.9;
constexpr double default_term_weight = 1.0;
constexpr double default_grow_weight = 250;
constexpr double default_seed_weight = 5000;


struct EventData
{
  std::vector<ClusterData> ref_clusters, test_clusters;
  std::vector<float> SNR_array;
  std::vector<float> energy_array;
  std::vector<int> ref_tag_array, test_tag_array;
  std::vector<int> r2t_matches, t2r_matches;
  std::vector<int> ref_unmatched, test_unmatched;

  std::vector<double> delta_R_array;
  std::vector<double> delta_Et_rel_array;
  //Indexed by reference cluster!
  //(We seem to get less reference clusters, on average.)

  ClusterData min_vals[4], max_vals[4], unmatched_min_vals[4], unmatched_max_vals[4];

  float min_reg_energy[4], max_reg_energy[4], min_reg_SNR[4], max_reg_SNR[4];
  //By region (all, central, end-cap or forward)

  float min_type_energy[4], max_type_energy[4], min_type_SNR[4], max_type_SNR[4];
  //By type (all, seed, grow, terminal)

  double min_delta_R[4], max_delta_R[4];

  double min_delta_E[4], max_delta_E[4];

  double min_delta_Et[4], max_delta_Et[4];

  double min_delta_E_rel[4], max_delta_E_rel[4];

  double min_delta_Et_rel[4], max_delta_Et_rel[4];

  double min_delta_eta[4], max_delta_eta[4];

  double min_delta_phi[4], max_delta_phi[4];

  int max_cluster_number[4], min_cluster_number[4];
  int max_unmatched_number[4], min_unmatched_number[4];

  bool test_has_more[4];

  int cluster_diff[4];

  double cell_diff_frac_reg[4], cell_diff_frac_type[4];

  std::vector<bool> cluster_same_cells[4];

  void find_matches(const double min_similarity = default_min_similarity,
                    const double term_weight = default_term_weight,
                    const double grow_weight = default_grow_weight,
                    const double seed_weight = default_seed_weight)
  {
    std::vector<int> similarity_map(test_clusters.size() * ref_clusters.size(), 0.f);

    std::vector<double> ref_normalization(ref_clusters.size(), 0.f);
    std::vector<double> test_normalization(test_clusters.size(), 0.f);


    for (int i = 0; i < NCaloCells; ++i)
      {
        const int ref_tag = ref_tag_array[i];
        const int test_tag = test_tag_array[i];
        const double SNR = std::abs(SNR_array[i]);
        const double weight = SNR * ( SNR > SNR_thresholds[2] ? seed_weight :
                                      (
                                              SNR > SNR_thresholds[1] ? grow_weight :
                                              (
                                                      SNR > SNR_thresholds[0] ? term_weight :
                                                      0
                                              )
                                      )
                                    );
        if (ref_tag >= 0 && test_tag >= 0)
          {
            similarity_map[test_tag * ref_clusters.size() + ref_tag] += weight;
          }
        if (ref_tag >= 0)
          {
            ref_normalization[ref_tag] += weight;
          }
        if (test_tag >= 0)
          {
            test_normalization[test_tag] += weight;
          }
      }

    /*
      for (int tc = 0; tc < test_clusters.size(); ++tc)
      {
      for (int rc = 0; rc < ref_clusters.size(); ++rc)
      {
      const double simil = similarity_map[tc * ref_clusters.size() + rc];
      if (simil > 0)
      {
      std::cout << tc << " " << rc << " " << simil << " " << simil/ref_normalization[rc] << " " << simil/test_normalization[tc] << std::endl;
      }
      }
      }

      std::cout << " --- " << std::endl;
    */


    //In essence, the Gale-Shapley Algorithm

    std::vector<std::vector<int>> sorted_GPU_matches;

    sorted_GPU_matches.reserve(test_clusters.size());

    for (size_t testc = 0; testc < test_clusters.size(); ++testc)
      {
        std::vector<int> sorter(ref_clusters.size());
        std::iota(sorter.begin(), sorter.end(), 0);

        std::sort(sorter.begin(), sorter.end(),
                  [&](const int a, const int b)
        {
          const double a_weight = similarity_map[testc * ref_clusters.size() + a];
          const double b_weight = similarity_map[testc * ref_clusters.size() + b];
          return a_weight > b_weight;
        }
                 );

        size_t wanted_size = 0;

        for (; wanted_size < sorter.size(); ++wanted_size)
          {
            const double match_weight = similarity_map[testc * ref_clusters.size() + sorter[wanted_size]] / test_normalization [testc];
            if (match_weight < min_similarity)
              {
                break;
              }
          }

        //Yeah, we could do a binary search for best worst-case complexity,
        //but we are expecting 1~2 similar clusters and the rest garbage,
        //so we're expecting only 1~2 iterations.
        //This actually means all that sorting is way way overkill,
        //but we must make sure in the most general case that this works...

        sorter.resize(wanted_size);

        sorted_GPU_matches.push_back(sorter);
      }

    int num_iter = 0;

    constexpr int max_iter = 32;

    std::vector<double> matched_weights(ref_clusters.size(), -1.);

    std::vector<size_t> skipped_matching(test_clusters.size(), 0);

    for (size_t stop_counter = 0; stop_counter < test_clusters.size() && num_iter < max_iter; ++num_iter)
      {
        stop_counter = 0;
        for (size_t testc = 0; testc < sorted_GPU_matches.size(); ++testc)
          {
            if (skipped_matching[testc] < sorted_GPU_matches[testc].size())
              {
                const int match_c = sorted_GPU_matches[testc][skipped_matching[testc]];
                const double match_weight = similarity_map[testc * ref_clusters.size() + match_c] / ref_normalization[match_c];
                if (match_weight >= min_similarity && match_weight > matched_weights[match_c])
                  {
                    const int prev_match = r2t_matches[match_c];
                    if (prev_match >= 0)
                      {
                        ++skipped_matching[prev_match];
                        --stop_counter;
                      }
                    r2t_matches[match_c] = testc;
                    matched_weights[match_c] = match_weight;
                    ++stop_counter;
                  }
                else
                  {
                    ++skipped_matching[testc];
                  }
              }
            else
              {
                ++stop_counter;
              }
          }
      }

    ref_unmatched.clear();
    test_unmatched.clear();

    for (size_t i = 0; i < r2t_matches.size(); ++i)
      {
        const int match = r2t_matches[i];
        if (r2t_matches[i] < 0)
          {
            ref_unmatched.push_back(i);
          }
        else
          {
            t2r_matches[match] = i;
          }
      }

    for (size_t i = 0; i < t2r_matches.size(); ++i)
      {
        if (t2r_matches[i] < 0)
          {
            test_unmatched.push_back(i);
          }
      }

    std::cout << "Matched in " << num_iter << " iterations: "
              << r2t_matches.size() - ref_unmatched.size() << " / " << r2t_matches.size() << "  ||  "
              << t2r_matches.size() - test_unmatched.size() << " / " << t2r_matches.size() << "  ||  " << ref_unmatched.size() << " | " << test_unmatched.size() << std::endl;
    /*
      {
      std::vector<int> testc(t2r_matches.size(), 0);

      for (int i = 0; i < r2t_matches.size(); ++i)
      {
      const int res = r2t_matches[i];
      if (res >= 0)
      {
      testc[res] += 1;
      }
      }

      for (int i = 0; i < testc.size(); ++i)
      {
      if (testc[i] != 1)
      {
      std::cout << i << ": " << testc[i] << "\n";
      }
      }

      std::cout << "\n\n----------" << std::endl;
      }
    */

  }


  EventData(const GeometryArr & geometry,
            const CellNoiseArr & noise_arr,
            const CellInfoArr & cell_info,
            const CellStateArr & ref_cells,
            const CellStateArr & test_cells,
            const ClusterInfoArr & reference,
            const ClusterInfoArr & test,
            const double min_similarity = default_min_similarity,
            const double term_weight = default_term_weight,
            const double grow_weight = default_grow_weight,
            const double seed_weight = default_seed_weight):
    ref_clusters(reference.number),
    test_clusters(test.number),
    SNR_array(NCaloCells),
    energy_array(NCaloCells),
    ref_tag_array(NCaloCells),
    test_tag_array(NCaloCells),
    r2t_matches(reference.number, -1),
    t2r_matches(test.number, -1),
    delta_R_array(reference.number, -1),
    delta_Et_rel_array(reference.number, -2)
    //delta_Et_rel >= -1 by definition
  {
    std::vector<int> ref_seed_cells(ref_clusters.size(), -1);
    std::vector<int> test_seed_cells(test_clusters.size(), -1);
    std::vector<float> ref_sc_snr(ref_clusters.size(), 0);
    std::vector<float> test_sc_snr(test_clusters.size(), 0);

    for (int i = 0; i < 4; ++i)
      {
        set_to_highest(min_reg_energy[i]);
        set_to_highest(min_reg_SNR[i]);

        set_to_lowest(max_reg_energy[i]);
        set_to_lowest(max_reg_SNR[i]);


        set_to_highest(min_type_energy[i]);
        set_to_highest(min_type_SNR[i]);

        set_to_lowest(max_type_energy[i]);
        set_to_lowest(max_type_SNR[i]);

      }


    for (size_t i = 0; i < ref_clusters.size(); ++i)
      {
        ref_clusters[i].num_cells = 0;
      }

    for (size_t i = 0; i < test_clusters.size(); ++i)
      {
        test_clusters[i].num_cells = 0;
      }


    for (int i = 0; i < NCaloCells; ++i)
      {
        const float this_energy = cell_info.energy[i];

        const int this_gain = cell_info.gain[i];


        float this_SNR = 0.00001;

        if (!GainConversion::is_bad_cell(this_gain))
          {
            const int corrected_gain = GainConversion::recover_invalid_seed_cell_gain(this_gain);
            const float this_noise = noise_arr.noise[corrected_gain][i];

            if (finite(this_noise) && this_noise > 0.0f)
              {
                this_SNR = std::abs(this_energy / this_noise);
              }
            if (this_SNR > SNR_thresholds[2] && GainConversion::is_invalid_seed_cell(this_gain))
              {
                this_SNR = (SNR_thresholds[2] + SNR_thresholds[1]) / 2;
                //Hack to turn invalid seeds to growing.
                //Does slightly skew the SNR metric, but...
              }
          }

        SNR_array[i] = this_SNR;

        const float this_scale_energy = this_energy * 1e-3;

        energy_array[i] = this_scale_energy;

        operate_on_regions(min_reg_energy, geometry.eta[i], [&](float * en)
        {
          *en = std::min(*en, this_scale_energy);
        });
        operate_on_regions(max_reg_energy, geometry.eta[i], [&](float * en)
        {
          *en = std::max(*en, this_scale_energy);
        });

        operate_on_regions(min_reg_SNR, geometry.eta[i], [&](float * snr)
        {
          *snr = std::min(*snr, this_SNR);
        });
        operate_on_regions(max_reg_SNR, geometry.eta[i], [&](float * snr)
        {
          *snr = std::max(*snr, this_SNR);
        });


        operate_on_types(min_type_energy, this_SNR, [&](float * en)
        {
          *en = std::min(*en, this_scale_energy);
        });
        operate_on_types(max_type_energy, this_SNR, [&](float * en)
        {
          *en = std::max(*en, this_scale_energy);
        });

        operate_on_types(min_type_SNR, geometry.eta[i], [&](float * snr)
        {
          *snr = std::min(*snr, this_SNR);
        });
        operate_on_types(max_type_SNR, geometry.eta[i], [&](float * snr)
        {
          *snr = std::max(*snr, this_SNR);
        });

        int ref_tag = get_legacy_tag(ref_cells.clusterTag[i]);
        int test_tag = get_legacy_tag(test_cells.clusterTag[i]);

        if (ref_tag >= 0 && ref_tag > int(ref_clusters.size()))
          {
            std::cout << "ERROR! Invalid tag: " << ref_tag << " " << std::hex << ref_cells.clusterTag[i] << std::dec << " (" << ref_clusters.size() << ")" << std::endl;
            ref_tag = -1;
          }

        if (test_tag >= 0 && test_tag > int(test_clusters.size()))
          {
            std::cout << "ERROR! Invalid tag: " << test_tag << " " << std::hex << test_cells.clusterTag[i] << std::dec << " (" << test_clusters.size() << ")" << std::endl;
            test_tag = -1;
          }

        ref_tag_array[i] = ref_tag;
        test_tag_array[i] = test_tag;

        if (ref_tag >= 0)
          {
            if (this_SNR > ref_sc_snr[ref_tag])
              {
                ref_sc_snr[ref_tag] = this_SNR;
                ref_seed_cells[ref_tag] = i;
                ++ref_clusters[ref_tag].num_cells;
              }
          }
        if (test_tag >= 0)
          {
            if (this_SNR > test_sc_snr[test_tag])
              {
                test_sc_snr[test_tag] = this_SNR;
                test_seed_cells[test_tag] = i;
                ++test_clusters[test_tag].num_cells;
              }
          }

      }


    for (size_t i = 0; i < ref_clusters.size(); ++i)
      {
        ref_clusters[i].abs_energy = 0;
      }

    for (size_t i = 0; i < test_clusters.size(); ++i)
      {
        test_clusters[i].abs_energy = 0;
      }

    for (int i = 0; i < NCaloCells; ++i)
      {

        const int ref_tag = ref_tag_array[i];
        const int test_tag = test_tag_array[i];

        const float this_energy = energy_array[i];

        const float this_abs_energy = std::abs(this_energy);

        if (ref_tag >= 0 && ref_seed_cells[ref_tag] >= 0)
          {
            ref_clusters[ref_tag].abs_energy += this_abs_energy;
            ref_clusters[ref_tag].eta_post += geometry.eta[i] * this_abs_energy;
            ref_clusters[ref_tag].phi_post += proxim_ath(geometry.phi[i], geometry.phi[ref_seed_cells[ref_tag]]) * this_abs_energy;
            ref_clusters[ref_tag].energy_post += this_energy;
          }
        if (test_tag >= 0 && test_seed_cells[test_tag] >= 0)
          {
            test_clusters[test_tag].abs_energy += this_abs_energy;
            test_clusters[test_tag].eta_post += geometry.eta[i] * this_abs_energy;
            test_clusters[test_tag].phi_post += proxim_ath(geometry.phi[i], geometry.phi[test_seed_cells[test_tag]]) * this_abs_energy;
            test_clusters[test_tag].energy_post += this_energy;
          }
      }

    int ref_cluster_counter[4], test_cluster_counter[4];

    for (int i = 0; i < 4; ++i)
      {
        set_to_lowest(max_vals[i]);

        set_to_highest(min_vals[i]);

        ref_cluster_counter[i] = 0;
        test_cluster_counter[i] = 0;
      }

    for (int i = 0; i < reference.number; ++i)
      {

        const float energy = reference.clusterEnergy[i] * 1e-3;
        const float transverse_energy = reference.clusterEt[i] * 1e-3;
        //So the energies are in GeV!
        const float eta = reference.clusterEta[i];
        const float phi = reference.clusterPhi[i];

        ref_clusters[i].set(ref_clusters[i].num_cells, energy, transverse_energy, eta, phi);

        operate_on_regions(max_vals, eta, [&](ClusterData * clu)
        {
          clu->set_to_max(ref_clusters[i]);
        });
        operate_on_regions(min_vals, eta, [&](ClusterData * clu)
        {
          clu->set_to_min(ref_clusters[i]);
        });

        operate_on_regions(ref_cluster_counter, eta, [](int * val)
        {
          *val = *val + 1;
        } );

        ref_clusters[i].eta_post /= ref_clusters[i].abs_energy;
        ref_clusters[i].phi_post /= ref_clusters[i].abs_energy;
        ref_clusters[i].phi_post = wrapPhi(ref_clusters[i].phi_post);

        ref_clusters[i].abs_energy *= 1e-3;
        ref_clusters[i].energy_post *= 1e-3;
      }

    for (int i = 0; i < test.number; ++i)
      {
        const float energy = test.clusterEnergy[i] * 1e-3;
        const float transverse_energy = test.clusterEt[i] * 1e-3;
        //So the energies are in GeV!
        const float eta = test.clusterEta[i];
        const float phi = test.clusterPhi[i];

        test_clusters[i].set(test_clusters[i].num_cells, energy, transverse_energy, eta, phi);

        operate_on_regions(max_vals, eta, [&](ClusterData * clu)
        {
          clu->set_to_max(test_clusters[i]);
        });
        operate_on_regions(min_vals, eta, [&](ClusterData * clu)
        {
          clu->set_to_min(test_clusters[i]);
        });

        operate_on_regions(test_cluster_counter, eta, [](int * val)
        {
          *val = *val + 1;
        } );

        test_clusters[i].eta_post /= test_clusters[i].abs_energy;
        test_clusters[i].phi_post /= test_clusters[i].abs_energy;
        test_clusters[i].phi_post = wrapPhi(test_clusters[i].phi_post);

        test_clusters[i].abs_energy *= 1e-3;
        test_clusters[i].energy_post *= 1e-3;
      }

    for (int i = 0; i < 4; ++i)
      {
        max_cluster_number[i] = std::max(ref_cluster_counter[i], test_cluster_counter[i]);
        min_cluster_number[i] = std::min(ref_cluster_counter[i], test_cluster_counter[i]);
        test_has_more[i] = test_cluster_counter[i] > ref_cluster_counter[i];
        cluster_diff[i] = int(test_cluster_counter[i]) - int(ref_cluster_counter[i]);
      }

    find_matches(min_similarity, term_weight, grow_weight, seed_weight);

    for (int i = 0; i < 4; ++i)
      {
        set_to_lowest(max_delta_R[i]);
        set_to_highest(min_delta_R[i]);

        set_to_lowest(max_delta_E[i]);
        set_to_highest(min_delta_E[i]);

        set_to_lowest(max_delta_E_rel[i]);
        set_to_highest(min_delta_E_rel[i]);

        set_to_lowest(max_delta_Et[i]);
        set_to_highest(min_delta_Et[i]);

        set_to_lowest(max_delta_Et_rel[i]);
        set_to_highest(min_delta_Et_rel[i]);

        set_to_lowest(max_delta_eta[i]);
        set_to_highest(min_delta_eta[i]);

        set_to_lowest(max_delta_phi[i]);
        set_to_highest(min_delta_phi[i]);

        ref_cluster_counter[i] = 0;
        test_cluster_counter[i] = 0;

        set_to_lowest(unmatched_max_vals[i]);

        set_to_highest(unmatched_min_vals[i]);
      }

    for (size_t refc = 0; refc < r2t_matches.size(); ++refc)
      {
        const int this_match = r2t_matches[refc];
        if (this_match >= 0)
          {
            const ClusterData * this_cluster = &(ref_clusters[refc]);
            const ClusterData * match_cluster = &(test_clusters[this_match]);

            const double delta_R = match_cluster->delta_R(*this_cluster);

            const double delta_E = match_cluster->energy - this_cluster->energy;

            const double delta_E_rel = std::abs(delta_E / this_cluster->energy);

            const double delta_Et = match_cluster->transverse_energy - this_cluster->transverse_energy;

            const double delta_Et_rel = delta_Et / this_cluster->transverse_energy;


            const double delta_eta = (match_cluster->eta - this_cluster->eta);
            const double delta_phi = minDiffPhi(match_cluster->phi, this_cluster->phi);

            delta_R_array[refc] = delta_R;

            delta_Et_rel_array[refc] = delta_Et_rel;

            operate_on_regions(max_delta_R, ref_clusters[refc].eta, [&](double * dr)
            {
              *dr = std::max(*dr, delta_R);
            });
            operate_on_regions(min_delta_R, ref_clusters[refc].eta, [&](double * dr)
            {
              *dr = std::min(*dr, delta_R);
            });

            operate_on_regions(max_delta_E, ref_clusters[refc].eta, [&](double * dE)
            {
              *dE = std::max(*dE, delta_E);
            });
            operate_on_regions(min_delta_E, ref_clusters[refc].eta, [&](double * dE)
            {
              *dE = std::min(*dE, delta_E);
            });

            operate_on_regions(max_delta_Et, ref_clusters[refc].eta, [&](double * dE)
            {
              *dE = std::max(*dE, delta_Et);
            });
            operate_on_regions(min_delta_Et, ref_clusters[refc].eta, [&](double * dE)
            {
              *dE = std::min(*dE, delta_Et);
            });

            operate_on_regions(max_delta_E_rel, ref_clusters[refc].eta, [&](double * dE)
            {
              *dE = std::max(*dE, delta_E_rel);
            });
            operate_on_regions(min_delta_E_rel, ref_clusters[refc].eta, [&](double * dE)
            {
              *dE = std::min(*dE, delta_E_rel);
            });

            operate_on_regions(max_delta_Et_rel, ref_clusters[refc].eta, [&](double * dE)
            {
              *dE = std::max(*dE, delta_Et_rel);
            });
            operate_on_regions(min_delta_Et_rel, ref_clusters[refc].eta, [&](double * dE)
            {
              *dE = std::min(*dE, delta_Et_rel);
            });

            operate_on_regions(max_delta_phi, ref_clusters[refc].eta, [&](double * dphi)
            {
              *dphi = std::max(*dphi, delta_phi);
            });
            operate_on_regions(min_delta_phi, ref_clusters[refc].eta, [&](double * dphi)
            {
              *dphi = std::min(*dphi, delta_phi);
            });

            operate_on_regions(max_delta_eta, ref_clusters[refc].eta, [&](double * deta)
            {
              *deta = std::max(*deta, delta_eta);
            });
            operate_on_regions(min_delta_eta, ref_clusters[refc].eta, [&](double * deta)
            {
              *deta = std::min(*deta, delta_eta);
            });
          }
        else
          {
            operate_on_regions(ref_cluster_counter, ref_clusters[refc].eta, [](int * val)
            {
              *val = *val + 1;
            } );

            operate_on_regions(unmatched_max_vals, ref_clusters[refc].eta, [&](ClusterData * clu)
            {
              clu->set_to_max(test_clusters[refc]);
            });
            operate_on_regions(unmatched_min_vals, ref_clusters[refc].eta, [&](ClusterData * clu)
            {
              clu->set_to_min(test_clusters[refc]);
            });
          }
        //Unmatched clusters get DeltaR == -1
      }

    for (const auto test_nomatch : test_unmatched)
      {
        operate_on_regions(test_cluster_counter, test_clusters[test_nomatch].eta, [](int * val)
        {
          *val = *val + 1;
        } );
        operate_on_regions(unmatched_max_vals, test_clusters[test_nomatch].eta,
                           [&](ClusterData * clu)
        {
          clu->set_to_max(test_clusters[test_nomatch]);
        });
        operate_on_regions(unmatched_min_vals, test_clusters[test_nomatch].eta,
                           [&](ClusterData * clu)
        {
          clu->set_to_min(test_clusters[test_nomatch]);
        });
      }

    for (int i = 0; i < 4; ++i)
      {
        max_unmatched_number[i] = std::max(ref_cluster_counter[i], test_cluster_counter[i]);
        min_unmatched_number[i] = std::min(ref_cluster_counter[i], test_cluster_counter[i]);
      }


    int cell_diff_type[4];
    int cell_diff_reg[4];
    int cell_count_type[4];
    int cell_count_reg[4];

    for (int i = 0; i < 4; ++i)
      {
        cell_diff_type[i] = 0;
        cell_diff_reg[i] = 0;
        cluster_same_cells[i].resize(ref_clusters.size(), true);
      }

    for (int i = 0; i < NCaloCells; ++i)
      {
        const int ref_tag = get_legacy_tag(ref_cells.clusterTag[i]);
        const int test_tag = get_legacy_tag(test_cells.clusterTag[i]);
        if (ref_tag < 0 && test_tag < 0)
          {
            //Do nothing in this case: they are both off.
          }
        else if ( (ref_tag < 0 && test_tag >= 0) || (ref_tag >= 0 && test_tag < 0) ||
                  r2t_matches[ref_tag] != test_tag || t2r_matches[test_tag] != ref_tag )
          {
            operate_on_types(cell_diff_type, SNR_array[i], [](int * count)
            {
              (*count) = *count + 1;
            });
            operate_on_regions(cell_diff_reg, geometry.eta[i], [](int * count)
            {
              (*count) = *count + 1;
            });
          }

        if (ref_tag >= 0 && r2t_matches[ref_tag] != test_tag)
          {
            operate_on_types(cluster_same_cells, SNR_array[i], [&](std::vector<bool> * v)
            {
              (*v)[ref_tag] = false;
            });
          }
        if (test_tag >= 0 && t2r_matches[test_tag] != ref_tag && t2r_matches[test_tag] >= 0)
          {
            operate_on_types(cluster_same_cells, SNR_array[i], [&](std::vector<bool> * v)
            {
              (*v)[t2r_matches[test_tag]] = false;
            });
          }

        operate_on_types(cell_count_type, SNR_array[i], [](int * count)
        {
          (*count) = *count + 1;
        });
        operate_on_regions(cell_count_reg, geometry.eta[i], [](int * count)
        {
          (*count) = *count + 1;
        });
      }

    for (int i = 0; i < 4; ++i)
      {
        cell_diff_frac_type[i] = double(cell_diff_type[i]) / double(cell_count_type[i]);
        cell_diff_frac_reg[i] = double(cell_diff_reg[i]) / double(cell_count_reg[i]);
      }
  }


};

struct ClusterPlotter : public BasePlotter
{
  std::vector<EventData> events;
  Helpers::CPU_object<GeometryArr> geometry;
  Helpers::CPU_object<CellNoiseArr> noise;


  double min_energy_cut = 1e-3;

  int min_size_cut = 128;


  ClusterData min_vals[4], max_vals[4], unmatched_min_vals[4], unmatched_max_vals[4];

  double min_delta_R[4], max_delta_R[4];


  double min_delta_E[4], max_delta_E[4];

  double min_delta_Et[4], max_delta_Et[4];

  double min_delta_E_rel[4], max_delta_E_rel[4];

  double min_delta_Et_rel[4], max_delta_Et_rel[4];

  double min_delta_eta[4], max_delta_eta[4];

  double min_delta_phi[4], max_delta_phi[4];


  float min_reg_energy[4], max_reg_energy[4], min_reg_SNR[4], max_reg_SNR[4];
  //By region (all, central, end-cap or forward)

  float min_type_energy[4], max_type_energy[4], min_type_SNR[4], max_type_SNR[4];
  //By type (all, seed, grow, terminal)

  int max_cluster_number[4], min_cluster_number[4];

  int max_unmatched_number[4], min_unmatched_number[4];

  int max_cluster_diff[4], min_cluster_diff[4];

  ClusterPlotter(const ClusterPlotter &) = delete;
  ClusterPlotter(ClusterPlotter &&) = default;

  ClusterPlotter & operator= (const ClusterPlotter &) = delete;
  ClusterPlotter & operator= (ClusterPlotter &&) = default;

  ~ClusterPlotter()
  {
  }


  virtual size_t num() const
  {
    return events.size();
  }

  ClusterPlotter(const boost::filesystem::path & constants_folder_path,
                 const boost::filesystem::path & reference_folder_path,
                 const boost::filesystem::path & test_folder_path,
                 const int max_events = -1,
                 const std::string & filter = "",
                 const double min_similarity = default_min_similarity,
                 const double term_weight = default_term_weight,
                 const double grow_weight = default_grow_weight,
                 const double seed_weight = default_seed_weight)
  {

    auto filename_filter = [&](const std::string & str)
    {
      return str.find(filter) == std::string::npos;
    };

    auto const_folder = StandaloneDataIO::load_folder_filter(filename_filter, constants_folder_path,
                                                             max_events, false, false, true, true);
    //Don't load clusters; need geometry and noise, obviously, but also cells
    //so we get a standard value for energies and gains
    //(which, in the current version, should be outputted by the GPU output,
    // so it should in principle be the same as the test_folder...)

    geometry = const_folder.geometry.begin()->second;
    noise = const_folder.noise.begin()->second;

    auto ref_folder = StandaloneDataIO::load_folder_filter(filename_filter, reference_folder_path,
                                                           max_events, true, true, false, false);
    auto test_folder = StandaloneDataIO::load_folder_filter(filename_filter, test_folder_path,
                                                            max_events, true, true, false, false);

    auto map_title_fixer = [](auto & map)
    {
      for (auto & it : map)
        {
          auto nodeHandler = map.extract(it.first);
          std::string new_title = it.first;
          auto endstring_it = std::remove_if(new_title.begin(), new_title.end(), [](const char & c)
          {
            return !std::isdigit(c);
          });
          new_title.erase(endstring_it, new_title.end());

          nodeHandler.key() = new_title;
          map.insert(std::move(nodeHandler));
        }
    };

    map_title_fixer(ref_folder.cell_state);
    map_title_fixer(ref_folder.clusters);
    map_title_fixer(test_folder.cell_info);
    map_title_fixer(test_folder.cell_state);
    map_title_fixer(test_folder.clusters);

    for (const auto & it : test_folder.clusters)
      {
        events.emplace_back(*geometry, *noise, *(test_folder.cell_info.at(it.first)),
                            *(ref_folder.cell_state.at(it.first)), *(test_folder.cell_state.at(it.first)),
                            *(ref_folder.clusters.at(it.first)), *(test_folder.clusters.at(it.first)),
                            min_similarity, term_weight, grow_weight, seed_weight                           );
      }

    for (int i = 0; i < 4; ++i)
      {
        set_to_lowest(max_vals[i]);

        set_to_highest(min_vals[i]);

        set_to_highest(min_reg_energy[i]);
        set_to_highest(min_reg_SNR[i]);

        set_to_lowest(max_reg_energy[i]);
        set_to_lowest(max_reg_SNR[i]);

        set_to_highest(min_type_energy[i]);
        set_to_highest(min_type_SNR[i]);

        set_to_lowest(max_type_energy[i]);
        set_to_lowest(max_type_SNR[i]);

        set_to_lowest(max_delta_R[i]);
        set_to_highest(min_delta_R[i]);

        set_to_lowest(max_cluster_number[i]);
        set_to_highest(min_cluster_number[i]);

        set_to_lowest(max_unmatched_number[i]);
        set_to_highest(min_unmatched_number[i]);

        set_to_lowest(max_cluster_diff[i]);
        set_to_highest(min_cluster_diff[i]);


        set_to_lowest(max_delta_eta[i]);
        set_to_highest(min_delta_eta[i]);

        set_to_lowest(max_delta_phi[i]);
        set_to_highest(min_delta_phi[i]);

        set_to_lowest(unmatched_max_vals[i]);

        set_to_highest(unmatched_min_vals[i]);
      }

    for (const auto & ev : events)
      {
        for (int i = 0; i < 4; ++i)
          {
            min_vals[i].set_to_min(ev.min_vals[i]);
            max_vals[i].set_to_max(ev.max_vals[i]);

            min_type_energy[i] = std::min(min_type_energy[i], ev.min_type_energy[i]);
            min_type_SNR[i] = std::min(min_type_SNR[i], ev.min_type_SNR[i]);
            min_reg_energy[i] = std::min(min_reg_energy[i], ev.min_reg_energy[i]);
            min_reg_SNR[i] = std::min(min_reg_SNR[i], ev.min_reg_SNR[i]);

            max_type_energy[i] = std::max(max_type_energy[i], ev.max_type_energy[i]);
            max_type_SNR[i] = std::max(max_type_SNR[i], ev.max_type_SNR[i]);
            max_reg_energy[i] = std::max(max_reg_energy[i], ev.max_reg_energy[i]);
            max_reg_SNR[i] = std::max(max_reg_SNR[i], ev.max_reg_SNR[i]);

            min_delta_R[i] = std::min(min_delta_R[i], ev.min_delta_R[i]);
            max_delta_R[i] = std::max(max_delta_R[i], ev.max_delta_R[i]);

            min_delta_E[i] = std::min(min_delta_E[i], ev.min_delta_E[i]);
            max_delta_E[i] = std::max(max_delta_E[i], ev.max_delta_E[i]);

            min_delta_Et[i] = std::min(min_delta_Et[i], ev.min_delta_Et[i]);
            max_delta_Et[i] = std::max(max_delta_Et[i], ev.max_delta_Et[i]);

            min_delta_E_rel[i] = std::min(min_delta_E_rel[i], ev.min_delta_E_rel[i]);
            max_delta_E_rel[i] = std::max(max_delta_E_rel[i], ev.max_delta_E_rel[i]);

            min_delta_Et_rel[i] = std::min(min_delta_Et_rel[i], ev.min_delta_Et_rel[i]);
            max_delta_Et_rel[i] = std::max(max_delta_Et_rel[i], ev.max_delta_Et_rel[i]);

            min_cluster_number[i] = std::min( min_cluster_number[i], ev.max_cluster_number[i] );
            max_cluster_number[i] = std::max( max_cluster_number[i], ev.min_cluster_number[i] );

            min_unmatched_number[i] = std::min( min_unmatched_number[i], ev.max_unmatched_number[i] );
            max_unmatched_number[i] = std::max( max_unmatched_number[i], ev.min_unmatched_number[i] );

            min_cluster_diff[i] = std::min( min_cluster_diff[i], ev.cluster_diff[i] );
            max_cluster_diff[i] = std::max( max_cluster_diff[i], ev.cluster_diff[i] );

            min_delta_eta[i] = std::min(min_delta_eta[i], ev.min_delta_eta[i]);
            max_delta_eta[i] = std::max(max_delta_eta[i], ev.max_delta_eta[i]);

            min_delta_phi[i] = std::min(min_delta_phi[i], ev.min_delta_phi[i]);
            max_delta_phi[i] = std::max(max_delta_phi[i], ev.max_delta_phi[i]);

            unmatched_min_vals[i].set_to_min(ev.unmatched_min_vals[i]);
            unmatched_max_vals[i].set_to_max(ev.unmatched_max_vals[i]);
          }
      }

    populate_plots();

  }

 private:
  void populate_plots();

};

#include "ClusterPlotterPlots.h"

#endif