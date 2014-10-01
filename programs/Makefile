CC=g++
ccflags = -Wall #-g -O0 #last 2 for valgrind
INSTALL_PATH=/usr/local/

ALL=convertlattice interptwiss

all: $(ALL)
.PHONY: all

convertlattice: convertlattice.cpp
	$(CC) $(ccflags) -o $@ $< -lpal

interptwiss: interptwiss.cpp
	$(CC) $(ccflags) -o $@ $< -lpal -lgsl

clean:
	rm $(ALL)

install:
	install -m 755 -p -v convertlattice $(INSTALL_PATH)/bin/

uninstall:
	rm -f $(INSTALL_PATH)/bin/convertlattice
