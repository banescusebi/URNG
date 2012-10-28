# MAKEFILE for linux GCC
#
# Sebastian Banescu

# Paths
PAPI_DIR=/usr/local
INSTALL_DIR=/usr/local
LIBRARY_PATH=${INSTALL_DIR}/lib
C_INCLUDE_PATH=${PAPI_DIR}/include

# Flags 
CFLAGS=-fopenmp -g -I${C_INCLUDE_PATH} 

# Libraries
LDLIBS=${LIBRARY_PATH}/libpapi.a

# Compilation target
urng-openmp: urng-openmp.c 

urng-pthreads: urng-pthreads.c
