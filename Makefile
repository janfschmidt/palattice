CC=g++
ccflags = -Wall #-g -O0 #last 2 for valgrind
INSTALL_PATH=/usr/local/
PROG_NAME=Bsupply
ALL_O=main.o timetag.o filenames.o resonances.o


all: $(PROG_NAME) strom2kick new_strom2kick
.PHONY: all

$(PROG_NAME): $(ALL_O)
	$(CC) $(ccflags) -L./libpal -o $@ $(ALL_O) -lpal -lgsl -lgslcblas -lm

main.o: main.cpp timetag.hpp filenames.hpp resonances.hpp bsupplyGitversion.hpp
	$(CC) $(ccflags) -c $<

timetag.o: timetag.cpp
	$(CC) $(ccflags) -c $<
filenames.o: filenames.cpp filenames.hpp
	$(CC) $(ccflags) -c $<
resonances.o: resonances.cpp resonances.hpp
	$(CC) $(ccflags) -c $<

bsupplyGitversion.hpp: ../.git/HEAD ../.git/index
	echo "#ifndef __BSUPPLY__GITVERSION_HPP_" > $@
	echo "#define __BSUPPLY__GITVERSION_HPP_" >> $@
	echo "inline const std::string bsupplyGitversion() {return \"$(shell git log -n 1 --date=iso --pretty=format:"%h from %ad")\";}" >> $@
	echo "#endif" >> $@

strom2kick: strom2kick.c
	gcc $(ccflags) $< -lm -o $@

new_strom2kick: new_strom2kick.c
	gcc $(ccflags) $< -lm -o $@


clean: 
	rm $(PROG_NAME) $(ALL_O) strom2kick new_strom2kick bsupplyGitversion.hpp

install:
	install -m 755 -p -D -v $(PROG_NAME) $(INSTALL_PATH)/bin/$(PROG_NAME)
uninstall:
	rm -f $(INSTALL_PATH)/bin/$(PROG_NAME)
