
CPPFILES=$(wildcard *.cpp)
HPPFILES=$(wildcard *.hpp)
OFILES_DEBUG=$(patsubst %.cpp,bin/debug/%.o,$(CPPFILES))
OFILES_PROD=$(patsubst %.cpp,bin/prod/%.o,$(CPPFILES))

GCCOPTS=-Wall
DEBUGOPTS=-DDEBUG -g
PRODOPTS=-O2

all: bin/midialign-debug bin/midialign

bin/midialign-debug: $(OFILES_DEBUG)
	g++ $(GCCOPTS) $(DEBUGOPTS) $^ -o $@

bin/debug/%.o: %.cpp $(HPPFILES) Makefile
	mkdir -p bin/debug && g++ -c $(GCCOPTS) $(DEBUGOPTS) $< -o $@

bin/midialign: $(OFILES_PROD)
	g++ $(GCCOPTS) $(PRODOPTS) $^ -o $@

bin/prod/%.o: %.cpp $(HPPFILES) Makefile
	mkdir -p bin/prod && g++ -c $(GCCOPTS) $(PRODOPTS) $< -o $@
