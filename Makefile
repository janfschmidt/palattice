CC=g++
cflags = -Wall #-g -O0 #last 2 for valgrind
INSTALL_PATH=/usr/local/bin/
PROG_NAME=Bsupply
ALL_O=main.o ELSASpuren.o metadata.o timetag.o filenames.o resonances.o spectrum.o interpolate.o functionofpos.o field.o AccElements.o AccLattice.o


all: $(PROG_NAME) strom2kick new_strom2kick
.PHONY: all

$(PROG_NAME): $(ALL_O)
	$(CC) $(cflags) -o $@ $(ALL_O) -lgsl -lgslcblas -lm

main.o: main.cpp types.hpp spectrum.hpp  metadata.hpp timetag.hpp filenames.hpp resonances.hpp interpolate.hpp interpolate.hxx functionofpos.hpp functionofpos.hxx field.hpp AccElements.hpp AccLattice.hpp gitversion.hpp ELSASpuren.hpp
	$(CC) $(cflags) -c $<

timetag.o: timetag.cpp
	$(CC) $(cflags) -c $<
filenames.o: filenames.cpp filenames.hpp types.hpp
	$(CC) $(cflags) -c $<
resonances.o: resonances.cpp resonances.hpp types.hpp functionofpos.hpp functionofpos.hxx AccLattice.hpp spectrum.hpp
	$(CC) $(cflags) -c $<


spectrum.o: spectrum.cpp spectrum.hpp config.hpp
	$(CC) $(cflags) -c $<
interpolate.o: interpolate.cpp interpolate.hpp interpolate.hxx types.hpp
	$(CC) $(cflags) -c $<
functionofpos.o: functionofpos.cpp functionofpos.hpp functionofpos.hxx interpolate.hpp interpolate.hxx spectrum.hpp ELSASpuren.hpp types.hpp
	$(CC) $(cflags) -c $<
field.o: field.cpp field.hpp functionofpos.hpp functionofpos.hxx types.hpp AccLattice.hpp
	$(CC) $(cflags) -c $<
ELSASpuren.o: ELSASpuren.cpp types.hpp
	$(CC) $(cflags) -c $<
metadata.o: metadata.cpp gitversion.hpp
	$(CC) $(cflags) -c $<
AccElements.o: AccElements.cpp AccElements.hpp types.hpp
	$(CC) $(cflags) -c $<
AccLattice.o: AccLattice.cpp AccLattice.hpp AccElements.hpp ELSASpuren.hpp metadata.hpp config.hpp types.hpp
	$(CC) $(cflags) -c $<

gitversion.hpp: ../.git/HEAD ../.git/index
	echo "#ifndef __BSUPPLY__GITVERSION_HPP_" > $@
	echo "#define __BSUPPLY__GITVERSION_HPP_" >> $@
	echo "inline const std::string gitversion() {return \"$(shell git log -n 1 --date=iso --pretty=format:"%h from %ad")\";}" >> $@
	echo "#endif" >> $@


strom2kick: strom2kick.c
	gcc $(cflags) $< -lm -o $@

new_strom2kick: new_strom2kick.c
	gcc $(cflags) $< -lm -o $@


clean: 
	rm $(PROG_NAME) $(ALL_O) strom2kick new_strom2kick gitversion.hpp

install:
	install -m 755 -p -v $(PROG_NAME) $(INSTALL_PATH)
uninstall:
	rm -f $(INSTALL_PATH)/$(PROG_NAME)
