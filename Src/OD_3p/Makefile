FILES = plotVdMOrbitDrift_3p


all: $(FILES)


$(FILES): % : %.cc
	@echo "Building $@ ... "

	g++ `root-config --libs --cflags` -o $@ $@.cc

	@echo "Done"

