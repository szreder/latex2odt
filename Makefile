.PHONY : clean
CXXFLAGS = -Wall -std=c++17 -fPIC
BIN = odtgen

$(BIN) : $(BIN).cpp
	g++ $(CXXFLAGS) -I /usr/include/qt5 -I /usr/include/qt5/QtCore -l Qt5Core odtgen.cpp -o $@

clean :
	rm -f $(BIN)
