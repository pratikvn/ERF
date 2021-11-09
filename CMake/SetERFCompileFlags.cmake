function(get_all_targets var)
    set(targets)
    get_all_targets_recursive(targets ${CMAKE_CURRENT_SOURCE_DIR})
    set(${var} ${targets} PARENT_SCOPE)
endfunction()

macro(get_all_targets_recursive targets dir)
    get_property(subdirectories DIRECTORY ${dir} PROPERTY SUBDIRECTORIES)
    foreach(subdir ${subdirectories})
        get_all_targets_recursive(${targets} ${subdir})
    endforeach()

    get_property(current_targets DIRECTORY ${dir} PROPERTY BUILDSYSTEM_TARGETS)
    list(APPEND ${targets} ${current_targets})
endmacro()

function(set_erf_compile_flags target)
# Logic for handling warnings
if(ERF_ENABLE_ALL_WARNINGS)
  # GCC, Clang, and Intel seem to accept these
  list(APPEND ERF_CXX_FLAGS "-Wall" "-Wextra" "-pedantic")
  if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
    # ifort doesn't like -Wall
    list(APPEND ERF_Fortran_FLAGS "-Wall")
  else()
    # Intel always reports some diagnostics we don't necessarily care about
    list(APPEND ERF_CXX_FLAGS "-diag-disable:11074,11076")
  endif()
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 7.0)
    # Avoid notes about -faligned-new with GCC > 7
    list(APPEND ERF_CXX_FLAGS "-faligned-new")
  endif()
endif()

# Add our extra flags according to language
separate_arguments(ERF_CXX_FLAGS)
target_compile_options(${target} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:${ERF_CXX_FLAGS}>)

if(ERF_ENABLE_CUDA)
  list(APPEND ERF_CUDA_FLAGS "--expt-relaxed-constexpr")
  list(APPEND ERF_CUDA_FLAGS "--expt-extended-lambda")
  list(APPEND ERF_CUDA_FLAGS "--Wno-deprecated-gpu-targets")
  list(APPEND ERF_CUDA_FLAGS "-m64")
  if(ENABLE_CUDA_FASTMATH)
    list(APPEND ERF_CUDA_FLAGS "--use_fast_math")
  endif()
  separate_arguments(ERF_CUDA_FLAGS)
  target_compile_options(${target} PUBLIC $<$<COMPILE_LANGUAGE:CUDA>:${ERF_CUDA_FLAGS}>)
  # Add arch flags to both compile and linker to avoid warnings about missing arch
  set(CMAKE_CUDA_FLAGS ${NVCC_ARCH_FLAGS})
  set_target_properties(
    ${target} PROPERTIES
    CUDA_SEPARABLE_COMPILATION ON
    CUDA_RESOLVE_DEVICE_SYMBOLS ON)
endif()
endfunction()