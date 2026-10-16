#
# DANTE :: System Automatycznych Testów Jednostkowych oraz Akceptacyjnych
# Tomasz Jaworski, 2018-2020
#
# Plik wygenerowany automatycznie
# Znacznik czasowy: 2023-10-16 23:58:43.007346
#

cmake_minimum_required(VERSION 3.17)
project(project1 C)

# Przyjmij standard C11
set(CMAKE_C_STANDARD 11)

# Ustaw opcje kompilatora (z raportu Dante)
add_compile_options(
      "-std=c11"
      "-Wall"
      "-Wno-error=implicit-fallthrough"
      "-ggdb3"
      "-xc"
      "-pedantic"
      "-Wextra"
      "-Werror=vla"
      "-DINSIDE_DANTE"
      "-Wno-error=unused-parameter"
      "-fdiagnostics-color"
      "-Wno-error=parentheses"
      "-Wno-parentheses"
      "-Werror"
      "-fmax-errors=5"
      "-D_GNU_SOURCE"
      "-D_ANSI_OUTPUT"

)

# Ustaw opcje konsolidatora (z raportu Dante)
add_link_options(
        "-Wl,-wrap,main"
        "-ggdb3"
        "-Wl,-Map=main.map"
        "-Wl,-cref"
)

# Utwórz target 'project1' i dodaj do niego pliki źródlowe:
add_executable(project1
        "main.c"
        "heap.c"
        "tests/unit_helper_v2.c"
        "tests/unit_test_v2.c"
        "tests/rdebug.c"
        "tests/memmanager.c"
)

target_include_directories(project1 PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
)

# Dołącz niezbędne biblioteki
target_link_libraries(project1
        "m"
        "pthread"
)