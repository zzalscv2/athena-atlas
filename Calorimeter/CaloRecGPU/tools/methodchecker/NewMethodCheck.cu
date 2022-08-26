// Dear emacs, this is -*- c++ -*-
#define TTAC_CALCULATE_PHI_BY_SIMPLE_AVERAGE 1

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <unordered_map>
#include <map>
#include <set>
#include <algorithm>
#include <numeric>

#include "CaloRecGPU/Helpers.h"
#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "CaloRecGPU/StandaloneDataIO.h"

#include "../../src/TopoAutomatonClusteringGPU.cu"
#include "../../src/BasicGPUClusterInfoCalculatorImpl.cu"

#include <chrono>

using namespace CaloRecGPU;

void generic_comparison_print(const std::vector<size_t> & vect, const std::string & title, const size_t break_after)
{
  std::cout << "\n\n " << title << ":";

  size_t accum = 0, local_accum = 0;

  for (size_t i = 0; i < vect.size(); ++i)
    {
      if (i % break_after == 0)
        {
          if (i > 0)
            {
              std::cout << "(" << local_accum / double(break_after) << ")";
            }
          local_accum = 0;
          std::cout << "\n             ";
        }
      const size_t res = vect[i];
      printf("%5zu ", res);
      accum += res;
      local_accum += res;
    }
  std::cout << "(" << local_accum / double(break_after) << ")";
  std::cout << "\n (" << accum / double(vect.size()) << ")";
}

struct Results
{
  Helpers::CPU_object<CellStateArr> m_state;

  std::vector<size_t> total, assignment, terminal, inclusion;

  bool has_total = false, has_assignment = false, has_terminal = false, has_inclusion = false;

  void set_reference(const EventDataHolder & event_data)
  {
    m_state = event_data.m_cell_state_dev;
  }

  void add_comparison(const EventDataHolder & event_data)
  {
    Helpers::CPU_object<CellStateArr> temp_state = event_data.m_cell_state_dev;
    size_t total_diffs = 0, just_assignment = 0, terminal_weird = 0, inclusion_or_not = 0;
    for (int i = 0; i < NCaloCells; ++i)
      {
        const tag_type test_tag = temp_state->clusterTag[i];
        const tag_type ref_tag = m_state->clusterTag[i];
        if (test_tag != ref_tag)
          {
            ++total_diffs;
            has_total = true;
            if (Tags::is_part_of_cluster(test_tag) && Tags::is_part_of_cluster(ref_tag))
              {
                ++just_assignment;
                has_assignment = true;
              }
            else if (Tags::is_non_assigned_terminal(test_tag) && Tags::is_non_assigned_terminal(ref_tag))
              {
                ++terminal_weird;
                has_terminal = true;
              }
            else
              {
                ++inclusion_or_not;
                has_inclusion = true;
              }
          }
      }
    total.push_back(total_diffs);
    assignment.push_back(just_assignment);
    terminal.push_back(terminal_weird);
    inclusion.push_back(inclusion_or_not);
  }

  void print_comparison(const size_t break_after, const bool force_print)
  {
    std::cout << " --------- CONSISTENCY --------- \n\n";

    if (has_total || force_print)
      {
        generic_comparison_print(total, "Total", break_after);
      }

    if (has_assignment || force_print)
      {
        generic_comparison_print(assignment, "Assignment", break_after);
      }


    if (has_terminal || force_print)
      {
        generic_comparison_print(terminal, "Terminal", break_after);
      }

    if (has_inclusion || force_print)
      {
        generic_comparison_print(inclusion, "Inclusion", break_after);
      }

    if (!has_total)
      {
        std::cout << "\nAll good, you can go rest now.\n";
      }
    std::cout << std::endl;

  }

};


struct CPUComparison
{

  struct SNRArray
  {
    float snr[NCaloCells];
  };

  Helpers::CPU_object<CellStateArr> m_state;
  Helpers::CPU_object<SNRArray> m_snr_arr;

  int m_ref_clusters = 0;

  std::vector<size_t> total, seed, grow, term;

  bool has_total = false, has_seed = false, has_grow = false, has_term = false;

