// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
//

#ifndef TOPOAUTOMATONCLUSTEROPTIMIZER_TEST_DEFINES_H
#define TOPOAUTOMATONCLUSTEROPTIMIZER_TEST_DEFINES_H

#include <vector>
#include <string>
#include <fstream>
#include <thread>
#include <algorithm>
#include "CaloRecGPU/StandaloneDataIO.h"
#include "CaloRecGPU/CUDAFriendlyClasses.h"

#include "OptimizerDataHolders.h"
#include "DefaultImplementation.h"

#include <time.h>


void setup_cuda_device()
{
  int devID = 0;
  cudaDeviceProp props;

  /* maybe we want something else here */
  cudaSetDevice(0);

  cudaGetDeviceProperties(&props, devID);
  std::cout << "[CUDA] Device " << devID << " " << props.name <<  " with compute capability " << props.major << "." << props.minor << std::endl;
}


template <class T>
struct top_n_holder
//Simple, no-frills sorted vector.
//(We assume T provides a strong ordering.)
{
 private:
  size_t m_nmax;
  std::vector<T> m_buff;
 public:
  top_n_holder(const size_t sz): m_nmax(sz)
  {
    m_buff.reserve(m_nmax);
  }
  void try_add(const T & val)
  {
    auto greater_it = std::lower_bound(m_buff.begin(), m_buff.end(), val);
    if (m_buff.size() < m_nmax)
      {
        m_buff.insert(greater_it, val);
      }
    else
      {
        if (greater_it == m_buff.begin())
          {
            return;
          }
        else
          {
            for (auto it = m_buff.begin(); (it + 1) != greater_it; ++it)
              {
                *it = *(it + 1);
              }
            *(greater_it - 1) = val;
          }
      }
  }
  const T & operator[] (const size_t i) const
  {
    return m_buff[i];
  }
  size_t size() const
  {
    return m_buff.size();
  }
};

struct loop_range
{
  using type = int;
  //It's enough for our purposes,
  //but might need to be bumped up to a int64_t to be sure...

  type start, stop, step;

  loop_range(const type begin = 0, const type end = 0, const type iter = 1):
    start(begin), stop(end), step(iter)
  {
  }

  type get_iteration(const type num) const
  //Returns the iteration number corresponding to the nearest
  //element of the iteration not greater than `num`
  //(that is, if `num` is not part of the numbers
  // that can be reached given `start` and `step`,
  // it's rounded down.)
  {
    return (num - start) / step ;
  }

  type get_number_of_iterations() const
  {
    return (stop - start) / step + 1;
  }

  type get_number_from_iteration(const type num) const
  {
    return start + num * step;
  }

  type get_number_in_the_middle() const
  //Rounded down in case there's an even number of iterations...
  {
    return get_number_from_iteration(get_number_of_iterations() / 2 - (get_number_of_iterations() % 2 == 0));
  }

  type last() const
  {
    return get_number_from_iteration(get_number_of_iterations() - 1);
    //Not necessarily stop...
  }

  type first() const
  {
    return start;
  }

  template <class F, class ... Args>
  void loop (F && f, Args && ... args) const
  {
    for (type i = start, count = 0; i <= stop; ++count, i += step )
      {
        f(count, i, std::forward<Args>(args)...);
      }
  }


};


namespace Timer
{
  using time_type = timespec;
  time_type mark_time()
  {
    time_type ret;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ret);
    return ret;
  }
  double time_diff(const time_type & begin, const time_type & end)
  //In miliseconds...
  {
    return ((double) (end.tv_sec - begin.tv_sec)) * 1e3 + ((double) (end.tv_nsec - begin.tv_nsec)) * 1e-6;
  }
};

union f_pointers
{
  void (*zero_loop) (ProcessingDataHolder &, const InstanceDataHolder &);
  void (*one_loop) (ProcessingDataHolder &, const InstanceDataHolder &, const int);
  void (*two_loop) (ProcessingDataHolder &, const InstanceDataHolder &, const int, const int);
  void (*three_loop) (ProcessingDataHolder &, const InstanceDataHolder &, const int, const int, const int);
  void (*four_loop) (ProcessingDataHolder &, const InstanceDataHolder &, const int, const int, const int, const int);
};

