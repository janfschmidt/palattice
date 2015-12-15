CC=g++
ccflags = -std=c++11 -Wall #-g -O0 #last 2 for valgrind
INSTALL_PATH=/usr/local/

SDDSLINKPATH= -L$(firstword $(wildcard /usr/lib/x86_64-linux-gnu/SDDS /usr/lib/i386-linux-gnu/SDDS /usr/lib64/SDDS /usr/lib/SDDS))

ALL=lattice interptwiss fields convertlattice

all: $(ALL)
.PHONY: all

lattice: lattice.cpp
	$(CC) $(ccflags) -o $@ $< -lpalattice -lz
interptwiss: interptwiss.cpp
	$(CC) $(ccflags) -o $@ $< -lpalattice -lgsl -lz
fields: fields.cpp
	$(CC) $(ccflags) -o $@ $< -lpalattice -lgsl -lz


convertlattice: convertlattice.cpp
	$(CC) $(ccflags) -o $@ $< -lpalattice -lz

clean:
	rm $(ALL) convertlattice_static

install:
	install -m 755 -p -v convertlattice $(INSTALL_PATH)/bin/

uninstall:
	rm -f $(INSTALL_PATH)/bin/convertlattice

static: convertlattice.cpp ../libpalattice.a
	$(CC) $(ccflags) -o convertlattice_static $^