  struct phi_errors
  {
    double mu = 0, sigma_sqr = 0;
    double differences = 0;
    double max = std::numeric_limits<double>::lowest();
    void add (const double val)
    {
      mu += val;
      sigma_sqr += val * val;
      max = std::max(max, val);
    }
    void add_difference()
    {
      differences += 1;
    }
    void finalize(const size_t count, const size_t num_events = 1)
    {
      mu /= count;
      sigma_sqr /= count;
      sigma_sqr -= mu * mu;
      differences /= num_events;
    }
    void combine(const phi_errors & dp, const size_t count)
    {
      mu += dp.mu * count;
      sigma_sqr += (dp.sigma_sqr + dp.mu * dp.mu) * count;
      differences += dp.differences;
      max = std::max(max, dp.max);
    }

    template <class stream>
    friend stream & operator<< (stream & s, const phi_errors & pe)
    {
      s << "|" << pe.differences << " " << pe.max << " " << pe.mu + (pe.mu < 1e-3) * 1e-3 << " " << std::sqrt(pe.sigma_sqr) << "|";
      return s;
    }

  };

  std::vector<std::string> phi_comparison_labels { "Reference to CPU-Calculated",
    "Reference to GPU-Calculated",
    "CPU-Calculated to GPU-Calculated",
    "CPU to CPU No Reg.",
    "CPU to CPU KBN",
    "CPU to CPU KBN No Reg.",
    "CPU to CPU Sin/Cos",
    "CPU to CPU Sin/Cos KBN",
    "CPU KBN No Reg. to GPU",
    "Reference No Reg. to GPU",
    "Reference Sin/Cos to GPU"};

  std::vector<std::vector<phi_errors>> phi_comparisons = std::vector<std::vector<phi_errors>>(phi_comparison_labels.size());

  std::vector<size_t> valid_phis_counts;

  template <class T>
  static inline T regularize(T b, T a)
  //a. k. a. proxim in Athena code.
  {
    const T aplus = a + Helpers::Constants::pi<T>;
    const T aminus = a - Helpers::Constants::pi<T>;
    if (b > aplus)
      {
        do
          {
            b -= 2 * Helpers::Constants::pi<T>;
          }
        while (b > aplus);
      }
    else if (b < aminus)
      {
        do
          {
            b += 2 * Helpers::Constants::pi<T>;
          }
        while (b < aminus);
      }
    return b;
  }
  static double angle_fix(const double angle)
  {
    return regularize(angle, 0.);
    //fmod, no?
  }

  struct base_phi_calc
  {
    virtual void resize(const size_t) = 0;
    virtual void update_cluster(const int cluster, const double energy, const double phi, const double seed_phi) = 0;
    virtual void finalize(std::vector<double> & array) = 0;
  };


  inline static
  void calculate_phis(base_phi_calc & phi_calculator, std::vector<double> & cluster_phi_array, const CellStateArr & state,
                      const int n_clusters, const float * phi, const float * snr, const float * energy)
  {

    std::vector<int> seed_cell_id(n_clusters, -1);
    std::vector<float> seed_snr(n_clusters, 0);

    for (int i = 0; i < NCaloCells; ++i)
      {
        const tag_type tag = state.clusterTag[i];
        if (Tags::is_part_of_cluster(tag))
          {
            const int index = Tags::get_index_from_tag(tag);
            const float this_snr = std::abs(snr[i]);
            if (this_snr > seed_snr[index])
              {
                seed_cell_id[index] = i;
                seed_snr[index] = this_snr;
              }
          }
      }

    std::vector<float> seed_phi(n_clusters, 0);

    for (int i = 0; i < n_clusters; ++i)
      {
        const int id = seed_cell_id[i];
        if (id < 0)
          {
            std::cout << "ERRRR! " << i << " " << id << std::endl;
          }
        else
          {
            seed_phi[i] = phi[id];
          }
      }

    phi_calculator.resize(n_clusters);

    for (int i = 0; i < NCaloCells; ++i)
      {
        const tag_type tag = state.clusterTag[i];
        if (Tags::is_part_of_cluster(tag))
          {
            const int index = Tags::get_index_from_tag(tag);
            phi_calculator.update_cluster(index, energy[i], phi[i], seed_phi[index]);
          }
      }

    cluster_phi_array.clear();
    cluster_phi_array.resize(n_clusters, -1000.);

    phi_calculator.finalize(cluster_phi_array);

  }

