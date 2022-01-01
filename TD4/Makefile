#include Make_msys2.inc
#include Make_linux.inc
include Make_osx.inc

CXXFLAGS = -std=c++11 -fPIC
ifdef DEBUG
CXXFLAGS += -g -O0 -Wall -fbounds-check -pedantic -D_GLIBCXX_DEBUG -fopenmp
else
CXXFLAGS += -O3 -march=native -Wall 
endif

default:	help

all:	raytracer_tbb.exe


raytracer_tbb.exe:	raytracer_tbb.cpp
	$(CXX) $(CXXFLAGS) -o raytracer_tbb.exe raytracer_tbb.cpp $(LIB) -ltbb


help: 
	@echo "Available targets : "
	@echo "    all                      : compile all executables"
	@echo "    raytracer_tbb.exe    : compile Raytracer with tbb parallelism"
	@echo "Add DEBUG=yes to compile in debug"
	@echo "Configuration :"
	@echo "    CXX      :    $(CXX)"
	@echo "    CXXFLAGS :    $(CXXFLAGS)"

clean:
	@rm -f *.exe *~ 

