CC=g++
ccflags = -Wall #-g -O0 #last 2 for valgrind
INSTALL_PATH=/usr/local/

ALL=lattice interptwiss fields convertlattice

all: $(ALL)
.PHONY: all

lattice: lattice.cpp
	$(CC) $(ccflags) -o $@ $< -lpal
interptwiss: interptwiss.cpp
	$(CC) $(ccflags) -o $@ $< -lpal -lgsl
fields: fields.cpp
	$(CC) $(ccflags) -o $@ $< -lpal -lgsl


convertlattice: convertlattice.cpp
	$(CC) $(ccflags) -o $@ $< -lpal

clean:
	rm $(ALL) convertlattice_static

install:
	install -m 755 -p -v convertlattice $(INSTALL_PATH)/bin/

uninstall:
	rm -f $(INSTALL_PATH)/bin/convertlattice

static: convertlattice.cpp ../libpal.a
	$(CC) $(ccflags) -o convertlattice_static $^
