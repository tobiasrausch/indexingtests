DEBUG ?= 0
STATIC ?= 0

# Submodules
PWD = $(shell pwd)
SDSL_ROOT ?= ${PWD}/src/sdslLite
SEQTK_ROOT ?= ${PWD}/src/htslib/

# Flags
CXX=g++
CXXFLAGS += -std=c++11 -O3 -DNDEBUG -I ${SEQTK_ROOT} -I ${SDSL_ROOT}/include -pedantic -W -Wall
LDFLAGS += -L ${SDSL_ROOT}/lib -lsdsl -ldivsufsort -ldivsufsort64 -L ${SEQTK_ROOT}

ifeq (${STATIC}, 1)
	LDFLAGS += -static -static-libgcc -pthread -lhts -lz
else
	LDFLAGS += -lhts -lz -Wl,-rpath,${SEQTK_ROOT}
endif

ifeq (${DEBUG}, 1)
	CXXFLAGS += -g -O0 -fno-inline -DDEBUG
else ifeq (${DEBUG}, 2)
	CXXFLAGS += -g -O0 -fno-inline -DPROFILE
	LDFLAGS += -lprofiler -ltcmalloc
else
	CXXFLAGS += -O3 -fno-tree-vectorize -DNDEBUG
endif

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