void prepare_data_for_event(ProcessingDataHolder & event_data, const ProcessingDataHolder & stored_data)
{
  event_data.m_cell_info_dev = stored_data.m_cell_info;
  event_data.m_cell_state_dev = stored_data.m_cell_state;
  event_data.m_pairs_dev = stored_data.m_pairs;
  event_data.m_half_pairs_dev = stored_data.m_half_pairs;
  event_data.m_clusters_dev = stored_data.m_clusters;
  event_data.m_temporaries_dev = stored_data.m_temporaries;
}

void get_data_from_event(const ProcessingDataHolder & event_data, ProcessingDataHolder & stored_data)
{
  stored_data.m_cell_info = event_data.m_cell_info_dev;
  stored_data.m_cell_state = event_data.m_cell_state_dev;
  stored_data.m_pairs = event_data.m_pairs_dev;
  stored_data.m_half_pairs = event_data.m_half_pairs_dev;
  stored_data.m_clusters = event_data.m_clusters_dev;
  stored_data.m_temporaries = event_data.m_temporaries_dev;
}

struct test_function
{
  constexpr static size_t s_max_num_loops = 4;

  loop_range::type num_loops;
  f_pointers pointers;
  loop_range block_sizes[s_max_num_loops];
  loop_range thread_numbers;
  std::string label;

  static loop_range::type s_num_reps;

  static loop_range::type s_compress_loops_after;

  static loop_range::type s_keep_best;

  static std::string s_prefix;

 private:

  void unpack_loops(loop_range * const block_sizes)
  {
  }


  void unpack_loops(loop_range * const block_sizes, const loop_range & lr)
  {
    *block_sizes = lr;
  }

  template <class ... Loops>
  void unpack_loops(loop_range * const block_sizes, const loop_range & lr, Loops && ... loops)
  {
    *block_sizes = lr;
    unpack_loops(block_sizes + 1, std::forward<Loops>(loops)...);
  }

 public:

  template <class Func, class ... Loops>
  test_function(const std::string & lbl, Func && f, const loop_range & threads, Loops && ... loops)
  {
    label = lbl;
    thread_numbers = threads;
    num_loops = sizeof...(loops);
    //We expect this to match.
    //Please don't break it.
    //Function pointers will barf and so on.
    //Stack corruption, world exploding,
    //computer on fire,
    //ANYTHING CAN HAPPEN WILL HAPPEN

    if (num_loops > s_max_num_loops)
      {
        num_loops = 0;
        std::cout << "ERROR: " << num_loops << " is too many loops for testing!" << std::endl;
        pointers.zero_loop = nullptr;
      }
    else
      {
        unpack_loops(block_sizes, std::forward<Loops>(loops)...);
        switch (num_loops)
          {
            case 0:
              pointers.zero_loop = (decltype(pointers.zero_loop)) f;
              break;
            case 1:
              pointers.one_loop = (decltype(pointers.one_loop)) f;
              break;
            case 2:
              pointers.two_loop = (decltype(pointers.two_loop)) f;
              break;
            case 3:
              pointers.three_loop = (decltype(pointers.three_loop)) f;
              break;
            case 4:
              pointers.four_loop = (decltype(pointers.four_loop)) f;
              break;
            default:
              break;
          }
        //I know, I know.
        //std::variant is just around the corner...
        //In C++17, that is.
      }


  }

 private:

  struct test_parameters
  {
    loop_range::type block_size[s_max_num_loops];
    loop_range::type n_threads;
    template <class stream>
    void print(stream & s, const loop_range::type num_loops, bool print_pretty = false) const
    {
      if (print_pretty)
      {
        s << n_threads << " |";
        for (loop_range::type i = 0; i < num_loops; ++i)
          {
            s << " " << block_size[i];
          }
      }
      else
      {
        s << n_threads;
        for (loop_range::type i = 0; i < num_loops; ++i)
          {
            s << "," << block_size[i];
          }
      }
    }
  };

