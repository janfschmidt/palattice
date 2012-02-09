cflags = -Wall -g -O0 #last 2 for valgrind

Bsupply: main.o getorbit.o getfields.o getspectrum.o exportfile.o madximport.o ELSAimport.o metadata.o
	g++ $(cflags) -lgsl -lgslcblas -lm -o Bsupply main.o getorbit.o getfields.o getspectrum.o exportfile.o madximport.o ELSAimport.o metadata.o

main.o: main.cpp constants.hpp types.hpp getorbit.hpp getfields.hpp exportfile.hpp madximport.hpp metadata.hpp
	g++ $(cflags) -c main.cpp
getorbit.o: getorbit.cpp constants.hpp types.hpp
	g++ $(cflags) -c getorbit.cpp
getfields.o: getfields.cpp constants.hpp types.hpp
	g++ $(cflags) -c getfields.cpp
getspectrum.o: getspectrum.cpp constants.hpp types.hpp
	g++ $(cflags) -c getspectrum.cpp
exportfile.o: exportfile.cpp constants.hpp types.hpp metadata.hpp
	g++ $(cflags) -c exportfile.cpp
madximport.o: madximport.cpp constants.hpp types.hpp
	g++ $(cflags) -c madximport.cpp
ELSAimport.o: ELSAimport.cpp constants.hpp types.hpp
	g++ $(cflags) -c ELSAimport.cpp
metadata.o: metadata.cpp
	g++ $(cflags) -c metadata.cpp

clean: 
	rm Bsupply main.o getorbit.o getfields.o getspectrum.o exportfile.o madximport.o ELSAimport.o metadata.o
