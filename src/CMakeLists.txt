add_library(aviutl_plugin SHARED)
set_target_properties(aviutl_plugin PROPERTIES OUTPUT_NAME "textassist.auf" PREFIX "" SUFFIX "")
target_sources(aviutl_plugin PRIVATE
  main.c
  fontlist.c
  textassist.c
  ods.c
)
target_link_libraries(aviutl_plugin PRIVATE
)

set(targets aviutl_plugin)
foreach(target ${targets})

if (("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU") OR
    ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang"))

    target_compile_definitions(${target} PRIVATE
        _WIN32_WINNT=0x0601
        $<$<CONFIG:Release>:NDEBUG=1>
    )
    target_compile_options(${target} PRIVATE
        -Wall
        -Wextra
        -pedantic-errors
        $<$<CONFIG:Debug>:-O0>
        $<$<CONFIG:Release>:-O3>
    )
    target_link_options(${target} PRIVATE
        $<$<CONFIG:Release>:-s>
        -Wl,--kill-at
    )

elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")

    target_compile_definitions(${target} INTERFACE
        _WIN32_WINNT=0x0601
    )
    target_compile_options(${target} INTERFACE
        -W4
    )

endif()

endforeach(target)