  double test_one_event(const test_parameters & params, ProcessingDataHolder & event_data, const InstanceDataHolder & instance_data) const
  {
    switch (num_loops)
      {
        case 0:
          {
            auto start = Timer::mark_time();
            pointers.zero_loop(event_data, instance_data);
            auto finish = Timer::mark_time();
            return Timer::time_diff(start, finish);
          }
        case 1:
          {
            auto start = Timer::mark_time();
            pointers.one_loop(event_data, instance_data, params.block_size[0]);
            auto finish = Timer::mark_time();
            return Timer::time_diff(start, finish);
          }
        case 2:
          {
            auto start = Timer::mark_time();
            pointers.two_loop(event_data, instance_data, params.block_size[0], params.block_size[1]);
            auto finish = Timer::mark_time();
            return Timer::time_diff(start, finish);
          }
        case 3:
          {
            auto start = Timer::mark_time();
            pointers.three_loop(event_data, instance_data, params.block_size[0], params.block_size[1], params.block_size[2]);
            auto finish = Timer::mark_time();
            return Timer::time_diff(start, finish);
          }
        case 4:
          {
            auto start = Timer::mark_time();
            pointers.four_loop(event_data, instance_data, params.block_size[0], params.block_size[1], params.block_size[2], params.block_size[3]);
            auto finish = Timer::mark_time();
            return Timer::time_diff(start, finish);
          }
        default:
          return 9e99;
      }
  }

  void test_one_thread(const loop_range::type thread_idx,
                       const test_parameters & params,
                       std::vector<double> & times,
                       ProcessingDataHolder & event_data,
                       const InstanceDataHolder & instance_data,
                       const std::vector<ProcessingDataHolder> & event_states) const
  {
    for (loop_range::type reps = 0; reps < s_num_reps; ++reps)
      {
        for (loop_range::type i = thread_idx; i < event_states.size(); i += params.n_threads)
          {
            prepare_data_for_event(event_data, event_states[i]);
            times[i] += test_one_event(params, event_data, instance_data) / s_num_reps;
          }
      }
  }

  struct test_result
  {
    double average;
    double corrected_average;
    template <class stream>
    friend stream & operator << (stream & s, const test_result & tr)
    {
      return s << tr.corrected_average << " (" << tr.average << ")";
    }
  };

  test_result execute_test(std::ofstream & out,
                           const test_parameters & params,
                           std::vector<double> & times,
                           std::vector<ProcessingDataHolder> & thread_data,
                           const InstanceDataHolder & instance_data,
                           const std::vector<ProcessingDataHolder> & event_states,
                           const bool pretty_print = false) const
  {
    if (pretty_print)
      {
        out << "\n";
      }
    params.print(out, num_loops, pretty_print);
    if (pretty_print)
      {
        out << "\n";
      }

    times.clear();
    times.resize(event_states.size(), 0.);

    std::vector<std::thread> threads;
    threads.reserve(params.n_threads);

    for (loop_range::type i = 0; i < params.n_threads; ++i)
      {
        threads.emplace_back( [ &, i]()
        {
          this->test_one_thread(i, params, times, thread_data[i], instance_data, event_states);
        } );
      }
    for (auto & thread : threads)
      {
        thread.join();
      }

    test_result ret {0., 0.};

    if (pretty_print)
      {
        out << "\nTimes (ms): ";
        for (const auto & t : times)
          {
            out << t << " ";
          }
      }

    std::sort(times.begin(), times.end());

    const size_t ignore_last = std::min(size_t(30), size_t(times.size() / 8));

    for (size_t i = 0; i < times.size(); ++i)
      {
        ret.average += times[i];
        if (i < times.size() - ignore_last)
          {
            ret.corrected_average += times[i];
          }
      }

    const double corr_mu = ret.corrected_average / (times.size() - ignore_last);
    const double exclude_factor = 20.0;
    //We exclude times that are greater than 20 times the distance from the approximate average to the first value,
    //assuming measurements are approximately symmetric to the average (e. g. normal distribution)
    //and thus those way outside the curve must be wrong.
    const double max_valid = corr_mu + (corr_mu - times[0]) * exclude_factor;
    if (times[times.size() - ignore_last - 1] > exclude_factor)
      {
        for (int i = times.size() - ignore_last; i > 0; --i)
          {
            const double this_time = times[i - 1];
            if (this_time <= exclude_factor)
              {
                ret.corrected_average /= i;
                break;
              }
            ret.corrected_average -= this_time;
          }
      }
    else if (times[times.size() - ignore_last - 1] < exclude_factor)
      {
        bool broke = false;
        for (int i = times.size() - ignore_last - 1; i < times.size(); ++i)
          {
            const double this_time = times[i];
            if (this_time > exclude_factor)
              {
                ret.corrected_average /= (i - 1);
                broke = true;
                break;
              }
            ret.corrected_average += this_time;
          }
        if (!broke)
          {
            ret.corrected_average /= times.size();
          }
      }
    else /*if (times[times.size() - ignore_last - 1] == exclude_factor)*/
      {
        ret.corrected_average /= (times.size() - ignore_last);
      }

    ret.average /= times.size();

    if (pretty_print)
      {
        out << "\nAverage (ms): " << ret << std::endl;
      }

    return ret;
  }