  struct standard_phi_calc : public base_phi_calc
  {
    std::vector<double> phis, weights;
    void resize(const size_t n_clusters)
    {
      phis.clear();
      phis.resize(n_clusters, 0);
      weights.clear();
      weights.resize(n_clusters, 0);
    }
    void update_cluster(const int cluster, const double energy, const double phi, const double seed_phi)
    {
      const double abs_e = std::abs(energy);
      weights[cluster] += abs_e;
      phis[cluster] += abs_e * regularize(phi, seed_phi);
    }
    void finalize(std::vector<double> & array)
    {
      for (size_t i = 0; i < phis.size(); ++i)
        {
          array[i] = angle_fix(phis[i] / weights[i]);
        }
    }
  };

  struct standard_phi_calc_no_reg : public standard_phi_calc
  {
    void update_cluster(const int cluster, const double energy, const double phi, const double seed_phi)
    {
      const double abs_e = std::abs(energy);
      weights[cluster] += abs_e;
      phis[cluster] += abs_e * phi;
    }
  };

  struct KBN_base_class : public base_phi_calc
  {
    template <class T> inline static
    void KahanBabushkaNeumaier_summate(const T res, T & Sum, T & Correct)
    {
      const T t = Sum + res;
      if (std::abs(Sum) >= std::abs(res))
        {
          Correct += (Sum - t) + res;
        }
      else
        {
          Correct += (res - t) + Sum;
        }
      Sum = t;
    }
  };

  struct KBN_phi_calc : public KBN_base_class
//Kahan-Babushka-Neumaier Summation
  {
    std::vector<double> phis, weights, corrections;
    void resize(const size_t n_clusters)
    {
      phis.clear();
      phis.resize(n_clusters, 0);
      weights.clear();
      weights.resize(n_clusters, 0);
      corrections.clear();
      corrections.resize(n_clusters, 0);
    }
    void update_cluster(const int cluster, const double energy, const double phi, const double seed_phi)
    {
      const double abs_e = std::abs(energy);
      weights[cluster] += abs_e;
      KahanBabushkaNeumaier_summate(abs_e * regularize(phi, seed_phi), phis[cluster], corrections[cluster]);
    }
    void finalize(std::vector<double> & array)
    {
      for (size_t i = 0; i < phis.size(); ++i)
        {
          array[i] = angle_fix((phis[i] + corrections[i]) / weights[i]);
        }
    }
  };

  struct KBN_phi_calc_no_reg : public KBN_phi_calc
  {
    void update_cluster(const int cluster, const double energy, const double phi, const double seed_phi)
    {
      const double abs_e = std::abs(energy);
      weights[cluster] += abs_e;
      KahanBabushkaNeumaier_summate(abs_e * phi, phis[cluster], corrections[cluster]);
    }
  };

  struct sincos_phi_calc : public base_phi_calc
  {
    std::vector<double> sins, coss;
    void resize(const size_t n_clusters)
    {
      sins.clear();
      sins.resize(n_clusters, 0);
      coss.clear();
      coss.resize(n_clusters, 0);
    }
    void update_cluster(const int cluster, const double energy, const double phi, const double seed_phi)
    {
      const double abs_e = std::abs(energy);
      sins[cluster] += abs_e * sin(phi);
      coss[cluster] += abs_e * cos(phi);
    }
    void finalize(std::vector<double> & array)
    {
      for (size_t i = 0; i < sins.size(); ++i)
        {
          const double raw_phi = atan2(sins[i], coss[i]);
          if (isnan(raw_phi) || isinf(raw_phi))
            {
              array[i] = -1000.;
            }
          else
            {
              array[i] = angle_fix(raw_phi);
            }
        }
    }
  };

  struct sincos_KBN_phi_calc : public KBN_base_class
  {
    std::vector<double> sins, coss, sincorr, coscorr;
    void resize(const size_t n_clusters)
    {
      sins.clear();
      sins.resize(n_clusters, 0);
      coss.clear();
      coss.resize(n_clusters, 0);
      sincorr.clear();
      sincorr.resize(n_clusters, 0);
      coscorr.clear();
      coscorr.resize(n_clusters, 0);
    }
    void update_cluster(const int cluster, const double energy, const double phi, const double seed_phi)
    {
      const double abs_e = std::abs(energy);
      KahanBabushkaNeumaier_summate(abs_e * sin(phi), sins[cluster], sincorr[cluster]);
      KahanBabushkaNeumaier_summate(abs_e * cos(phi), coss[cluster], coscorr[cluster]);
    }
    void finalize(std::vector<double> & array)
    {
      for (size_t i = 0; i < sins.size(); ++i)
        {
          const double raw_phi = atan2(sins[i] + sincorr[i], coss[i] + coscorr[i]);
          if (isnan(raw_phi) || isinf(raw_phi))
            {
              array[i] = -1000.;
            }
          else
            {
              array[i] = angle_fix(raw_phi);
            }
        }
    }
  };


