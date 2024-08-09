# //===----------------------------------------------------------------------===//
# //
# // Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# // See https://llvm.org/LICENSE.txt for details.
# // SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# //
# //===----------------------------------------------------------------------===//
  
  

if(LIBOMP_OMPT_SUPPORT)
  include_directories(${LIBOMP_INCLUDE_DIR})
  include_directories(../multiplex/)

  add_library(archer SHARED ompt-tsan.cpp)
  add_library(archer_static STATIC ompt-tsan.cpp)

  target_link_libraries(archer ${CMAKE_DL_LIBS})
  target_link_libraries(archer_static ${CMAKE_DL_LIBS})
  install(TARGETS archer archer_static
    LIBRARY DESTINATION ${OPENMP_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${OPENMP_INSTALL_LIBDIR})

  add_subdirectory(tests)
endif()
