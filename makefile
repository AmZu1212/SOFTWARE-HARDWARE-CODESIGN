# Compiler
CXX = g++
CXXFLAGS = -std=c++17 -O0 -mavx

# Targets
TARGETS = parallel.exe serial.exe validate.exe cache.exe

# Default rule
all: $(TARGETS)

# Build rules
parallel.exe: main_parallel.cpp
	$(CXX) $(CXXFLAGS) main_parallel.cpp -o parallel.exe

serial.exe: main_serial.cpp
	$(CXX) $(CXXFLAGS) main_serial.cpp -o serial.exe

validate.exe: main_validate.cpp
	$(CXX) $(CXXFLAGS) main_validate.cpp -o validate.exe

cache.exe: cache_trasher.cpp
	$(CXX) $(CXXFLAGS) cache_trasher.cpp -o cache.exe

# Clean rule
clean:
	rm -f $(TARGETS)
