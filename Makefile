CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
SRC = src/prog.cpp
OUT = bin/run

all: $(OUT)
	./$(OUT)

$(OUT): $(SRC)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $< -o $@

run: $(OUT)
	./$(OUT)

clean: 
	rm -r bin/*
