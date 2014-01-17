cflags = -Wall #-g -O0 #last 2 for valgrind

all: Bsupply strom2kick new_strom2kick
.PHONY: all

Bsupply: main.o ELSAimport.o metadata.o timetag.o filenames.o resonances.o spectrum.o interpolate.o functionofpos.o field.o AccElements.o AccLattice.o
	g++ $(cflags) -o Bsupply main.o ELSAimport.o metadata.o timetag.o filenames.o resonances.o spectrum.o interpolate.o functionofpos.o field.o AccElements.o AccLattice.o -lgsl -lgslcblas -lm

main.o: main.cpp constants.hpp types.hpp spectrum.hpp  metadata.hpp timetag.hpp filenames.hpp resonances.hpp interpolate.hpp interpolate.hxx functionofpos.hpp functionofpos.hxx field.hpp AccElements.hpp AccLattice.hpp
	g++ $(cflags) -c main.cpp
ELSAimport.o: ELSAimport.cpp constants.hpp types.hpp functionofpos.hpp functionofpos.hxx
	g++ $(cflags) -c ELSAimport.cpp
metadata.o: metadata.cpp
	g++ $(cflags) -c metadata.cpp
timetag.o: timetag.cpp
	g++ $(cflags) -c timetag.cpp
filenames.o: filenames.cpp filenames.hpp
	g++ $(cflags) -c filenames.cpp
resonances.o: resonances.cpp resonances.hpp types.hpp functionofpos.hpp functionofpos.hxx AccLattice.hpp spectrum.hpp
	g++ $(cflags) -c resonances.cpp
spectrum.o: spectrum.cpp spectrum.hpp
	g++ $(cflags) -c spectrum.cpp
interpolate.o: interpolate.cpp interpolate.hpp interpolate.hxx
	g++ $(cflags) -c interpolate.cpp
functionofpos.o: functionofpos.cpp functionofpos.hpp functionofpos.hxx
	g++ $(cflags) -c functionofpos.cpp
field.o: field.cpp field.hpp functionofpos.hpp functionofpos.hxx types.hpp AccLattice.hpp
	g++ $(cflags) -c field.cpp
AccElements.o: AccElements.cpp AccElements.hpp types.hpp
	g++ $(cflags) -c AccElements.cpp
AccLattice.o: AccLattice.cpp AccLattice.hpp AccElements.hpp
	g++ $(cflags) -c AccLattice.cpp



strom2kick: strom2kick.c
	gcc $(cflags) strom2kick.c -lm -o strom2kick

new_strom2kick: new_strom2kick.c
	gcc $(cflags) new_strom2kick.c -lm -o new_strom2kick


clean: 
	rm Bsupply main.o ELSAimport.o metadata.o timetag.o filenames.o resonances.o spectrum.o interpolate.o functionofpos.o field.o AccElements.o AccLattice.o strom2kick new_strom2kick
