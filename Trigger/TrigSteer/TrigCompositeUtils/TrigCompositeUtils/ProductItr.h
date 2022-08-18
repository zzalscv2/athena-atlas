/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGCOMPOSITEUTILS_PRODUCTITR_H
#define TRIGCOMPOSITEUTILS_PRODUCTITR_H

#include <functional>
#include <iterator>
#include <type_traits>
#include <vector>

namespace TrigCompositeUtils {
    /**
     * @brief Iterates over all combinations of the provided input iterators
     * @tparam Iterator The type of input iterator
     *
     * Dereferencing the iterator gives the states of all the input iterators in the current
     * combination.
     *
     * Combinations are generated such that the last iterator is the most fastly varying.
     *
     * std::vector<std::size_t> v1{0, 1, 2};
     * std::vector<std::size_t> v2{0, 1};
     * ProductItr<std::vector<std::size_t>::const_iterator> itr(
     *          {v1.begin(), v2.begin()}, {v1.end(), v2.end()});
     * for (; !itr.exhausted(); ++itr)
     * {
     *     for (std::vector<std::size_t>::const_iterator itr2 ; *itr)
     *         std::cout << *itr2 << ", "
     *     std::cout << std::endl;
     * }
     *
     * Will produce
     * 0, 0,
     * 0, 1,
     * 1, 0,
     * 1, 1,
     * 2, 0,
     * 2, 1,
     */
    template <typename Iterator>
    class ProductItr {
    public:
        /// Iterator traits
        using iterator_category = std::input_iterator_tag;
        using value_type = std::vector<Iterator>;
        using reference = const value_type &;
        using pointer = const value_type *;
        using difference_type = std::ptrdiff_t;

        /// Default constructor creates a generic past-the-end iterator
        ProductItr() = default;

        /**
         * @brief Construct the iterator from an input vector of iterators
         *
         * @param itrs The start positions of the internal iterators
         * @param endItrs The end positions of the internal iterators
         */
        ProductItr(const std::vector<Iterator> &itrs, const std::vector<Iterator> &endItrs);

        /// The number of input iterators
        std::size_t nItrs() const { return m_startItrs.size(); }

        /// Helper function to reset this iterator to its start position
        void reset();

        /// True if this iterator is past the end
        bool exhausted() const;

        /// Dereference
        reference operator*() const;
        pointer operator->() const;

        /// Pre-increment operator
        ProductItr &operator++();

        /// Post-increment operator
        ProductItr operator++(int);

        /// Iterator comparison functions
        bool operator==(const ProductItr &other) const;
        bool operator!=(const ProductItr &other) const;

    private:
        std::vector<Iterator> m_startItrs;
        std::vector<Iterator> m_endItrs;
        std::vector<Iterator> m_currentItrs;

    }; //> end class ProductItr<Iterator>
} // namespace TrigCompositeUtils

#include "TrigCompositeUtils/ProductItr.icc"

#endif //> !TRIGCOMPOSITEUTILS_PRODUCT_ITR_H
