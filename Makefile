CC=g++
ccflags = -Wall -fPIC #-g -O0 #last 2 for valgrind
LIB_NAME=libpal
Vmajor=1
Vminor=0.0
INSTALL_PATH=/usr/local/

ALL_O=Interpolate.o Metadata.o ELSASpuren.o FunctionOfPos.o Field.o AccElements.o AccLattice.o Spectrum.o
LIB_FILE=$(LIB_NAME).so


all: $(LIB_NAME)
.PHONY: all

$(LIB_NAME): $(ALL_O)
	$(CC) $(ccflags) -shared -Wl,-soname,$(LIB_FILE).$(Vmajor) -o $(LIB_FILE).$(Vmajor).$(Vminor)  $(ALL_O) -lgsl -lgslcblas -lm

Spectrum.o: Spectrum.cpp Spectrum.hpp config.hpp
	$(CC) $(ccflags) -c $<
Interpolate.o: Interpolate.cpp Interpolate.hpp Interpolate.hxx types.hpp
	$(CC) $(ccflags) -c $<
FunctionOfPos.o: FunctionOfPos.cpp FunctionOfPos.hpp FunctionOfPos.hxx Interpolate.hpp Interpolate.hxx Spectrum.hpp ELSASpuren.hpp types.hpp config.hpp
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

libpalGitversion.hpp: .git/HEAD .git/index
	echo "#ifndef __LIBPAL__GITVERSION_HPP_" > $@
	echo "#define __LIBPAL__GITVERSION_HPP_" >> $@
	echo "namespace pal {" >> $@
	echo "inline const std::string libpalGitversion() {return \"$(Vmajor).$(Vminor), git ID $(shell git log -n 1 --date=iso --pretty=format:"%h from %ad")\";} }" >> $@
	echo "#endif" >> $@

clean: 
	rm $(LIB_FILE)* $(ALL_O) libpalGitversion.hpp

install: $(LIB_FILE).$(Vmajor).$(Vminor)
	install -m 755 -p -v $< $(INSTALL_PATH)/lib/
	ln -sf $< $(INSTALL_PATH)/lib/$(LIB_FILE).$(Vmajor)
	ln -sf $< $(INSTALL_PATH)/lib/$(LIB_FILE)
	mkdir -p $(INSTALL_PATH)/include/$(LIB_NAME)
	install -m 664 -p -v *.hpp $(INSTALL_PATH)/include/$(LIB_NAME)/
	install -m 664 -p -v *.hxx $(INSTALL_PATH)/include/$(LIB_NAME)/

uninstall:
	rm -f $(INSTALL_PATH)/lib/$(LIB_FILE)*
	rm -rf $(INSTALL_PATH)/include/$(LIB_NAME)*
