CC=g++
lib_ccflags = -Wall -fPIC #-g -O0 #last 2 for valgrind
LIB_NAME=libpal.so
INSTALL_PATH=/usr/local/

LIB_ALL_O=Interpolate.o Metadata.o ELSASpuren.o FunctionOfPos.o Field.o AccElements.o AccLattice.o Spectrum.o


all: $(LIB_NAME)
.PHONY: all

$(LIB_NAME): $(LIB_ALL_O)
	$(CC) $(lib_ccflags) -shared -o $@ $(LIB_ALL_O) -lgsl -lgslcblas -lm

Spectrum.o: Spectrum.cpp Spectrum.hpp config.hpp
	$(CC) $(lib_ccflags) -c $<
Interpolate.o: Interpolate.cpp Interpolate.hpp Interpolate.hxx types.hpp
	$(CC) $(lib_ccflags) -c $<
FunctionOfPos.o: FunctionOfPos.cpp FunctionOfPos.hpp FunctionOfPos.hxx Interpolate.hpp Interpolate.hxx Spectrum.hpp ELSASpuren.hpp types.hpp config.hpp
	$(CC) $(lib_ccflags) -c $<
Field.o: Field.cpp Field.hpp FunctionOfPos.hpp FunctionOfPos.hxx types.hpp AccLattice.hpp
	$(CC) $(lib_ccflags) -c $<
ELSASpuren.o: ELSASpuren.cpp types.hpp
	$(CC) $(lib_ccflags) -c $<
Metadata.o: Metadata.cpp libpalGitversion.hpp
	$(CC) $(lib_ccflags) -c $<
AccElements.o: AccElements.cpp AccElements.hpp types.hpp
	$(CC) $(lib_ccflags) -c $<
AccLattice.o: AccLattice.cpp AccLattice.hpp AccElements.hpp ELSASpuren.hpp Metadata.hpp config.hpp types.hpp
	$(CC) $(lib_ccflags) -c $<

libpalGitversion.hpp: ../../.git/HEAD ../../.git/index
	echo "#ifndef __LIBPAL__GITVERSION_HPP_" > $@
	echo "#define __LIBPAL__GITVERSION_HPP_" >> $@
	echo "namespace pal {" >> $@
	echo "inline const std::string libpalGitversion() {return \"$(shell git log -n 1 --date=iso --pretty=format:"%h from %ad")\";} }" >> $@
	echo "#endif" >> $@

clean: 
	rm $(LIB_NAME) $(LIB_ALL_O) libpalGitversion.hpp

install:
	install -m 755 -p -D -v $(LIB_NAME) $(INSTALL_PATH)/lib/$(LIB_NAME)
uninstall:
	rm -f $(INSTALL_PATH)/lib/$(LIB_NAME)
