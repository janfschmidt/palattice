CC=g++
ccflags = -std=c++11 -Wall -fPIC -g #-O0
LIB_NAME=libpalattice
Vmajor=3.7
Vminor=0
INSTALL_PATH=/usr/local/

ALL_O=Interpolate.o Metadata.o ELSASpuren.o FunctionOfPos.o Field.o AccElements.o AccLattice.o Spectrum.o SimTools.o ResStrengths.o
LIB_FILE=$(LIB_NAME).so
SIMTOOL_PATH=$(INSTALL_PATH)/lib/libpalattice_simTools

SDDSLINKPATH=-L$(firstword $(wildcard /usr/lib/x86_64-linux-gnu/SDDS /usr/lib/i386-linux-gnu/SDDS /usr/lib64/SDDS /usr/lib/SDDS))
SDDSFLAGS =-lmdbcommon -lSDDS1 -lmdblib -llzma -lz
LINKFLAGS=-lgsl -lgslcblas -lm $(SDDSFLAGS)
GTESTFLAGS=-lpthread $(LINKFLAGS)


$(LIB_NAME): gitversion.hpp simToolPath.hpp $(ALL_O)
	$(CC) $(ccflags) -shared -Wl,-soname,$(LIB_FILE).$(Vmajor) -o $(LIB_FILE).$(Vmajor).$(Vminor)  $(ALL_O) $(SDDSLINKPATH) $(LINKFLAGS)
	ln -sf $(LIB_FILE).$(Vmajor).$(Vminor) $(LIB_FILE).$(Vmajor)
	ln -sf $(LIB_FILE).$(Vmajor).$(Vminor) $(LIB_FILE)

programs: 
	make -C ./programs

tests:
	make -C ./tests

.PHONY: programs tests

Spectrum.o: Spectrum.cpp Spectrum.hpp config.hpp
	$(CC) $(ccflags) -c $<
Interpolate.o: Interpolate.cpp Interpolate.hpp Interpolate.hxx types.hpp Metadata.hpp Metadata.hxx
	$(CC) $(ccflags) -c $<
FunctionOfPos.o: FunctionOfPos.cpp FunctionOfPos.hpp FunctionOfPos.hxx Interpolate.hpp Interpolate.hxx Spectrum.hpp ELSASpuren.hpp types.hpp config.hpp SimTools.hpp
	$(CC) $(ccflags) -c $<
Field.o: Field.cpp Field.hpp FunctionOfPos.hpp FunctionOfPos.hxx types.hpp AccLattice.hpp
	$(CC) $(ccflags) -c $<
ELSASpuren.o: ELSASpuren.cpp types.hpp
	$(CC) $(ccflags) -c $<
Metadata.o: Metadata.cpp Metadata.hpp Metadata.hxx gitversion.hpp SimTools.hpp
	$(CC) $(ccflags) -c $<
AccElements.o: AccElements.cpp AccElements.hpp types.hpp SimTools.hpp config.hpp
	$(CC) $(ccflags) -c $<
AccLattice.o: AccLattice.cpp AccLattice.hpp AccElements.hpp ELSASpuren.hpp Metadata.hpp Metadata.hxx config.hpp types.hpp SimTools.hpp
	$(CC) $(ccflags) -c $<
SimTools.o: SimTools.cpp SimTools.hpp types.hpp config.hpp simToolPath.hpp
	$(CC) $(ccflags) -c $<
ResStrengths.o: ResStrengths.cpp ResStrengths.hpp AccLattice.hpp FunctionOfPos.hpp FunctionOfPos.hxx Metadata.hpp Metadata.hxx
	$(CC) $(ccflags) -c $<

gitversion.hpp: Makefile .git/HEAD .git/index
	echo "#ifndef __LIBPALATTICE__GITVERSION_HPP_" > $@
	echo "#define __LIBPALATTICE__GITVERSION_HPP_" >> $@
	echo "namespace pal {" >> $@
	echo "inline const std::string gitversion() {return \"$(Vmajor).$(Vminor), git ID $(shell git log -n 1 --date=iso --pretty=format:"%h from %ad")\";} }" >> $@
	echo "#endif" >> $@

simToolPath.hpp: Makefile
	echo "#ifndef __LIBPALATTICE__SIMTOOLPATH_HPP_" > $@
	echo "#define __LIBPALATTICE__SIMTOOLPATH_HPP_" >> $@
	echo "namespace pal {" >> $@
	echo "inline const std::string simToolPath() {return \"$(SIMTOOL_PATH)\";} }" >> $@
	echo "#endif" >> $@


clean: 
	rm $(LIB_FILE)* $(ALL_O) $(LIB_NAME).a gitversion.hpp simToolPath.hpp
	make clean -C ./programs
	make clean -C ./tests

install: $(LIB_FILE).$(Vmajor).$(Vminor)
	install -m 755 -p -v $< $(INSTALL_PATH)/lib/                     #library
	ln -sf $< $(INSTALL_PATH)/lib/$(LIB_FILE).$(Vmajor)
	ln -sf $< $(INSTALL_PATH)/lib/$(LIB_FILE)
	mkdir -p $(INSTALL_PATH)/include/$(LIB_NAME)                     #includes
	install -m 664 -p -v *.hpp $(INSTALL_PATH)/include/$(LIB_NAME)/
	install -m 664 -p -v *.hxx $(INSTALL_PATH)/include/$(LIB_NAME)/
	mkdir -p $(SIMTOOL_PATH)                                         #simTool files
	install -m 664 -p -v simTools/*.madx $(SIMTOOL_PATH)
	install -m 664 -p -v simTools/*.ele $(SIMTOOL_PATH)
	install -m 755 -p -v simTools/elegant2libpalattice.sh $(INSTALL_PATH)/bin/elegant2libpalattice
	ldconfig

install_programs:
	make install -C ./programs

uninstall:
	rm -f $(INSTALL_PATH)/lib/$(LIB_FILE)*
	rm -rf $(INSTALL_PATH)/include/$(LIB_NAME)*
	rm -rf $(SIMTOOL_PATH)
	make uninstall -C ./programs

static:
	ar rcs $(LIB_NAME).a $(ALL_O)
