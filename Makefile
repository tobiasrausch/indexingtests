DEBUG ?= 0
STATIC ?= 0

# Submodules
PWD = $(shell pwd)
SDSL_ROOT ?= ${PWD}/src/sdslLite

# Flags
CXX=g++
CXXFLAGS += -std=c++11 -O3 -DNDEBUG -I ${SDSL_ROOT}/include -pedantic -W -Wall
LDFLAGS += -L ${SDSL_ROOT}/lib -lsdsl -ldivsufsort -ldivsufsort64

# External sources
SDSLSOURCES = $(wildcard src/sdsl/lib/*.cpp)
IDXSOURCES = $(wildcard src/*.cpp) $(wildcard src/*.h)
PBASE=$(shell pwd)

# Targets
TARGETS = .sdsl src/index

all:   	$(TARGETS)

.sdsl: $(SDSLSOURCES)
	cd src/sdsl/ && ./install.sh ${PBASE}/src/sdslLite && cd ../../ && touch .sdsl

src/index: .sdsl ${IDXSOURCES}
	$(CXX) $(CXXFLAGS) $@.cpp -o $@ $(LDFLAGS)

clean:
	cd src/sdsl/ && ./uninstall.sh && cd ../../ && rm -rf src/sdslLite/
	rm -f $(TARGETS) $(TARGETS:=.o)
