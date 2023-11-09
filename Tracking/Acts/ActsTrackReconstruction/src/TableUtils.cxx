/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "TableUtils.h"

namespace TableUtils {

   std::size_t computeSum( const std::vector< SummandDefinition >  &sum_def,
                                  std::size_t eta_offset,
                                  std::size_t row_stride,
                                  std::size_t seed_i,
                                  const std::vector<std::size_t> &stat) {
      std::size_t sum=0;
      for (const SummandDefinition &summand : sum_def ) {
         assert( eta_offset + row_stride * summand.first + seed_i < stat.size() );
         sum += stat[eta_offset + row_stride * summand.first + seed_i ] * summand.second;
      }
      return sum;
   }

   std::vector<float> computeRatios(const std::vector<RatioDefinition> &ratio_def,
                                    const std::size_t categories,
                                    const std::size_t sub_categories,
                                    const std::vector< std::size_t> &counter)
   {
       std::vector<float> ratio;
       if (counter.size() % (categories*sub_categories) ) {
          std::stringstream msg;
          msg << "Category and sub category dimensions " << categories << " * " << sub_categories
              << " not a common factor of the input counter vector "
              << counter.size();
          throw std::logic_error(msg.str());
       }
       std::size_t n_counter = counter.size() / (categories*sub_categories);
       ratio.resize( categories * ratio_def.size() * sub_categories);

       std::size_t input_counter_stride=categories;
       std::size_t input_sub_category_stride = categories * n_counter;
       std::size_t ratio_stride = categories;
       std::size_t ratio_sub_category_stride = ratio_stride * ratio_def.size();

       for (std::size_t sub_category_i=0; sub_category_i < sub_categories; ++sub_category_i) {
          for (std::size_t ratio_i=0; ratio_i<ratio_def.size(); ++ratio_i) {
             for (std::size_t category_i=0; category_i<categories; ++category_i) {
                std::size_t ratio_dest_idx = sub_category_i * ratio_sub_category_stride + ratio_stride * ratio_i + category_i;
                assert(ratio_dest_idx < ratio.size());
                assert( sub_category_i * input_sub_category_stride < counter.size() );
                assert( sub_category_i * input_sub_category_stride + n_counter * input_counter_stride <= counter.size() );
                ratio[ratio_dest_idx] = computeRatio( ratio_def.at(ratio_i),
                                                      sub_category_i * input_sub_category_stride,
                                                      input_counter_stride,
                                                      category_i,
                                                      counter);
             }
          }
       }
       return ratio;
   }
}
