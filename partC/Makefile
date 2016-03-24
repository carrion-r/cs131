language=C++
prefix=/pkg/openmpi/1.10.2
exec_prefix=${prefix}
includedir=${prefix}/include
libdir=${exec_prefix}/lib
compiler_env=CXX
compiler_flags_env=CXXFLAGS
compiler=g++
compiler_flags=-pthread
linker_flags=    -Wl,-rpath -Wl,@{libdir} -Wl,--enable-new-dtags
libs=-lmpi_cxx -lmpi
dyn_lib_file=libmpi.so
CXX=mpic++
CPPFLAGS=-std=c++0x -Wall
PROGRAM=PartC
CXXFLAGS=-I${includedir} -pthread
LIBS= ${linker_flags} -L${libdir} ${libs}

PartC: partC.cpp
	$(CXX) $(CXXFLAGS) $(LIBS) $(CPPFLAGS) partC.cpp -o $(PROGRAM) 

clean:
	rm $(PROGRAM)

run: $(PROGRAM)
	./$(PROGRAM)

