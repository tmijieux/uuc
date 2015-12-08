TARGET=uuc
LEX=lex
YACC=bison
CXXFLAGS=-Wall -ggdb -O0 -std=c++11 -fpermissive  -Wno-write-strings \
	`llvm-config --cxxflags`
LDFLAGS=-lLLVM-3.5 -lgc
CXX=c++
SRC= $(wildcard *.cpp) \
	grammar.cpp \
	scanner.cpp \

OBJ=$(patsubst %.cpp,%.o,$(SRC))


%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CFLAGS) -o $@ $^ $(LDFLAGS)

grammar.cpp: grammar.ypp
	$(YACC) -d -o $@ --defines=grammar.tab.h $^

grammar.tab.h: grammar.cpp

scanner.cpp: scanner.l grammar.tab.h
	$(LEX) -o $@ $<

clean:
	$(RM) $(OBJ) grammar.cpp scanner.cpp grammar.tab.h


