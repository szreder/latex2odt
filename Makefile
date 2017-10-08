.PHONY : clean
CXXFLAGS = -Wall -std=c++17 -fPIC

latex2odt : tex_parser.cpp
	g++ $(CXXFLAGS) -I /usr/include/qt5 -I /usr/include/qt5/QtCore -l Qt5Core tex_parser.cpp -o latex2odt

clean :
	rm -f latex2odt
