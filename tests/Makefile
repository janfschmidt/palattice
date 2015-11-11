CC=g++
ccflags = -std=c++11 -Wall -fPIC -g

GTEST_DIR=/home/thiry/dev/gtest-1.7.0

SDDSLINKPATH=-L /home/control/epics/extensions/lib/linux-x86_64/ -L /home/control/epics/extensions/src/SDDS/SDDSlib/O.linux-x86_64/
SDDSFLAGS =-lSDDS1c -lz -lmdbcommon -lmatlib -lfftpack -lnnetwork -lcsa -lrpnlib  -lmdblib -lmdbmth
#SDDSLINKPATH=-L$(firstword $(wildcard /usr/lib/x86_64-linux-gnu/SDDS /usr/lib/i386-linux-gnu/SDDS /usr/lib64/SDDS /usr/lib/SDDS))
#SDDSFLAGS =-lmdbcommon -lSDDS1 -lmdblib -llzma -lz
LINKFLAGS=-lpthread -lgsl -lgslcblas -lm $(SDDSFLAGS)



tests: test-sdds


libgtest.a:
	$(CC) -I${GTEST_DIR}/include -I${GTEST_DIR} -c ${GTEST_DIR}/src/gtest-all.cc
	ar -rv $@ gtest-all.o

test-sdds: test-sdds.cpp libgtest.a
	$(CC) $(ccflags) $^ -o $@ -I$(GTEST_DIR)/include $(SDDSLINKPATH) -I../ -L../ $(LINKFLAGS) -Wl,-rpath,../ -lpalattice

clean:
	rm test-sdds gtest-all.o libgtest.a
