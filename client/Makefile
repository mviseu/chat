IDIR=./include

CXX=clang++
CXXFLAGS=-std=c++17 -Wall -Wextra -Werror
CPPFLAGS= -Wall -O2 -I$(IDIR) -fPIC

_DEPS:=Connect.h
DEPS:=$(patsubst %,$(IDIR)/%,$(_DEPS))


SHARED:=libclient.so
LDFLAGSSHARED += -shared
LDFLAGSEXE += -L. $(SHARED)


OBJ = Connect.o
MAIN=main.o
EXE=main

all:  $(OBJ) $(SHARED) $(MAIN) $(EXE)


$(EXE): $(MAIN)
	$(CXX) $(LDFLAGSEXE) -o $@ $^


$(SHARED): $(OBJ)
	$(CXX) $(LDFLAGSSHARED) -o $@ $^



%.o: src/%.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<


.PHONY:clean

clean:
	rm -rf $(EXE) *.o *.so main*
