// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
//

#ifndef CALORECGPU_TOOLS_TIMEPLOTTER_H
#define CALORECGPU_TOOLS_TIMEPLOTTER_H

#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <string>
#include <cstring>
#include <fstream>
#include "PlotterAuxDefines.h"


#include <vector>
#include <string>
#include <set>
#include <limits>
#include <utility>
#include <map>
#include <type_traits>

#include <boost/filesystem.hpp>

using namespace std::literals::string_literals;
//Not too good.
//But we kinda need that ""s thing for ease of writing something...

#include "CxxUtils/checker_macros.h"

#ifdef ATLAS_CHECK_THREAD_SAFETY

  ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#endif


namespace PlaceholderClassesForColumnProperties
{
  struct ColMaxT {};
  inline static constexpr ColMaxT col_max{};
  struct ColMinT {};
  inline static constexpr  ColMinT col_min{};
  struct ColAvgT {};
  inline static constexpr ColAvgT col_avg{};
  struct ColStdDevT {};
  inline static constexpr ColStdDevT col_stddev{};
}

struct TimeTable
{
  std::vector<std::string> column_text;
  std::unordered_map<std::string, size_t> column_map;
  std::vector<std::vector<double>> columns;

  std::vector<double> maxs, mins, averages, std_devs;

  size_t num_events() const
  {
    if (columns.size() == 0)
      {
        return 0;
      }
    return columns.front().size();
  }

  void load_from_file(const boost::filesystem::path & filename)
  {
    std::ifstream is(filename.native());
    if (!is.is_open())
      {
        std::cout << "ERROR: Could not open '" << filename << "' to load times!" << std::endl;
        return;
      }
    else
      {
        std::cout << "INFO: Loading times from '" << filename << "'." << std::endl;
      }

    column_text.clear();
    column_map.clear();
    columns.clear();

    while (is.good() && is.peek() != '\n')
      {
        std::string temp;
        is >> temp;
        //This reads space-separated columns,
        //which is exactly what we want.

        if (temp == "Event_Number")
          {
            continue;
          }

        for (auto & x : temp)
          //Replace underscores by spaces to get the intended names...
          {
            if (x == '_')
              {
                x = ' ';
              }
          }

        const auto pos = temp.rfind("HybridClusterProcessor.");

        if (pos != std::string::npos)
          {
            temp = temp.substr(pos + 23);
            //Get rid of the prefix for the global...
          }

        column_map[temp] = column_text.size();
        column_text.push_back(temp);
        if (is.good() && is.peek() == ' ')
          {
            is.get();
          }
        //Exclude possible spaces before the line break
      }

    columns.resize(column_text.size());

    is.get();

    while (is.get() != '\n')
      {
        continue;
      }
    //Exclude the first event: setup-time delays and so on.

    while (is.good())
      {
        int count = 0;
        while (is.good() && is.peek() != '\n')
          {
            double temp = 0.;
            is >> temp;
            if (is.fail())
              {
                break;
              }
            if (count > 0)
              {
                columns[count - 1].push_back(temp);
              }
            //The first element is the event number...
            ++count;
            if (is.good() && is.peek() == ' ')
              {
                is.get();
              }
            //Exclude possible spaces before the line break
          }
        is.get();
      }
    is.close();
  }

  TimeTable(const boost::filesystem::path & filename)
  {
    load_from_file(filename);
  }

 private:

  void check_column_outliers(std::set<int> & to_remove, const size_t col_idx, const double factor = 20., const int ignore_largest = 30) const
  {
    const std::vector<double> & col = columns[col_idx];
    if (col.size() < 1)
      {
        std::cout << "ERROR: Trying to remove outliers from empty time column (" << col_idx << ": " << column_text[col_idx] << ")!" << std::endl;
        return;
      }

    std::vector<double> copy_col = col;

    std::sort(copy_col.begin(), copy_col.end());

    const int ignore_limit = std::min(ignore_largest, int(copy_col.size() / 4));


    double mu = copy_col[0];

    for (size_t i = 1; i < copy_col.size() - ignore_limit; ++i)
      {
        mu += copy_col[i];
      }

    mu /= std::max(size_t(copy_col.size() - ignore_limit), size_t(1));

    const double upper_limit = (mu - copy_col[0]) * factor + mu;

    for (size_t i = 0; i < copy_col.size(); ++i)
      {
        if (col[i] > upper_limit)
          {
            to_remove.insert(i);
          }
      }
  }

