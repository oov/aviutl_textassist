option(FORMAT_SOURCES "execute clang-format" ON)

if(FORMAT_SOURCES)
  file(GLOB_RECURSE sources LIST_DIRECTORIES false CONFIGURE_DEPENDS "*.h" "*.c")
  list(FILTER sources EXCLUDE REGEX "${CMAKE_CURRENT_SOURCE_DIR}/3rd")
  find_program(CLANG_FORMAT_EXE clang-format)
  add_custom_target(${PROJECT_NAME}-format ALL
    COMMAND ${CLANG_FORMAT_EXE} -style=file -i ${sources}
  )
endif()

add_custom_target(generate_version_h COMMAND
  ${CMAKE_COMMAND}
  -Dlocal_dir="${CMAKE_CURRENT_SOURCE_DIR}"
  -Dinput_file="${CMAKE_CURRENT_SOURCE_DIR}/version.h.in"
  -Doutput_file="${CMAKE_CURRENT_BINARY_DIR}/version.h"
  -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.cmake"
)

add_library(textassist_intf INTERFACE)
target_compile_definitions(textassist_intf INTERFACE
  __STDC_NO_THREADS__
  _WIN32_WINNT=0x0601
  _WINDOWS
  $<$<CONFIG:Release>:NDEBUG>
)
target_compile_options(textassist_intf INTERFACE
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
  -Wno-unsafe-buffer-usage
  -Wno-switch-default
  -ffunction-sections
  -fdata-sections
  -flto
  $<$<CONFIG:Debug>:-O0>
  $<$<CONFIG:Release>:-O2>
)
target_link_options(textassist_intf INTERFACE
  -fuse-ld=lld
  -Wl,--gc-sections
  -Wl,--kill-at
  -static
  $<$<CONFIG:Release>:-s>
)
target_include_directories(textassist_intf BEFORE INTERFACE
  "${CMAKE_CURRENT_BINARY_DIR}" # for version.h
)
target_link_libraries(textassist_intf INTERFACE
  comctl32
)

add_library(textassist SHARED)
set_target_properties(textassist PROPERTIES OUTPUT_NAME "textassist.auf" PREFIX "" SUFFIX "" RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
target_sources(textassist PRIVATE
  main.c
  fontlist.c
  textassist.c
  ods.c
)
target_link_libraries(textassist PRIVATE textassist_intf)
add_dependencies(textassist ${PROJECT_NAME}-format generate_version_h)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/../README.md" "${CMAKE_BINARY_DIR}/bin/textassist.txt" @ONLY NEWLINE_STYLE CRLF)

add_executable(textassist_test
  test.c
  fontlist.c
  ods.c
)
target_link_libraries(textassist_test PRIVATE textassist_intf)
add_test(NAME textassist_test COMMAND $<TARGET_FILE:textassist_test>)
add_dependencies(textassist_test ${PROJECT_NAME}-format generate_version_h)
