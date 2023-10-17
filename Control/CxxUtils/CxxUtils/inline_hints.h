/*
 * Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
 */

/**
 * @file CxxUtils/always_inline.h
 * @author Christos Anastopoulos
 * @date 2023
 * @brief @c wrappers
 * for forcing inline/no-inline and flatten.
 *
 * Remember in most of the cases the compiler
 * will do the "right" thing with inlining.
 *
 * There are 3 extensions/attributes we use in the Athena code base :
 *
 * - The flatten attribute causes calls within the attributed function to be
 * inlined unless it is impossible to do so, for example if the body of the
 * callee is unavailable or if the callee has the noinline attribute.
 *
 * - The noinline attribute suppresses the inlining of a function at the call
 * sites of the function.
 *
 * - The always_inline attibute disables inlining heuristics and inlining is
 * always attempted regardless of optimization level.
 *
 * These can and will affect debug and optimised builds
 *
 * As an example :
 * - We use flatten quite a bit to avoid out of line eigen calls
 * - We use always_inline for helpers containing code that was repeated in
 *   different methods and we factorized it in a separate function.
 *   But we want to keep the semantics of the original
 * - We use noinline to avoid the compiler being too smart
 *
 * More or less all these assume that given a context one knows something more
 * than the compiler. Inlining (the optimization) can improve or worsen the
 * performance, it can reduce or increase the code size. Check carefully
 * before deciding and confirm.
 *
 *
 */

#ifndef CXXUTILS_ALWAYS_INLINE_H
#define CXXUTILS_ALWAYS_INLINE_H

#if (defined(__GNUC__) || defined(__clang__))
#define ATH_FLATTEN [[gnu::flatten]]
#else
#define ATH_FLATTEN
#endif

#if (defined(__GNUC__) || defined(__clang__))
#define ATH_ALWAYS_INLINE [[gnu::always_inline]] inline
#else
#define ATH_ALWAYS_INLINE inline
#endif

#if (defined(__GNUC__) || defined(__clang__))
#define ATH_NOINLINE [[gnu::noinline]]
#else
#define ATH_NOINLINE
#endif

#endif
