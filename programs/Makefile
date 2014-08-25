CC=g++
ccflags = -Wall #-g -O0 #last 2 for valgrind
INSTALL_PATH=/usr/local/

ALL=convertlattice

all: $(ALL)
.PHONY: all

convertlattice: convertlattice.cpp
	$(CC) $(ccflags) -o $@ $< -lpal

clean:
	rm $(ALL)

install:
	install -m 755 -p -v convertlattice $(INSTALL_PATH)/bin/

uninstall:
	rm -f $(INSTALL_PATH)/bin/convertlattice
