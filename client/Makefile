IDIR = ./include
SRCDIR = src

CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror
CPPFLAGS = -I$(IDIR) -fPIC


SHARED = libclient.so
LDFLAGSSHARED += -shared
LDFLAGSEXE += -L. $(SHARED)

EXE = main
MAIN = $(EXE:=.o)

SRCFILES = $(filter-out ./$(SRCDIR)/$(EXE).cpp, $(wildcard ./$(SRCDIR)/*.cpp))
OBJLIB = $(patsubst ./$(SRCDIR)/%.cpp, %.o, $(SRCFILES))



all: $(SHARED) $(EXE)


$(EXE): $(MAIN)
	$(CXX) $(LDFLAGSEXE) -o $@ $^


$(SHARED): $(OBJLIB)
	$(CXX) $(LDFLAGSSHARED) -o $@ $^


%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<


.PHONY:clean


clean:
	rm -rf $(EXE) *.o *.so
