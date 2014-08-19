CC=g++
ccflags = -Wall #-g -O0 #last 2 for valgrind
INSTALL_PATH=/usr/local/bin/
PROG_NAME=Bsupply
ALL_O=main.o ELSASpuren.o Metadata.o timetag.o filenames.o resonances.o Spectrum.o Interpolate.o FunctionOfPos.o Field.o AccElements.o AccLattice.o


all: $(PROG_NAME) strom2kick new_strom2kick
.PHONY: all

$(PROG_NAME): $(ALL_O)
	$(CC) $(ccflags) -o $@ $(ALL_O) -lgsl -lgslcblas -lm

main.o: main.cpp types.hpp Spectrum.hpp  Metadata.hpp timetag.hpp filenames.hpp resonances.hpp Interpolate.hpp Interpolate.hxx FunctionOfPos.hpp FunctionOfPos.hxx Field.hpp AccElements.hpp AccLattice.hpp bsupplyGitversion.hpp libpalGitversion.hpp ELSASpuren.hpp
	$(CC) $(ccflags) -c $<

timetag.o: timetag.cpp
	$(CC) $(ccflags) -c $<
filenames.o: filenames.cpp filenames.hpp types.hpp
	$(CC) $(ccflags) -c $<
resonances.o: resonances.cpp resonances.hpp types.hpp FunctionOfPos.hpp FunctionOfPos.hxx AccLattice.hpp Spectrum.hpp
	$(CC) $(ccflags) -c $<


Spectrum.o: Spectrum.cpp Spectrum.hpp config.hpp
	$(CC) $(ccflags) -c $<
Interpolate.o: Interpolate.cpp Interpolate.hpp Interpolate.hxx types.hpp
	$(CC) $(ccflags) -c $<
FunctionOfPos.o: FunctionOfPos.cpp FunctionOfPos.hpp FunctionOfPos.hxx Interpolate.hpp Interpolate.hxx Spectrum.hpp ELSASpuren.hpp types.hpp
	$(CC) $(ccflags) -c $<
Field.o: Field.cpp Field.hpp FunctionOfPos.hpp FunctionOfPos.hxx types.hpp AccLattice.hpp
	$(CC) $(ccflags) -c $<
ELSASpuren.o: ELSASpuren.cpp types.hpp
	$(CC) $(ccflags) -c $<
Metadata.o: Metadata.cpp libpalGitversion.hpp
	$(CC) $(ccflags) -c $<
AccElements.o: AccElements.cpp AccElements.hpp types.hpp
	$(CC) $(ccflags) -c $<
AccLattice.o: AccLattice.cpp AccLattice.hpp AccElements.hpp ELSASpuren.hpp Metadata.hpp config.hpp types.hpp
	$(CC) $(ccflags) -c $<

bsupplyGitversion.hpp: ../.git/HEAD ../.git/index
	echo "#ifndef __BSUPPLY__GITVERSION_HPP_" > $@
	echo "#define __BSUPPLY__GITVERSION_HPP_" >> $@
	echo "inline const std::string bsupplyGitversion() {return \"$(shell git log -n 1 --date=iso --pretty=format:"%h from %ad")\";}" >> $@
	echo "#endif" >> $@

libpalGitversion.hpp: ../.git/HEAD ../.git/index
	echo "#ifndef __LIBPAL__GITVERSION_HPP_" > $@
	echo "#define __LIBPAL__GITVERSION_HPP_" >> $@
	echo "inline const std::string libpalGitversion() {return \"$(shell git log -n 1 --date=iso --pretty=format:"%h from %ad")\";}" >> $@
	echo "#endif" >> $@


strom2kick: strom2kick.c
	gcc $(ccflags) $< -lm -o $@

new_strom2kick: new_strom2kick.c
	gcc $(ccflags) $< -lm -o $@


clean: 
	rm $(PROG_NAME) $(ALL_O) strom2kick new_strom2kick bsupplyGitversion.hpp libpalGitversion.hpp

install:
	install -m 755 -p -v $(PROG_NAME) $(INSTALL_PATH)
uninstall:
	rm -f $(INSTALL_PATH)/$(PROG_NAME)
