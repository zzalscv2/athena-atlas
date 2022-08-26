# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration


# This package needs CUDA to be handled as a "first class language" by CMake.
cmake_minimum_required(VERSION 3.11)

# Set the name of the package.
atlas_subdir( CaloRecGPU )

# The build of this package needs CUDA. If it's not available, don't try to do
# anything...
if( NOT CMAKE_CUDA_COMPILER )
  message( STATUS "CUDA not found, CaloRecGPU is not built" )
  return()
endif()

find_package( Boost COMPONENTS chrono filesystem )
find_package( ROOT COMPONENTS Core MathCore Hist Gpad Graf )

# Add a component library that has some CUDA code in it.
atlas_add_library( CaloRecGPULib
   CaloRecGPU/*.h src/*.cxx src/*.cu src/*.h
   PUBLIC_HEADERS CaloRecGPU
   LINK_LIBRARIES AthenaBaseComps GaudiKernel CaloEvent CaloRecLib CaloInterfaceLib TrigT2CaloCommonLib CxxUtils
   PRIVATE_INCLUDE_DIRS ${Boost_INCLUDE_DIRS})

atlas_add_component( CaloRecGPU
   src/components/*.cxx
   LINK_LIBRARIES CaloRecGPULib )

atlas_add_executable( calorecgpu_plotter
      tools/plotter/clusterplotter.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} CaloRecGPULib )

atlas_add_executable( calorecgpu_optimizer
      tools/optimizer/Optimize.cu
      INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
      LINK_LIBRARIES CaloRecGPULib )

atlas_add_executable( calorecgpu_methodchecker
      tools/methodchecker/NewMethodCheck.cu
      INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
      LINK_LIBRARIES CaloRecGPULib )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

set_target_properties( CaloRecGPULib PROPERTIES
   CUDA_SEPARABLE_COMPILATION ON
   POSITION_INDEPENDENT_CODE ON
   INTERFACE_POSITION_INDEPENDENT_CODE ON
   CUDA_STANDARD 17
   CUDA_STANDARD_REQUIRED TRUE )

set_target_properties( calorecgpu_methodchecker PROPERTIES
   CUDA_SEPARABLE_COMPILATION ON
   POSITION_INDEPENDENT_CODE ON
   INTERFACE_POSITION_INDEPENDENT_CODE ON
   CUDA_STANDARD 17
   CUDA_STANDARD_REQUIRED TRUE )
   
set_target_properties( calorecgpu_optimizer PROPERTIES
   CUDA_SEPARABLE_COMPILATION ON
   POSITION_INDEPENDENT_CODE ON
   INTERFACE_POSITION_INDEPENDENT_CODE ON
   CUDA_STANDARD 17
   CUDA_STANDARD_REQUIRED TRUE )
   
target_compile_definitions(CaloRecGPULib PRIVATE CUDA_API_PER_THREAD_DEFAULT_STREAM=1)
target_compile_options(CaloRecGPULib PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:-expt-relaxed-constexpr>)

target_compile_definitions(calorecgpu_optimizer PRIVATE CUDA_API_PER_THREAD_DEFAULT_STREAM=1)
target_compile_options(calorecgpu_optimizer PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:-expt-relaxed-constexpr>)

target_compile_definitions(calorecgpu_methodchecker PRIVATE CUDA_API_PER_THREAD_DEFAULT_STREAM=1)
target_compile_options(calorecgpu_methodchecker PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:-expt-relaxed-constexpr>)