  struct best_result
  {
    test_parameters parameters;
    double throughput;

    friend bool operator > (const best_result & r1, const best_result & r2)
    {
      return r1.throughput > r2.throughput;
    }
    friend bool operator < (const best_result & r1, const best_result & r2)
    {
      return r1.throughput < r2.throughput;
    }
    template <class stream>
    void print(stream & s, const loop_range::type num_loops, const bool pretty_print = false) const
    {
      if (pretty_print)
        { 
          s << throughput << " (";
          parameters.print(s, num_loops, pretty_print);
          s << ")";
        }
    }
  };


  best_result recurse_test(loop_range::type it_num,
                           std::ofstream & out,
                           test_parameters & params,
                           std::vector<double> & times,
                           std::vector<ProcessingDataHolder> & thread_data,
                           const InstanceDataHolder & instance_data,
                           const std::vector<ProcessingDataHolder> & event_states,
                           const bool pretty_print = false) const
  {
    best_result res {{}, 0.};
    if (it_num >= num_loops)
      {
        test_result tr = execute_test(out, params, times, thread_data, instance_data, event_states, pretty_print);
        res.parameters = params;
        res.throughput = 1e3 * params.n_threads / tr.corrected_average; //So we get events/second
        if (!pretty_print)
          {
            out << "," << res.throughput << "\n";
          }
        return res;
      }
    if ((it_num + 1) >= s_compress_loops_after)
      {
        top_n_holder<best_result> best_parameters(s_keep_best);

        block_sizes[it_num].loop([&](loop_range::type count, loop_range::type block_size)
        {
          params.block_size[it_num] = block_size;

          const best_result temp = this->recurse_test(num_loops, out, params, times, thread_data, instance_data, event_states, pretty_print);

          best_parameters.try_add(temp);
        }
                                );

        if (it_num + 1 >= num_loops)
          {
            if (best_parameters.size() > 0)
              {
                res = best_parameters[best_parameters.size() - 1];
              }
          }
        else
          {
            for (loop_range::type i = 0; i < best_parameters.size(); ++i)
              {
                params.block_size[it_num] = best_parameters[i].parameters.block_size[it_num];
                const best_result temp = recurse_test(it_num + 1, out, params, times, thread_data, instance_data, event_states, pretty_print);
                res = std::max(res, temp);
              }
          }
      }
    else
      {
        block_sizes[it_num].loop([&](loop_range::type count, loop_range::type block_size)
        {
          params.block_size[it_num] = block_size;

          const best_result temp = this->recurse_test(it_num + 1, out, params, times, thread_data, instance_data, event_states, pretty_print);

          res = std::max(res, temp);
        }
                                );
      }
    return res;
  }

