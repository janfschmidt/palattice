CC=g++
ccflags = -Wall #-g -O0 #last 2 for valgrind
INSTALL_PATH=/usr/local/

ALL=lattice interptwiss fields convertlattice

all: $(ALL)
.PHONY: all

lattice: lattice.cpp
	$(CC) $(ccflags) -o $@ $< -lpalattice
interptwiss: interptwiss.cpp
	$(CC) $(ccflags) -o $@ $< -lpalattice -lgsl
fields: fields.cpp
	$(CC) $(ccflags) -o $@ $< -lpalattice -lgsl


convertlattice: convertlattice.cpp
	$(CC) $(ccflags) -o $@ $< -lpalattice

clean:
	rm $(ALL) convertlattice_static

install:
	install -m 755 -p -v convertlattice $(INSTALL_PATH)/bin/

uninstall:
	rm -f $(INSTALL_PATH)/bin/convertlattice

static: convertlattice.cpp ../libpalattice.a
	$(CC) $(ccflags) -o convertlattice_static $^