  void reorder_column_to_exclude_outliers(const std::set<int> & to_remove, std::vector<double> & col)
  {
    int last_valid = col.size() - 1;
    const auto stop = to_remove.crend();
    for (auto it = to_remove.crbegin(); it != stop; ++it)
      {
        col[*it] = col[last_valid];
        last_valid--;
      }
    col.resize(col.size() - to_remove.size());
  }

 public:

  void check_outliers(std::set<int> & to_remove, const double factor = 20., const int ignore_largest = 30) const
  {
    for (size_t i = 0; i < columns.size(); ++i)
      {
        check_column_outliers(to_remove, i, factor, ignore_largest);
      }
  }

  void exclude_outliers(const std::set<int> & to_remove)
  {
    for (auto & col : columns)
      {
        reorder_column_to_exclude_outliers(to_remove, col);
      }
  }

  void calculate_stats()
  {
    for (const auto & col : columns)
      {
        double sum = 0, sum_sqr = 0, max, min;
        set_to_highest(min);
        set_to_lowest(max);
        for (const auto & t : col)
          {
            sum += t;
            sum_sqr += t * t;
            min = std::min(min, t);
            max = std::max(max, t);
          }
        const double avg = sum / col.size();
        const double std_dev = std::sqrt(sum_sqr / col.size() - avg * avg);
        maxs.push_back(max);
        mins.push_back(min);
        averages.push_back(avg);
        std_devs.push_back(std_dev);
      }
  }


  template <class ... Args>
  auto get(const std::string & column_name, Args && ... args) const
  {
    return this->get(column_map.at(column_name), std::forward<Args>(args)...);
  }

  template <class ... Args>
  auto get(const std::string & column_name, Args && ... args)
  {
    return this->get(column_map.at(column_name), std::forward<Args>(args)...);
  }

  const std::vector<double> & get(const size_t column_index) const
  {
    return columns[column_index];
  }

  std::vector<double> & get(const size_t column_index)
  {
    return columns[column_index];
  }

  double get(const size_t column_index, const size_t idx) const
  {
    return columns[column_index][idx];
  }

  double & get(const size_t column_index, const size_t idx)
  {
    return columns[column_index][idx];
  }

  template <class T>
  double get(const size_t column_index, const T & ) const
  {
    using namespace PlaceholderClassesForColumnProperties;

    if constexpr (std::is_same_v<T, ColMaxT>)
      {
        return maxs[column_index];
      }
    else if constexpr (std::is_same_v<T, ColMinT>)
      {
        return mins[column_index];
      }
    else if constexpr (std::is_same_v<T, ColAvgT>)
      {
        return averages[column_index];
      }
    else if constexpr (std::is_same_v<T, ColStdDevT>)
      {
        return std_devs[column_index];
      }
    else
      {
        return -1;
      }
  }

};

struct TimeHolder
{
  size_t num_threads;
  std::map<std::string, TimeTable> times;

  void ready_for_plotting(const double factor = 20., const int ignore_largest = 30)
  {
    std::set<int> to_exclude;
    for (const auto & it : times)
      {
        it.second.check_outliers(to_exclude, factor, ignore_largest);
      }
    for (auto & it : times)
      {
        it.second.exclude_outliers(to_exclude);
        it.second.calculate_stats();
      }
  }

  TimeHolder(const boost::filesystem::path & foldername, const size_t n_thrs): num_threads(n_thrs)
  {
    for (auto const & dir_entry : boost::filesystem::directory_iterator(foldername))
      {
        if (!boost::filesystem::is_regular_file(dir_entry))
          {
            continue;
          }
        const boost::filesystem::path file_path = dir_entry;
        if (file_path.extension() != ".txt")
          {
            continue;
          }
        const std::string file_string = file_path.stem().native();
        const auto pos = file_string.rfind("Times");
        if (pos == std::string::npos)
          {
            continue;
          }
        times.emplace(file_string.substr(0, pos), file_path);
      }
    ready_for_plotting();
  }


