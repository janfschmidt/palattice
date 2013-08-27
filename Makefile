cflags = -Wall -g -O0 #last 2 for valgrind

all: Bsupply strom2kick new_strom2kick
.PHONY: all

Bsupply: main.o getfields.o getspectrum.o exportfile.o madximport.o ELSAimport.o metadata.o difference.o timetag.o filenames.o resonances.o fieldmap.o spectrum.o orbit.o interpolate.o field.o
	g++ $(cflags) -o Bsupply main.o getfields.o getspectrum.o exportfile.o madximport.o ELSAimport.o metadata.o difference.o timetag.o filenames.o resonances.o fieldmap.o spectrum.o orbit.o interpolate.o field.o -lgsl -lgslcblas -lm

main.o: main.cpp constants.hpp types.hpp orbit.hpp fieldmap.hpp spectrum.hpp getfields.hpp exportfile.hpp madximport.hpp metadata.hpp difference.hpp timetag.hpp filenames.hpp resonances.hpp interpolate.hpp interpolate.hxx functionofpos.hpp functionofpos.hxx field.hpp
	g++ $(cflags) -c main.cpp
getfields.o: getfields.cpp constants.hpp types.hpp resonances.hpp fieldmap.hpp orbit.hpp interpolate.hpp interpolate.hxx functionofpos.hpp functionofpos.hxx field.hpp
	g++ $(cflags) -c getfields.cpp
getspectrum.o: getspectrum.cpp constants.hpp types.hpp resonances.hpp fieldmap.hpp spectrum.hpp field.hpp
	g++ $(cflags) -c getspectrum.cpp
exportfile.o: exportfile.cpp constants.hpp types.hpp metadata.hpp spectrum.hpp
	g++ $(cflags) -c exportfile.cpp
madximport.o: madximport.cpp constants.hpp types.hpp orbit.hpp filenames.hpp functionofpos.hpp functionofpos.hxx
	g++ $(cflags) -c madximport.cpp
ELSAimport.o: ELSAimport.cpp constants.hpp types.hpp orbit.hpp functionofpos.hpp functionofpos.hxx
	g++ $(cflags) -c ELSAimport.cpp
metadata.o: metadata.cpp
	g++ $(cflags) -c metadata.cpp
difference.o: difference.cpp constants.hpp types.hpp ELSAimport.hpp fieldmap.hpp spectrum.hpp orbit.hpp interpolate.hpp interpolate.hxx functionofpos.hpp functionofpos.hxx
	g++ $(cflags) -c difference.cpp
timetag.o: timetag.cpp
	g++ $(cflags) -c timetag.cpp
filenames.o: filenames.cpp filenames.hpp
	g++ $(cflags) -c filenames.cpp
resonances.o: resonances.cpp resonances.hpp types.hpp
	g++ $(cflags) -c resonances.cpp
fieldmap.o: fieldmap.cpp fieldmap.hpp types.hpp
	g++ $(cflags) -c fieldmap.cpp
spectrum.o: spectrum.cpp spectrum.hpp
	g++ $(cflags) -c spectrum.cpp
orbit.o: orbit.cpp orbit.hpp types.hpp
	g++ $(cflags) -c orbit.cpp
interpolate.o: interpolate.cpp interpolate.hpp interpolate.hxx
	g++ $(cflags) -c interpolate.cpp
field.o: field.cpp field.hpp functionofpos.hpp functionofpos.hxx types.hpp
	g++ $(cflags) -c field.cpp



strom2kick: strom2kick.c
	gcc $(cflags) strom2kick.c -lm -o strom2kick

new_strom2kick: new_strom2kick.c
	gcc $(cflags) new_strom2kick.c -lm -o new_strom2kick


clean: 
	rm Bsupply main.o getfields.o getspectrum.o exportfile.o madximport.o ELSAimport.o metadata.o difference.o timetag.o filenames.o resonances.o fieldmap.o spectrum.o orbit.o interpolate.o field.o strom2kick new_strom2kick