  void set_reference(const CellStateArr & state, const CellInfoArr & cell_info, const ConstantDataHolder & instance_data, const int num_clusters)
  {
    m_ref_clusters = num_clusters;

    m_state.allocate();
    *(m_state) = state;

    m_snr_arr.allocate();

    for (int i = 0; i < NCaloCells; ++i)
      {
        float snr = 0.00001f;

        const int local_gain = cell_info.gain[i];

        if (GainConversion::is_invalid_cell(local_gain))
          {
            snr = 0;
            //I mean... These won't be used anyway, so... any value goes.
          }
        else if (GainConversion::is_normal_cell(local_gain) || GainConversion::is_invalid_seed_cell(local_gain))
          {
            const float local_noise = instance_data.m_cell_noise->noise[GainConversion::recover_invalid_seed_cell_gain(local_gain)][i];
            if (finite(local_noise) && local_noise > 0.0f)
              {
                snr = std::abs(cell_info.energy[i] / local_noise);
              }
          }
        m_snr_arr->snr[i] = snr;
      }

    std::vector<double> phi_calcs;
    standard_phi_calc spc{};
    calculate_phis(spc, phi_calcs, *m_state, m_ref_clusters,
                   instance_data.m_geometry->phi, m_snr_arr->snr, cell_info.energy);

  }

  static constexpr double default_min_similarity = 0.75;
  static constexpr double default_term_weight = 0.0;
  static constexpr double default_grow_weight = 250;
  static constexpr double default_seed_weight = 10000;


  static constexpr float SNR_thresholds[3] = {0., 2., 4.};

