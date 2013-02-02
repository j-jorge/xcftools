cmake_minimum_required( VERSION 2.8 )

project( xcftools )

#-------------------------------------------------------------------------------
set( EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/bin" )

include_directories(src)

#-------------------------------------------------------------------------------
set(
  XCFTOOLS_COMMON_SOURCE_FILES
  src/enums.c
  src/flatspec.c
  src/flatten.c
  src/nlsini.c
  src/options.c
  src/process.c
)

#-------------------------------------------------------------------------------
set(
  XCFINFO_SOURCE_FILES
  ${XCFTOOLS_COMMON_SOURCE_FILES}
  src/io-unix.c
  src/palette.c
  src/pixels.c
  src/scaletab.c
  src/utils.c
  src/xcf-general.c
  src/xcfinfo.c
)

add_executable(
  xcfinfo
  ${XCFINFO_SOURCE_FILES}
  )

#-------------------------------------------------------------------------------
include( ${CMAKE_ROOT}/Modules/FindPNG.cmake )

set(
  XCF2PNG_SOURCE_FILES
  ${XCFTOOLS_COMMON_SOURCE_FILES}
  src/io-unix.c
  src/palette.c
  src/pixels.c
  src/scaletab.c
  src/table.c
  src/utils.c
  src/xcf2png.c
  src/xcf-general.c
)

add_executable(
  xcf2png
  ${XCF2PNG_SOURCE_FILES}
)

target_link_libraries(
  xcf2png
  ${PNG_LIBRARIES}
  )

set_target_properties(
  xcf2png
  PROPERTIES
  INCLUDE_DIRECTORIES "${PNG_INCLUDE_DIR}"
)

#-------------------------------------------------------------------------------
set(
  XCF2XPM_SOURCE_FILES
  ${XCFTOOLS_COMMON_SOURCE_FILES}
  src/io-unix.c
  src/palette.c
  src/pixels.c
  src/scaletab.c
  src/utils.c
  src/xcf2pnm.c
  src/xcf-general.c
)

#-------------------------------------------------------------------------------
add_executable(
  xcf2pnm
  ${XCF2XPM_SOURCE_FILES}
  )
