CC=g++
cflags = -Wall #-g -O0 #last 2 for valgrind
INSTALL_PATH=/usr/local/bin/
PROG_NAME=Bsupply
ALL_O=main.o ELSAimport.o metadata.o timetag.o filenames.o resonances.o spectrum.o interpolate.o functionofpos.o field.o AccElements.o AccLattice.o


all: $(PROG_NAME) strom2kick new_strom2kick
.PHONY: all

$(PROG_NAME): $(ALL_O)
	$(CC) $(cflags) -o $(PROG_NAME) $(ALL_O) -lgsl -lgslcblas -lm

main.o: main.cpp constants.hpp types.hpp spectrum.hpp  metadata.hpp timetag.hpp filenames.hpp resonances.hpp interpolate.hpp interpolate.hxx functionofpos.hpp functionofpos.hxx field.hpp AccElements.hpp AccLattice.hpp gitversion.hpp
	$(CC) $(cflags) -c main.cpp
ELSAimport.o: ELSAimport.cpp constants.hpp types.hpp functionofpos.hpp functionofpos.hxx
	$(CC) $(cflags) -c ELSAimport.cpp
metadata.o: metadata.cpp gitversion.hpp
	$(CC) $(cflags) -c metadata.cpp
timetag.o: timetag.cpp
	$(CC) $(cflags) -c timetag.cpp
filenames.o: filenames.cpp filenames.hpp types.hpp
	$(CC) $(cflags) -c filenames.cpp
resonances.o: resonances.cpp resonances.hpp types.hpp functionofpos.hpp functionofpos.hxx AccLattice.hpp spectrum.hpp
	$(CC) $(cflags) -c resonances.cpp
spectrum.o: spectrum.cpp spectrum.hpp constants.hpp
	$(CC) $(cflags) -c spectrum.cpp
interpolate.o: interpolate.cpp interpolate.hpp interpolate.hxx types.hpp
	$(CC) $(cflags) -c interpolate.cpp
functionofpos.o: functionofpos.cpp functionofpos.hpp functionofpos.hxx interpolate.hpp interpolate.hxx spectrum.hpp filenames.hpp types.hpp
	$(CC) $(cflags) -c functionofpos.cpp
field.o: field.cpp field.hpp functionofpos.hpp functionofpos.hxx types.hpp AccLattice.hpp
	$(CC) $(cflags) -c field.cpp
AccElements.o: AccElements.cpp AccElements.hpp types.hpp
	$(CC) $(cflags) -c AccElements.cpp
AccLattice.o: AccLattice.cpp AccLattice.hpp AccElements.hpp
	$(CC) $(cflags) -c AccLattice.cpp

gitversion.hpp: ../.git/HEAD ../.git/index
	echo "#ifndef __BSUPPLY__GITVERSION_HPP_" > $@
	echo "#define __BSUPPLY__GITVERSION_HPP_" >> $@
	echo "inline const string gitversion() {return \"$(shell git log -n 1 --date=iso --pretty=format:"%h from %ad")\";}" >> $@
	echo "#endif" >> $@


strom2kick: strom2kick.c
	gcc $(cflags) strom2kick.c -lm -o strom2kick

new_strom2kick: new_strom2kick.c
	gcc $(cflags) new_strom2kick.c -lm -o new_strom2kick


clean: 
	rm $(PROG_NAME) $(ALL_O) strom2kick new_strom2kick gitversion.hpp

install:
	install -m 755 -p -v $(PROG_NAME) $(INSTALL_PATH)
uninstall:
	rm -f $(INSTALL_PATH)/$(PROG_NAME)
