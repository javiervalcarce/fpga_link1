########################################################################################################################
# CMakefile para compilar la librería thdanalyzer y los programas de ejemplo asociados.
#
# Para compilar hay que instalar previamente la herramienta cmake 2.8+, disponible para Linux, Windows y Mac, una vez
# instalada, hacemos:
#
# mkdir -p build
# cd build
# cmake ..
# make
########################################################################################################################
PROJECT(fpgalink1)
CMAKE_MINIMUM_REQUIRED (VERSION 2.8.6)
set (CMAKE_LEGACY_CYGWIN_WIN32 0)

set (FPGALINK1_VERSION_MAJOR 0)
set (FPGALINK1_VERSION_MINOR 1)
set (FPGALINK1_VERSION_MICRO 0)
set (FPGALINK1_VERSION_STRING ${FPGALINK1_VERSION_MAJOR}.${FPGALINK1_VERSION_MINOR}.${FPGALINK1_VERSION_MICRO})

set (fpgalink1_SRCS src/fpga_link1.cpp)
set (test_fpgalink1_SRCS src/main.cpp)

set (CMAKE_VERBOSE_MAKEFILE on)

# Directorios de ficheros cabecera y de bibliotecas (opciones -I y -L respectivamente)
include_directories (src/include)

# opciones de compilación (CFLAGS)
add_definitions ("-Wall -g")

add_library(fpgalink1 SHARED ${fpgalink1_SRCS})
set_target_properties(fpgalink1 PROPERTIES VERSION ${FPGALINK1_VERSION_STRING})

add_executable(test_fpgalink1 ${test_fpgalink1_SRCS})
target_link_libraries(test_fpgalink1 fpgalink1 m pthread rt)