  size_t num_events() const
  {
    if (times.size() == 0)
      {
        return 0;
      }
    return times.begin()->second.num_events();
  }

  template <class ... Args>
  auto get(const std::string & table_name, Args && ... args) const
  {
    return times.at(table_name).get(std::forward<Args>(args)...);
  }

  template <class ... Args>
  auto get(const std::string & table_name, Args && ... args)
  {
    return times.at(table_name).get(std::forward<Args>(args)...);
  }

  const TimeTable & get(const std::string & table_name) const
  {
    return times.at(table_name);
  }

  TimeTable & get(const std::string & table_name)
  {
    return times.at(table_name);
  }


  bool has_growing_data() const
  {
    return times.count("TopoAutomatonClustering") > 0;
  }

  bool has_splitting_data() const
  {
    return times.count("ClusterSplitter") > 0;
  }

};

struct TimePlotter : public BasePlotter
{
  std::vector<TimeHolder> runs;

  bool has_growing_data() const
  {
    return runs.back().has_growing_data();
  }

  bool has_splitting_data() const
  {
    return runs.back().has_splitting_data();
  }

  TimePlotter(const boost::filesystem::path & pth)
  {
    for (auto const & dir_entry : boost::filesystem::directory_iterator(pth))
      {
        if (!boost::filesystem::is_directory(dir_entry))
          {
            continue;
          }
        const boost::filesystem::path inner_dir = dir_entry;
        const std::string dir_string = inner_dir.native();
        const auto pos = dir_string.rfind("times_");
        if (pos == std::string::npos)
          {
            continue;
          }
        const size_t num_threads = std::strtoull(dir_string.substr(pos + 6).c_str(), nullptr, 10);
        if (num_threads == 0)
          {
            std::cout << "WARNING: Skipping '" << inner_dir << "' for invalid thread number (" << dir_string.substr(pos + 6) << ")" << std::endl;
            continue;
          }
        runs.emplace_back(inner_dir, num_threads);
      }
    std::sort(runs.begin(), runs.end(), [](const TimeHolder & a, const TimeHolder & b)
    {
      return a.num_threads < b.num_threads;
    });
    if (has_growing_data())
      {
        populate_growing_plots();
      }
    if (has_splitting_data())
      {
        populate_splitting_plots();
      }
    if (has_growing_data() && has_splitting_data())
      {
        populate_overall_plots();
      }
  }

  size_t max_num_threads() const
  {
    return runs.back().num_threads;
  }

  virtual size_t num() const
  {
    if (runs.size() == 0)
      {
        return 0;
      }
    return runs.back().num_events();
  }

  template <class ... Args>
  auto get(const size_t num_threads, Args && ... args) const
  {
    return runs[num_threads].get(std::forward<Args>(args)...);
  }

  const TimeHolder & get(const size_t num_threads) const
  {
    return runs[num_threads];
  }

  template <class ... Args>
  auto get(const size_t num_threads, Args && ... args)
  {
    return runs[num_threads].get(std::forward<Args>(args)...);
  }

  TimeHolder & get(const size_t num_threads)
  {
    return runs[num_threads];
  }

  ~TimePlotter()
  {
  }

  TimePlotter(const TimePlotter &) = delete;
  TimePlotter(TimePlotter &&) = default;

  TimePlotter & operator= (const TimePlotter &) = delete;
  TimePlotter & operator= (TimePlotter &&) = default;

 private:



  void populate_generic_plots(const std::string & suffix,
                              const std::vector <std::pair<std::string, std::string>> & ref_ts,
                              const std::vector <std::pair<std::string, std::string>> & test_ts,
                              const std::vector <std::pair<std::string, std::string>> & extra_ts);

  public:

  std::vector <std::pair<std::string, std::string>>
                                     growing_default = {std::make_pair("Global"s, "DefaultGrowing"s)},
                                     growing_test = {std::make_pair("TopoAutomatonClustering"s, "Total"s), std::make_pair("PropCalcPostGrowing"s, "Total"s)},
                                     growing_extra = {std::make_pair("EventDataExporter"s, "Total"s), std::make_pair("ClusterImporter"s, "Total"s)},
                                     splitting_default = {std::make_pair("Global"s, "DefaultSplitting"s)},
                                     splitting_test = {std::make_pair("ClusterSplitter"s, "Total"s), std::make_pair("PropCalcPostSplitting"s, "Total"s)},
                                     splitting_extra = {std::make_pair("EventDataExporter"s, "Total"s), std::make_pair("ClusterImporter"s, "Total"s)},
                                     growsplit_default = {std::make_pair("Global"s, "DefaultGrowing"s), std::make_pair("Global"s, "DefaultSplitting"s)},
                                     growsplit_test = {std::make_pair("TopoAutomatonClustering"s, "Total"s), std::make_pair("ClusterSplitter"s, "Total"s), std::make_pair("PropCalcPostSplitting"s, "Total"s)},
                                     growsplit_extra = {std::make_pair("EventDataExporter"s, "Total"s), std::make_pair("ClusterImporter"s, "Total"s)};


  void populate_growing_plots()
  {
    populate_generic_plots("grow", growing_default, growing_test, growing_extra);
  }
  void populate_splitting_plots()
  {
    populate_generic_plots("split", splitting_default, splitting_test, splitting_extra);
  }
  void populate_overall_plots()
  {
    populate_generic_plots("growsplit", growsplit_default, growsplit_test, growsplit_extra);
  }


  template <class T>
  static double sum_all(const TimeHolder & holder, const std::vector < std::pair<std::string, std::string> > & vs, const T & id)
  {
    double ret = 0;
    for (const auto & pr : vs)
      {
        ret += holder.get(pr.first, pr.second, id);
      }
    return ret;
  }

  template <class F>
  static void calculate_stats (const size_t num_events, double & avg, double & std_dev, const F & func)
  {
    double sum = 0, sum_sqr = 0;
    for (size_t i = 0; i < num_events; ++i)
      {
        const double res = func(i);
        sum += res;
        sum_sqr += res * res;
      }
    avg = sum / num_events;
    std_dev = std::sqrt(sum_sqr / num_events - avg * avg);
  }


};

#ifndef RETFUNC
  #define RETFUNC(...) [&]([[maybe_unused]] const int i){ return double(__VA_ARGS__); }
#endif