  best_result thread_test(std::ofstream & out,
                          std::vector<double> & times,
                          std::vector<ProcessingDataHolder> & thread_data,
                          const InstanceDataHolder & instance_data,
                          const std::vector<ProcessingDataHolder> & event_states,
                          const bool pretty_print = false)
  {
    best_result res {{}, 0.};
    test_parameters params;
    for (loop_range::type i = 0; i < num_loops; ++i)
      {
        params.block_size[i] = block_sizes[i].get_number_in_the_middle();
      }
    thread_numbers.loop([&](loop_range::type count, loop_range::type num_threads)
    {
      params.n_threads = num_threads;
      best_result temp = this->recurse_test(0, out, params, times, thread_data, instance_data, event_states, pretty_print);

      res = std::max(res, temp);

      if (pretty_print)
        {
          out << "\nBest: " << temp.throughput << " events / s | ";
        }

      temp.throughput = 1.0 / (temp.throughput * num_threads);
      //For pretty printing

      temp.print(out, num_loops, pretty_print);

      if (pretty_print)
        {
          out << "\n";
        }
    }
                       );
    return res;
  }


 public:

  void test(std::vector<ProcessingDataHolder> & thread_data,
            const InstanceDataHolder & instance_data,
            const std::vector<ProcessingDataHolder> & event_states,
            const bool pretty_print)
  {
    std::cout << "Testing " << label << " (" << thread_numbers.start << " " << thread_numbers.stop << " " << thread_numbers.step;

    for (loop_range::type i = 0; i < num_loops; ++i)
      {
        std::cout << " | " << block_sizes[i].start << " " << block_sizes[i].stop << " " << block_sizes[i].step;
      }

    std::cout << ")" << std::endl;

    std::ofstream out(s_prefix + (s_prefix.size() > 0 ? "_" : "") + label + ".txt", std::ios_base::app);

    out << label << "\n";

    std::vector<double> times;

    const best_result best = thread_test(out, times, thread_data, instance_data, event_states, pretty_print);

    if (pretty_print)
      {
        out << "\n\nOut of all: " << best.throughput << " events / s | "
            << 1.0 / (best.throughput * best.parameters.n_threads) << " ms (";
        best.parameters.print(out, num_loops, pretty_print);
        out << ")" << std::endl;
      }
  }

};


loop_range::type test_function::s_num_reps = 2;
loop_range::type test_function::s_compress_loops_after = 1;
loop_range::type test_function::s_keep_best = 5;
std::string test_function::s_prefix = "";


#include "DefaultImplementation.h"

#include "HalvedPairs.h"


namespace DefaultImplementation
{
  //The following are just for us to work with default parameters and function pointers.
  //(Which won't work otherwise.)

  void signal_to_noise_def(ProcessingDataHolder & holder, const InstanceDataHolder & instance_data)
  {
    signal_to_noise(holder, instance_data);
  }
  void cell_pairs_def(ProcessingDataHolder & holder, const InstanceDataHolder & instance_data)
  {
    cell_pairs(holder, instance_data);
    HalvedPairs::cell_pairs(holder, instance_data);
  }
  void cluster_growing_def(ProcessingDataHolder & holder, const InstanceDataHolder & instance_data)
  {
    cluster_growing(holder, instance_data);
  }
  void finalize_clusters_def(ProcessingDataHolder & holder, const InstanceDataHolder & instance_data)
  {
    finalize_clusters(holder, instance_data);
  }
}

struct TestHolder
{
 private:

  std::vector<test_function> snr_funcs;
  std::vector<test_function> pair_funcs;
  std::vector<test_function> grow_funcs;
  std::vector<test_function> finalize_funcs;

  std::vector<ProcessingDataHolder> event_states;


  mutable std::vector<ProcessingDataHolder> thread_data;

  InstanceDataHolder instance_data;

  loop_range::type max_threads;
  loop_range::type events_to_use;

  template <class Str>
  bool ends_with(const Str & str1, const Str & str2)
  {
    if (str1.size() < str2.size())
      {
        return false;
      }
    else
      {
        return (str1.substr(str1.size() - str2.size()) == str2);
      }
  }