  void find_matches(std::vector<int> & r2t_matches,
                    std::vector<int> & t2r_matches,
                    const float * SNR_array,
                    const tag_type * ref_tag_array,
                    const tag_type * test_tag_array,
                    const size_t num_ref_clusters,
                    const size_t num_test_clusters,
                    const double min_similarity = default_min_similarity,
                    const double term_weight = default_term_weight,
                    const double grow_weight = default_grow_weight,
                    const double seed_weight = default_seed_weight)
  //Expects clusters numbered from 0 to N-1...
  {
    std::vector<int> similarity_map(num_ref_clusters * num_test_clusters, 0.f);

    std::vector<double> ref_normalization(num_ref_clusters, 0.f);
    std::vector<double> test_normalization(num_test_clusters, 0.f);


    for (int i = 0; i < NCaloCells; ++i)
      {
        const tag_type ref_tag = ref_tag_array[i];
        const tag_type test_tag = test_tag_array[i];
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
        const int ref_idx = Tags::get_index_from_tag(ref_tag);
        const int test_idx = Tags::get_index_from_tag(test_tag);
        if (Tags::is_part_of_cluster(ref_tag) && Tags::is_part_of_cluster(test_tag))
          {
            similarity_map[test_idx * num_ref_clusters + ref_idx] += weight;
          }
        if (Tags::is_part_of_cluster(ref_tag))
          {
            ref_normalization[ref_idx] += weight;
          }
        if (Tags::is_part_of_cluster(test_tag))
          {
            test_normalization[test_idx] += weight;
          }
      }

    //In essence, the Gale-Shapley Algorithm

    std::vector<std::vector<int>> sorted_GPU_matches;

    sorted_GPU_matches.reserve(num_test_clusters);

    for (int testc = 0; testc < num_test_clusters; ++testc)
      {
        std::vector<int> sorter(num_ref_clusters);
        std::iota(sorter.begin(), sorter.end(), 0);

        std::sort(sorter.begin(), sorter.end(),
                  [&](const int a, const int b)
        {
          const double a_weight = similarity_map[testc * num_ref_clusters + a];
          const double b_weight = similarity_map[testc * num_ref_clusters + b];
          return a_weight > b_weight;
        }
                 );

        int wanted_size = 0;

        for (; wanted_size < sorter.size(); ++wanted_size)
          {
            const double match_weight = similarity_map[testc * num_ref_clusters + sorter[wanted_size]] / test_normalization [testc];
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


    r2t_matches.clear();
    t2r_matches.clear();

    r2t_matches.resize(num_ref_clusters, -1);
    t2r_matches.resize(num_test_clusters, -1);


    std::vector<double> matched_weights(num_ref_clusters, -1.);

    std::vector<int> skipped_matching(num_test_clusters, 0);

    for (int stop_counter = 0; stop_counter < num_test_clusters && num_iter < max_iter; ++num_iter)
      {
        stop_counter = 0;
        for (int testc = 0; testc < sorted_GPU_matches.size(); ++testc)
          {
            if (skipped_matching[testc] < sorted_GPU_matches[testc].size())
              {
                const int match_c = sorted_GPU_matches[testc][skipped_matching[testc]];
                const double match_weight = similarity_map[testc * num_ref_clusters + match_c] / ref_normalization[match_c];
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

    for (int i = 0; i < r2t_matches.size(); ++i)
      {
        const int match = r2t_matches[i];
        if (match >= 0)
          {
            t2r_matches[match] = i;
          }
      }

  }


  void add_comparison(const EventDataHolder & event_data, const ConstantDataHolder & instance_data)
  {

    cudaDeviceSynchronize();

    Helpers::CPU_object<CellStateArr> cell_state = event_data.m_cell_state_dev;

    Helpers::CPU_object<ClusterInfoArr> cluster_info = event_data.m_clusters_dev;

    std::unordered_map<int, int> tag_map;

    {

      std::vector<int> cluster_order(cluster_info->number);

      std::iota(cluster_order.begin(), cluster_order.end(), 0);

      std::sort(cluster_order.begin(), cluster_order.end(), [&](const int a, const int b)
      {
        if (cluster_info->seedCellID[a] < 0)
          {
            return false;
            //This means that clusters with no cells
            //(marked as invalid) always compare lower,
            //so they appear in the end.
          }
        else if (cluster_info->seedCellID[b] < 0)
          {
            return true;
          }
        return cluster_info->clusterEt[a] > cluster_info->clusterEt[b];
      } );

      int real_cluster_numbers = cluster_info->number;

      for (size_t i = 0; i < cluster_order.size(); ++i)
        {
          const int this_id = cluster_order[i];
          if (cluster_info->seedCellID[this_id] < 0)
            {
              tag_map[this_id] = -1;
              --real_cluster_numbers;
            }
          else
            {
              tag_map[this_id] = i;
            }
        }

      const Helpers::CPU_object<ClusterInfoArr> temp_clusters(cluster_info);

      cluster_info->number = real_cluster_numbers;

      for (int i = 0; i < temp_clusters->number; ++i)
        {
          cluster_info->clusterEnergy[i] = temp_clusters->clusterEnergy[cluster_order[i]];
          cluster_info->clusterEt[i] = temp_clusters->clusterEt[cluster_order[i]];
          cluster_info->clusterEta[i] = temp_clusters->clusterEta[cluster_order[i]];
          cluster_info->clusterPhi[i] = temp_clusters->clusterPhi[cluster_order[i]];
          cluster_info->seedCellID[i] = temp_clusters->seedCellID[cluster_order[i]];
        }

    }
    for (int i = 0; i < NCaloCells; ++i)
      {
        const tag_type this_tag = cell_state->clusterTag[i];
        if (!Tags::is_part_of_cluster(this_tag))
          {
            cell_state->clusterTag[i] = Tags::InvalidTag;
          }
        else
          {
            const int old_idx = Tags::get_index_from_tag(this_tag);
            const int new_idx = tag_map[old_idx];
            if (new_idx < 0)
              {
                cell_state->clusterTag[i] = Tags::InvalidTag;
              }
            else
              {
                cell_state->clusterTag[i] = Tags::make_seed_tag(0x7f7fffff, cluster_info->seedCellID[new_idx], new_idx);
                //To match what we do on the CPU side...
              }
          }
      }

    std::vector<int> r2t_matches, t2r_matches;

    find_matches(r2t_matches, t2r_matches, m_snr_arr->snr, m_state->clusterTag, cell_state->clusterTag, m_ref_clusters, cluster_info->number);
    /*
    for (int i = 0; i < r2t_matches.size(); ++i)
      {
    std::cout << i << " " << r2t_matches[i] << "\n";
      }
    for (int i = 0; i < t2r_matches.size(); ++i)
      {
    std::cout << i << " " << t2r_matches[i] << "\n";
      }
    std::cout << std::endl;
    */

    std::vector<bool> equal_cells(t2r_matches.size(), true);

    size_t diffs = 0, seed_count = 0, grow_count = 0, term_count = 0;

    for (int i = 0; i < NCaloCells; ++i)
      {
        const tag_type test_real_tag = cell_state->clusterTag[i];
        const tag_type ref_real_tag = m_state->clusterTag[i];

        const int test_tag = ( Tags::is_part_of_cluster(test_real_tag) ?
                               Tags::get_index_from_tag(test_real_tag) : -1 );
        const int ref_tag = ( Tags::is_part_of_cluster(ref_real_tag) ?
                              Tags::get_index_from_tag(ref_real_tag) : -1 );

        if ((test_tag >= 0 && t2r_matches[test_tag] != ref_tag) || (test_tag < 0 && ref_tag >= 0))
          {
            equal_cells[ref_tag] = false;
            ++diffs;
            has_total = true;
            const float snr = m_snr_arr->snr[i];
            if (snr > 4)
              {
                has_seed = true;
                ++seed_count;
                //std::cout << i << ": " << test_tag << " " << ref_tag << " (" << snr << ")" << std::endl;
              }
            else if (snr > 2)
              {
                has_grow = true;
                //std::cout << i << ": " << test_tag << " " << ref_tag << " (" << snr << ")" << std::endl;
                ++grow_count;
              }
            else if (snr > 0)
              {
                has_term = true;
                ++term_count;
              }
            else
              {
                std::cout << "Hmmm " << i << std::endl;
              }
          }
      }

    total.push_back(diffs);
    seed.push_back(seed_count);
    grow.push_back(grow_count);
    term.push_back(term_count);

    std::vector< std::vector< std::vector<double> > > phi_calcs;

    {

      standard_phi_calc pc_s{};
      standard_phi_calc_no_reg pc_snr{};
      KBN_phi_calc pc_kbn{};
      KBN_phi_calc_no_reg pc_kbnnr{};
      sincos_phi_calc pc_sc{};
      sincos_KBN_phi_calc pc_sckbn{};

      std::vector<base_phi_calc *> calcs{& pc_s, & pc_snr, & pc_kbn, & pc_kbnnr, & pc_sc, & pc_sckbn};

      std::vector<const CellStateArr *> tag_arrays{m_state, cell_state};
      std::vector<int> num_state_clusters{m_ref_clusters, cluster_info->number};

      for (size_t i = 0; i < tag_arrays.size(); ++i)
        {
          phi_calcs.emplace_back();
          for (auto & calc_method : calcs)
            {
              phi_calcs.back().emplace_back();
              calculate_phis(*calc_method, phi_calcs.back().back(), *tag_arrays[i], num_state_clusters[i],
                             instance_data.m_geometry->phi, m_snr_arr->snr, event_data.m_cell_info->energy);
            }
        }

    }

    std::vector<phi_errors> phi_errs(phi_comparison_labels.size());

    constexpr double max_diff = 0.25;

    int count = 0;
    for (int test_id = 0; test_id < cluster_info->number; ++test_id)
      {
        if (t2r_matches[test_id] < 0)
          {
            continue;
          }
        const double GPU_phi = cluster_info->clusterPhi[test_id];

        std::vector<int> indices{t2r_matches[test_id], test_id};

        auto check = [&](const double val, const int id1, const int id2)
        {
          if (val < -Helpers::Constants::pi<double> || val >  Helpers::Constants::pi<double>)
            {
              std::cout << id1 << " " << id2 << ": " << val << std::endl;
              return true;
            }
          return false;
        };

        bool stop = false;

        if (check(GPU_phi, -1, -1))
          {
            stop = true;
          }


        for (int i = 0; i < phi_calcs.size(); ++i)
          {
            for (int j = 0; j < phi_calcs[i].size(); ++j)
              {
                if (check(phi_calcs[i][j][indices[i]], i, j))
                  {
                    stop = true;
                  }
              }
          }

        if (stop)
          {
            continue;
          }

        struct calc_spec
        {
          int origin;
          int calc;
        };

        struct diff_spec
        {
          calc_spec a, b;
        };

        std::vector<diff_spec> diffs{ {{0, 0}, {1, 0}},   //Reference to CPU-Calculated
          {{0, 0}, {-1, -1}}, //Reference to GPU-Calculated
          {{1, 0}, {-1, -1}}, //CPU-Calculated to GPU-Calculated
          {{1, 0}, {1, 1}},   //CPU to CPU No Reg.
          {{1, 0}, {1, 2}},   //CPU to CPU KBN
          {{1, 0}, {1, 3}},   //CPU to CPU KBN No Reg.
          {{1, 0}, {1, 4}},   //CPU to CPU Sin/Cos
          {{1, 0}, {1, 5}},   //CPU to CPU Sin/Cos KBN
          {{1, 3}, {-1, -1}}, //CPU KBN No Reg. to GPU
          {{0, 1}, {-1, -1}}, //Reference No Reg. to GPU
          {{0, 4}, {-1, -1}}  //Reference Sin/Cos to GPU
        };

        auto getty = [&](const calc_spec & cs)
        {
          if (cs.origin < 0 || cs.calc < 0)
            {
              return GPU_phi;
            }
          else
            {
              return phi_calcs[cs.origin][cs.calc][indices[cs.origin]];
            }
        };

        auto setty = [&](phi_errors & pe, const double delta)
        {
          pe.add(delta);
          if (delta > max_diff)
            {
              pe.add_difference();
            }
        };

        for (int i = 0; i < diffs.size(); ++i)
          {
            const double one = getty(diffs[i].a);
            const double two = getty(diffs[i].b);

            const double dif = Helpers::Constants::pi<double> - std::abs(std::fmod(std::abs(one - two), 2 * Helpers::Constants::pi<double>) - Helpers::Constants::pi<double>);

            setty(phi_errs[i], dif);
          }
        ++count;
      }

    valid_phis_counts.push_back(count);

    for (int i = 0; i < phi_errs.size(); ++i)
      {
        phi_errs[i].finalize(count);
        phi_comparisons[i].push_back(phi_errs[i]);
      }


  }


  void print_phi(const std::vector<phi_errors> & vect, const std::vector<size_t> & counts, const std::string & title, const size_t break_after)
  {
    std::cout << "\n\n " << title << ":";

    phi_errors accum{0, 0, 0}, local_accum{0, 0, 0};
    size_t counter = 0, local_counter = 0;

    for (size_t i = 0; i < vect.size(); ++i)
      {
        if (i % break_after == 0)
          {
            if (i > 0)
              {
                local_accum.finalize(local_counter, break_after);
                std::cout << "("  << local_accum << ")";
              }
            local_accum = phi_errors {0, 0, 0};
            local_counter = 0;
            std::cout << "\n             ";
          }
        const phi_errors res = vect[i];
        const size_t num = counts[i];
        printf("|%7.3f %7.3f %7.3f %7.3f| ", res.differences, res.max, res.mu + (res.mu < 1e-3) * 1e-3, std::sqrt(res.sigma_sqr));
        local_accum.combine(res, num);
        local_counter += num;
        accum.combine(res, num);
        counter += num;
      }
    local_accum.finalize(local_counter, break_after);
    std::cout << "(" << local_accum << ")";
    accum.finalize(counter, vect.size());
    std::cout << "\n (" << accum << ")";
  }


  void print_comparison(const size_t break_after, const bool force_print)
  {
    std::cout << " --------- CPU COMPARISON --------- \n\n";

    if (has_total || force_print)
      {
        generic_comparison_print(total, "Total", break_after);
      }

    if (has_seed || force_print)
      {
        generic_comparison_print(seed, "Seed", break_after);
      }


    if (has_grow || force_print)
      {
        generic_comparison_print(grow, "Grow", break_after);
      }

    if (has_term || force_print)
      {
        generic_comparison_print(term, "Terminal", break_after);
      }

    if (!has_total)
      {
        std::cout << "\nAll good, you can go rest now.\n";
      }

    std::cout << "\n\n --------- PHI COMPARISON --------- \n\n";

    for (size_t i = 0; i < phi_comparison_labels.size(); ++i)
      {
        print_phi(phi_comparisons[i], valid_phis_counts, phi_comparison_labels[i], break_after);
      }

    std::cout << std::endl;

  }

};

void setup_cuda_device()
{
  int devID = 0;
  cudaDeviceProp props;

  /* maybe we want something else here */
  cudaSetDevice(0);

  cudaGetDeviceProperties(&props, devID);
  std::cout << "[CUDA] Device " << devID << " " << props.name <<  " with compute capability " << props.major << "." << props.minor << std::endl;
}

int main(int argc, char ** argv)
{
  if (argc < 3)
    {
      std::cout << "Expected arguments: <program> <num reps> <max events> <geometry and noise folder> <events folder>" << std::endl;
      return 0;
    }

  setup_cuda_device();

  const size_t num_reps = std::strtoull(argv[1], nullptr, 10);

  const int max_events = std::atoi(argv[2]);
  
  const auto constants_folder = StandaloneDataIO::load_folder(argv[3], 0, false, false, true, true);

  const auto loaded_folder = StandaloneDataIO::load_folder(argv[4], max_events, true, true, false, false);

  ConstantDataHolder fixed_data;

  fixed_data.m_geometry = constants_folder.geometry.begin()->second;
  fixed_data.m_cell_noise = constants_folder.noise.begin()->second;

  fixed_data.sendToGPU(false);

  TACTemporariesHolder temporary_holder;

  temporary_holder.allocate();

  TACOptionsHolder options_holder;

  options_holder.allocate();

  *(options_holder.m_options) = TopoAutomatonOptions { 4.0f, 2.0f, 0.0f, true, true, true, false, 0x7FFFFFFF};

  options_holder.sendToGPU();

  BasicGPUClusterInfoCalculatorTemporariesHolder other_temporary_holder;
  
  other_temporary_holder.allocate();


  Helpers::CPU_object<CellStateArr> temp_cells;
  Helpers::CPU_object<TopoAutomatonTemporaries> temp_temp;

  temp_cells.allocate();
  temp_temp.allocate();

  EventDataHolder event_data;
  event_data.allocate();

  std::vector<size_t> times;

  Results results;
  CPUComparison comparison;


  for (const auto & it : loaded_folder.cell_info)
    {
      event_data.m_cell_info = (*it.second);

      event_data.sendToGPU();

      //CUDA_ERRCHECK(cudaDeviceSynchronize());

      //std::cout << "Sent data." << std::endl;

      signalToNoise(event_data, temporary_holder, fixed_data, options_holder, true);

      //CUDA_ERRCHECK(cudaDeviceSynchronize());

      cellPairs(event_data, temporary_holder, fixed_data, options_holder, true);

      //CUDA_ERRCHECK(cudaDeviceSynchronize());

      temp_cells = event_data.m_cell_state_dev;
      temp_temp = temporary_holder.m_temporaries_dev;
      

      for (size_t rep = 0; rep <= num_reps; ++rep)
        {

          //CUDA_ERRCHECK(cudaDeviceSynchronize());

          if (rep != 0)
            {
              event_data.m_cell_state_dev = temp_cells;
              temporary_holder.m_temporaries_dev = temp_temp;
            }

          //CUDA_ERRCHECK(cudaDeviceSynchronize());


          //std::cout << "So far, so good... " << rep << std::endl;

          auto start = std::chrono::steady_clock::now();
          
          clusterGrowing(event_data, temporary_holder, fixed_data, options_holder, true);
          
          auto end = std::chrono::steady_clock::now();
          
          times.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());

          //CUDA_ERRCHECK(cudaDeviceSynchronize());
          if (rep == 0)
            {
              updateSeedCellProperties(event_data, other_temporary_holder, fixed_data, true);
              calculateClusterProperties(event_data, other_temporary_holder, fixed_data, true);
              
              results.set_reference(event_data);
              comparison.set_reference(
              *(loaded_folder.cell_state.at(it.first)),
              (*it.second),
              fixed_data,
              loaded_folder.clusters.at(it.first)->number
              );
              comparison.add_comparison(event_data, fixed_data);
            }
          else
            {
              results.add_comparison(event_data);
            }
        }
    }
    
  generic_comparison_print(times, "Execution Times", num_reps);
  std::cout << "\n";
  results.print_comparison(num_reps, false);
  comparison.print_comparison(loaded_folder.cell_info.size(), true);

  //CUDA_ERRCHECK(cudaDeviceSynchronize());

  return 0;
}