void TimePlotter::populate_generic_plots(const std::string & suffix,
                                         const std::vector <std::pair<std::string, std::string>> & ref_ts,
                                         const std::vector <std::pair<std::string, std::string>> & test_ts,
                                         const std::vector <std::pair<std::string, std::string>> & extra_ts)
{
  std::cout << "Started populating '" << suffix << "' time plots." << std::endl;

  using namespace PlaceholderClassesForColumnProperties;

  for (const TimeHolder & holder : runs)
    {
      add_plot<joined_plotter_time>
      (std::string("time_dist_") + suffix + "_" + std::to_string(holder.num_threads),
       "Event Processing Time", "#font[52]{t} [#mus]", (normalize ? "Fraction of Events" : "Events"),
       add_plot<H1D_plotter_time>
       (std::string("time") + "dist_reference_" + std::to_string(holder.num_threads),
        RETFUNC(sum_all(holder, ref_ts, col_min)),
        RETFUNC(sum_all(holder, ref_ts, col_max)),
        false, 0, "CPU Event Processing Time", "#font[52]{t} [#mus]", (normalize ? "Fraction of Events" : "Events"),
        [&](hist_group_1D * group)
      {

        for (size_t i = 0; i < holder.num_events(); ++i)
          {
            group->global->Fill(sum_all(holder, ref_ts, i));
          }
      },
      StyleKinds::ref),
      ref_name,
      add_plot<H1D_plotter_time>
      (std::string("time_dist_test_") + suffix + "_" + std::to_string(holder.num_threads),
       RETFUNC(sum_all(holder, test_ts, col_min) + sum_all(holder, extra_ts, col_min)),
       RETFUNC(sum_all(holder, test_ts, col_max) + sum_all(holder, extra_ts, col_max)),
       false, 0, "GPU Event Processing Time", "#font[52]{t} [#mus]", (normalize ? "Fraction of Events" : "Events"),
       [&](hist_group_1D * group)
      {
        for (size_t i = 0; i < holder.num_events(); ++i)
          {
            group->global->Fill(sum_all(holder, test_ts, i) + sum_all(holder, extra_ts, i));
          }
      },
      StyleKinds::test),
      test_name
      );


      add_plot<H1D_plotter_time>
      (std::string("time_frac_algorithm_") + suffix + "_" + std::to_string(holder.num_threads),
       RETFUNC(0),
       RETFUNC(1),
       false, 0, "Fraction of the GPU Processing Time Due to the Algorithm",
       "#font[52]{t}^{(Algorithm)}/#font[52]{t}^{(" + test_name + ")}", (normalize ? "Fraction of Events" : "Events"),
       [&](hist_group_1D * group)
      {
        for (size_t i = 0; i < holder.num_events(); ++i)
          {
            group->global->Fill(sum_all(holder, test_ts, i) / (sum_all(holder, test_ts, i) + sum_all(holder, extra_ts, i)));
          }
      },
      StyleKinds::test);

      add_plot<H1D_plotter_time>
      (std::string("time_frac_convs_") + suffix + "_" + std::to_string(holder.num_threads),
       RETFUNC(0),
       RETFUNC(1),
       false, 0, "Fraction of the GPU Processing Time Due to Data Conversions",
       "#font[52]{t}^{(Conversions)}/#font[52]{t}^{(" + test_name + ")}", (normalize ? "Fraction of Events" : "Events"),
       [&](hist_group_1D * group)
      {
        for (size_t i = 0; i < holder.num_events(); ++i)
          {
            const double conversion_t = holder.get("EventDataExporter", "Total", i) +
                                        holder.get("ClusterImporter", "Total", i) -
                                        holder.get("EventDataExporter", "Transfer to GPU", i) -
                                        holder.get("ClusterImporter", "Transfer from GPU", i);
            //Safe to hardcode this since it will be the same for all (grow, split & both)

            group->global->Fill(conversion_t / (sum_all(holder, test_ts, i) + sum_all(holder, extra_ts, i)));
          }
      },
      StyleKinds::test);


      add_plot<H1D_plotter_time>
      (std::string("time_frac_transfers_") + suffix + "_" + std::to_string(holder.num_threads),
       RETFUNC(0),
       RETFUNC(1),
       false, 0, "Fraction of the GPU Processing Time Due to Data Transfers",
       "#font[52]{t}^{(Transfers)}/#font[52]{t}^{(" + test_name + ")}", (normalize ? "Fraction of Events" : "Events"),
       [&](hist_group_1D * group)
      {
        for (size_t i = 0; i < holder.num_events(); ++i)
          {
            const double transfer_t = holder.get("EventDataExporter", "Transfer to GPU", i) +
                                      holder.get("ClusterImporter", "Transfer from GPU", i);
            //Safe to hardcode this since it will be the same for all (grow, split & both)

            group->global->Fill(transfer_t / (sum_all(holder, test_ts, i) + sum_all(holder, extra_ts, i)));
          }
      },
      StyleKinds::test);

      add_plot<H1D_plotter_time>
      (std::string("time_ratio_") + suffix + "_" + std::to_string(holder.num_threads),
       RETFUNC(0),
       RETFUNC(1),
       false, 0, "Ratio Between GPU and CPU Event Processing Time",
       "#font[52]{t}^{(" + test_name + ")}/#font[52]{t}^{(" + ref_name + ")}", (normalize ? "Fraction of Events" : "Events"),
       [&](hist_group_1D * group)
      {
        for (size_t i = 0; i < holder.num_events(); ++i)
          {
            const double gpu_t = sum_all(holder, test_ts, i) + sum_all(holder, extra_ts, i);
            const double cpu_t = sum_all(holder, ref_ts, i);

            group->global->Fill(gpu_t / cpu_t);
          }
      },
      StyleKinds::joined);

      add_plot<H1D_plotter_time>
      (std::string("time_speedup_") + suffix + "_" + std::to_string(holder.num_threads),
       RETFUNC(0),
       RETFUNC(50),
       false, 0, "GPU Speed-Up of Event Processing Time in Relation to the CPU",
       "Speed-Up #(){#font[52]{t}^{(" + ref_name + ")}/#font[52]{t}^{(" + test_name + ")}}", (normalize ? "Fraction of Events" : "Events"),
       [&](hist_group_1D * group)
      {
        for (size_t i = 0; i < holder.num_events(); ++i)
          {
            const double gpu_t = sum_all(holder, test_ts, i) + sum_all(holder, extra_ts, i);
            const double cpu_t = sum_all(holder, ref_ts, i);

            group->global->Fill(cpu_t / gpu_t);
          }
      },
      StyleKinds::joined);


      add_plot<H1D_plotter_time>
      (std::string("time_speedup_fake_") + suffix + "_" + std::to_string(holder.num_threads),
       RETFUNC(0),
       RETFUNC(75),
       false, 0, "Algorithm Speed-Up of Event Processing Time in Relation to the CPU",
       "Speed-Up #(){#font[52]{t}^{(" + ref_name + ")}/#font[52]{t}^{(Algorithm)}}", (normalize ? "Fraction of Events" : "Events"),
       [&](hist_group_1D * group)
      {
        for (size_t i = 0; i < holder.num_events(); ++i)
          {
            const double gpu_t = sum_all(holder, test_ts, i);
            const double cpu_t = sum_all(holder, ref_ts, i);

            group->global->Fill(cpu_t / gpu_t);
          }
      },
      StyleKinds::joined);

    }

  //-----------------------------------------------------------//
  //                  Start of global plots                    //
  //-----------------------------------------------------------//


  add_plot<joined_plotter_graph>
  (std::string("time_global_exec_") + suffix, "Execution Time", "# CPU Threads", "Execution Time [#mu#font[52]{s}]",
   add_plot<graph_plotter>
   (std::string("time_global_exec_ref_") + suffix,
    RETFUNC(0),
    RETFUNC(0),
    false, 0, "Execution Time (CPU)",
    "# CPU Threads", "Execution Time [#mu#font[52]{s}]",
    [&](graph_group_1D * group)
  {
    for (auto & holder : runs)
      {
        double avg = 0, std_dev = 0;

        calculate_stats(holder.num_events(), avg, std_dev,
                        [&](const size_t i)
        {
          return sum_all(holder, ref_ts, i);
        });

        group->global->AddPoint(holder.num_threads, avg);
        group->global->SetPointError(group->global->GetN() - 1, 0.,  std_dev);
      }
  },
  StyleKinds::ref),
  ref_name,
  add_plot<graph_plotter>
  (std::string("time_global_exec_test_") + suffix,
   RETFUNC(0),
   RETFUNC(0),
   false, 0, "Execution Time (GPU)",
   "# CPU Threads", "Execution Time [#mu#font[52]{s}]",
   [&](graph_group_1D * group)
  {
    for (auto & holder : runs)
      {
        double avg = 0, std_dev = 0;

        calculate_stats(holder.num_events(), avg, std_dev,
                        [&](const size_t i)
        {
          return sum_all(holder, test_ts, i) + sum_all(holder, extra_ts, i);
        });

        group->global->AddPoint(holder.num_threads, avg);
        group->global->SetPointError(group->global->GetN() - 1, 0.,  std_dev);
      }
  },
  StyleKinds::test),
  test_name);

  add_plot<joined_plotter_graph>
  (std::string("time_global_throughput_") + suffix, "Throughput", "# CPU Threads", "Throughput [event/#font[52]{s}]",
   add_plot<graph_plotter>
   (std::string("time_global_throughput_ref_") + suffix,
    RETFUNC(0),
    RETFUNC(0),
    false, 0, "Throughput (CPU)",
    "# CPU Threads", "Throughput [event/#font[52]{s}]",
    [&](graph_group_1D * group)
  {
    for (auto & holder : runs)
      {
        double avg = 0, std_dev = 0;

        calculate_stats(holder.num_events(), avg, std_dev,
                        [&](const size_t i)
        {
          return 1e6 * double(holder.num_threads) / sum_all(holder, ref_ts, i);
        });

        group->global->AddPoint(holder.num_threads, avg);
        group->global->SetPointError(group->global->GetN() - 1, 0.,  std_dev);
      }
  },
  StyleKinds::ref),
  ref_name,
  add_plot<graph_plotter>
  (std::string("time_global_throughput_test_") + suffix,
   RETFUNC(0),
   RETFUNC(0),
   false, 0, "Throughput (GPU)",
   "# CPU Threads", "Throughput [event/#font[52]{s}]",
   [&](graph_group_1D * group)
  {
    for (auto & holder : runs)
      {
        double avg = 0, std_dev = 0;

        calculate_stats(holder.num_events(), avg, std_dev,
                        [&](const size_t i)
        {
          const double gpu_t = sum_all(holder, test_ts, i) + sum_all(holder, extra_ts, i);
          return 1e6 * double(holder.num_threads) / gpu_t;
        });

        group->global->AddPoint(holder.num_threads, avg);
        group->global->SetPointError(group->global->GetN() - 1, 0.,  std_dev);
      }
  },
  StyleKinds::test),
  test_name);


  add_plot<joined_plotter_graph>
  (std::string("time_global_scaling_") + suffix, "Multithreading Scaling", "# CPU Threads",
   "Scaling #(){Throughput/N_{threads}/Throughput#(){1 thread}}",
   add_plot<graph_plotter>
   (std::string("time_global_scaling_ref_") + suffix,
    RETFUNC(0),
    RETFUNC(0),
    false, 0, "Multithreading Scaling (CPU)",
    "# CPU Threads", "Scaling #(){Throughput/{N}_{threads}/Throughput#(){1 thread}}",
    [&](graph_group_1D * group)
  {
    for (auto & holder : runs)
      {
        double avg = 0, std_dev = 0;

        calculate_stats(holder.num_events(), avg, std_dev,
                        [&](const size_t i)
        {
          return sum_all(get(0), ref_ts, i) / sum_all(holder, ref_ts, i);
        });

        group->global->AddPoint(holder.num_threads, avg);
        group->global->SetPointError(group->global->GetN() - 1, 0.,  std_dev);

      }
  },
  StyleKinds::ref),
  ref_name,
  add_plot<graph_plotter>
  (std::string("time_global_scaling_test_") + suffix,
   RETFUNC(0),
   RETFUNC(0),
   false, 0, "Multithreading Scaling (GPU)",
   "# CPU Threads", "Scaling #(){Throughput/{N}_{threads}/Throughput#(){1 thread}}",
   [&](graph_group_1D * group)
  {
    for (auto & holder : runs)
      {
        double avg = 0, std_dev = 0;

        calculate_stats(holder.num_events(), avg, std_dev,
                        [&](const size_t i)
        {
          const double gpu_ref_t = sum_all(get(0), test_ts, i) + sum_all(get(0), extra_ts, i);

          const double gpu_t = sum_all(holder, test_ts, i) + sum_all(holder, extra_ts, i);

          return gpu_ref_t / gpu_t;
        });

        group->global->AddPoint(holder.num_threads, avg);
        group->global->SetPointError(group->global->GetN() - 1, 0.,  std_dev);

      }
  },
  StyleKinds::test),
  test_name);

  add_plot<graph_plotter>
  (std::string("time_global_speedup_") + suffix,
   RETFUNC(0),
   RETFUNC(0),
   false, 0, "GPU Speed-Up of Event Processing Time in Relation to the CPU",
   "# CPU Threads", "Speed-Up #(){#font[52]{t}^{(" + ref_name + ")}/#font[52]{t}^{(" + test_name + ")}}",
   [&](graph_group_1D * group)
  {
    for (auto & holder : runs)
      {
        double avg = 0, std_dev = 0;

        calculate_stats(holder.num_events(), avg, std_dev,
                        [&](const size_t i)
        {
          const double gpu_t = sum_all(holder, test_ts, i) + sum_all(holder, extra_ts, i);
          return sum_all(holder, ref_ts, i) / gpu_t;
        });

        group->global->AddPoint(holder.num_threads, avg);
        group->global->SetPointError(group->global->GetN() - 1, 0.,  std_dev);

      }
  },
  StyleKinds::joined);

  add_plot<graph_plotter>
  (std::string("time_global_speedup_fake_") + suffix,
   RETFUNC(0),
   RETFUNC(0),
   false, 0, "Algorithm Speed-Up of Event Processing Time in Relation to the CPU",
   "# CPU Threads", "Speed-Up #(){#font[52]{t}^{(" + ref_name + ")}/#font[52]{t}^{(Algorithm)}}",
   [&](graph_group_1D * group)
  {
    for (auto & holder : runs)
      {
        double avg = 0, std_dev = 0;

        calculate_stats(holder.num_events(), avg, std_dev,
                        [&](const size_t i)
        {
          return sum_all(holder, ref_ts, i) / sum_all(holder, test_ts, i);
        });

        group->global->AddPoint(holder.num_threads, avg);
        group->global->SetPointError(group->global->GetN() - 1, 0.,  std_dev);
      }
  },
  StyleKinds::joined);


  add_plot<graph_plotter>
  (std::string("time_global_frac_algorithm_") + suffix,
   RETFUNC(0),
   RETFUNC(0),
   false, 0, "Fraction of the GPU Processing Time Due to the Algorithm",
   "# CPU Threads", "#font[52]{t}^{(Algorithm)}/#font[52]{t}^{(" + test_name + ")}",
   [&](graph_group_1D * group)
  {
    for (auto & holder : runs)
      {
        double avg = 0, std_dev = 0;

        calculate_stats(holder.num_events(), avg, std_dev,
                        [&](const size_t i)
        {
          return sum_all(holder, test_ts, i) / (sum_all(holder, test_ts, i) + sum_all(holder, extra_ts, i));
        });

        group->global->AddPoint(holder.num_threads, avg);
        group->global->SetPointError(group->global->GetN() - 1, 0.,  std_dev);

      }
  },
  StyleKinds::joined);


  add_plot<graph_plotter>
  (std::string("time_global_frac_convs_") + suffix,
   RETFUNC(0),
   RETFUNC(0),
   false, 0, "Fraction of the GPU Processing Time Due to Data Conversions",
   "# CPU Threads", "#font[52]{t}^{(Conversions)}/#font[52]{t}^{(" + test_name + ")}",
   [&](graph_group_1D * group)
  {
    for (auto & holder : runs)
      {
        double avg = 0, std_dev = 0;

        calculate_stats(holder.num_events(), avg, std_dev,
                        [&](const size_t i)
        {
          const double conversion_t = holder.get("EventDataExporter", "Total", i) +
                                      holder.get("ClusterImporter", "Total", i) -
                                      holder.get("EventDataExporter", "Transfer to GPU", i) -
                                      holder.get("ClusterImporter", "Transfer from GPU", i);
          //Safe to hardcode this since it will be the same for all (grow, split & both)

          const double total_time = sum_all(holder, test_ts, i) + sum_all(holder, extra_ts, i);
          return conversion_t / total_time;
        });

        group->global->AddPoint(holder.num_threads, avg);
        group->global->SetPointError(group->global->GetN() - 1, 0.,  std_dev);

      }
  },
  StyleKinds::joined);


  add_plot<graph_plotter>
  (std::string("time_global_frac_transfers_") + suffix,
   RETFUNC(0),
   RETFUNC(0),
   false, 0, "Fraction of the GPU Processing Time Due to Data Transfers",
   "# CPU Threads", "#font[52]{t}^{(Transfers)}/#font[52]{t}^{(" + test_name + ")}",
   [&](graph_group_1D * group)
  {
    for (auto & holder : runs)
      {
        double avg = 0, std_dev = 0;

        calculate_stats(holder.num_events(), avg, std_dev,
                        [&](const size_t i)
        {
          const double transfer_t = holder.get("EventDataExporter", "Transfer to GPU", i) + holder.get("ClusterImporter", "Transfer from GPU", i);
          const double total_time = sum_all(holder, test_ts, i) + sum_all(holder, extra_ts, i);
          return transfer_t / total_time;
        });

        group->global->AddPoint(holder.num_threads, avg);
        group->global->SetPointError(group->global->GetN() - 1, 0.,  std_dev);

      }
  },
  StyleKinds::joined);

}

#undef RETFUNC

#endif //CALORECGPU_TOOLS_TIMEPLOTTER_H