  void setup_thread_data()
  {
    thread_data.resize(max_threads);
    for (auto & thd : thread_data)
      {
        thd.m_cell_info_dev.allocate();
        thd.m_cell_state_dev.allocate();
        thd.m_pairs_dev.allocate();
        thd.m_half_pairs_dev.allocate();
        thd.m_clusters_dev.allocate();
        thd.m_temporaries_dev.allocate();
      }
  }

  template <class F>
  void process_one_thread(const loop_range::type thread_idx, F && f)
  {
    for (loop_range::type i = thread_idx; i < event_states.size(); i += max_threads)
      {
        prepare_data_for_event(thread_data[thread_idx], event_states[i]);
        f(thread_data[thread_idx], instance_data);
        get_data_from_event(thread_data[thread_idx], event_states[i]);
      }
  }

  template <class F>
  void process_events(F && f)
  {
    std::vector<std::thread> threads;
    threads.reserve(max_threads);

    for (loop_range::type i = 0; i < max_threads; ++i)
      {
        threads.emplace_back([ &, i]()
        {
          this->process_one_thread(i, std::forward<F>(f));
        });
      }
    for (auto & thread : threads)
      {
        thread.join();
      }
    std::cout << "\nDone." << std::endl;
  }

 public:

  void load(const std::string & folder, const size_t max_events)
  {
    events_to_use = max_events;
    auto fold_info = StandaloneDataIO::load_folder(folder, max_events, false); //Can skip loading clusters.

    instance_data.prepare(std::move(fold_info.geometry.begin()->second), std::move(fold_info.noise.begin()->second));
    instance_data.send(true);
    //No need to keep this data on CPU.

    for (auto & kv : fold_info.cell_info)
      {
        event_states.emplace_back();
        event_states.back().prepare(std::move(kv.second));
      }
  }

  void load(const std::string & folder)
  {
    load(folder, events_to_use);
  }

  TestHolder(const loop_range::type evs = std::numeric_limits<loop_range::type>::max()):
    max_threads(0), events_to_use(evs)
  {
  }

  TestHolder(const std::string & folder, const loop_range::type evs = std::numeric_limits<loop_range::type>::max()):
    max_threads(0), events_to_use(evs)
  {
    load(folder);
  }

  template <class ... T>
  void add_snr_func(T && ... args)
  //Template for perfect forwarding...
  {
    snr_funcs.emplace_back(std::forward<T>(args)...);
    max_threads = std::max(max_threads, snr_funcs.back().thread_numbers.last());
  }

  template <class ... T>
  void add_pair_func(T && ... args)
  //Template for perfect forwarding...
  {
    pair_funcs.emplace_back(std::forward<T>(args)...);
    max_threads = std::max(max_threads, pair_funcs.back().thread_numbers.last());
  }

  template <class ... T>
  void add_grow_func(T && ... args)
  //Template for perfect forwarding...
  {
    grow_funcs.emplace_back(std::forward<T>(args)...);
    max_threads = std::max(max_threads, grow_funcs.back().thread_numbers.last());
  }

  template <class ... T>
  void add_finalize_func(T && ... args)
  //Template for perfect forwarding...
  {
    finalize_funcs.emplace_back(std::forward<T>(args)...);
    max_threads = std::max(max_threads, finalize_funcs.back().thread_numbers.last());
  }

  void do_tests(const bool pretty_print = false)
  {
    setup_thread_data();
    for (auto & tf : snr_funcs)
      {
        tf.test(thread_data, instance_data, event_states, pretty_print);
      }

    process_events(DefaultImplementation::signal_to_noise_def);

    for (auto & tf : pair_funcs)
      {
        tf.test(thread_data, instance_data, event_states, pretty_print);
      }

    process_events(&DefaultImplementation::cell_pairs_def);

    for (auto & tf : grow_funcs)
      {
        tf.test(thread_data, instance_data, event_states, pretty_print);
      }

    process_events(&DefaultImplementation::cluster_growing_def);

    for (auto & tf : finalize_funcs)
      {
        tf.test(thread_data, instance_data, event_states, pretty_print);
      }

  }

};

#endif