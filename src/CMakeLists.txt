find_program(CLANG_FORMAT_EXE clang-format)
file(GLOB sources "${CMAKE_CURRENT_SOURCE_DIR}/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
add_custom_target(${PROJECT_NAME}_format ALL
  COMMAND ${CLANG_FORMAT_EXE} -style=file -i ${sources}
)

add_custom_target(generate_version_h COMMAND
  ${CMAKE_COMMAND}
  -Dlocal_dir="${CMAKE_CURRENT_SOURCE_DIR}"
  -Dinput_file="${CMAKE_CURRENT_SOURCE_DIR}/version.h.in"
  -Doutput_file="${CMAKE_CURRENT_BINARY_DIR}/version.h"
  -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.cmake"
)

add_library(aviutl_plugin SHARED)
set_target_properties(aviutl_plugin PROPERTIES OUTPUT_NAME "textassist.auf" PREFIX "" SUFFIX "" RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
target_sources(aviutl_plugin PRIVATE
  main.c
  fontlist.c
  textassist.c
  ods.c
)
target_include_directories(aviutl_plugin BEFORE PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}" # for version.h
)
target_link_libraries(aviutl_plugin PRIVATE
  comctl32
)
add_dependencies(aviutl_plugin ${PROJECT_NAME}_format generate_version_h)
file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/bin/textassist.txt" INPUT "${CMAKE_CURRENT_SOURCE_DIR}/../README.md")

set(targets aviutl_plugin)
foreach(target ${targets})

  target_compile_definitions(${target} PRIVATE
      __STDC_NO_THREADS__
      _WIN32_WINNT=0x0601
      _WINDOWS
      $<$<CONFIG:Release>:NDEBUG>
  )
  target_compile_options(${target} PRIVATE
      --rtlib=compiler-rt
      -mstackrealign
      -Wall
      -Wextra
      -Werror
      -Weverything
      -Wshadow
      -Werror=return-type
      -pedantic-errors
      -Wno-declaration-after-statement
      -Wno-padded
      -Wno-used-but-marked-unused
      -ffunction-sections
      -fdata-sections
      -flto
      $<$<CONFIG:Debug>:-O0>
      $<$<CONFIG:Release>:-O2>
  )
  target_link_options(${target} PRIVATE
      -fuse-ld=lld
      -Wl,--gc-sections
      -Wl,--kill-at
      -no-pthread
      -static
      $<$<CONFIG:Release>:-s>
  )

endforeach(target)
