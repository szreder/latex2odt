.PHONY : clean
CXXFLAGS = -Wall -std=c++17 -fPIC
BIN = odtgen
OBJS = AST.o Document.o odtgen.o Markup/Cpp.o Strings.o XmlGen.o

$(BIN) : $(OBJS)
	g++ -o $@ $^ -l Qt5Core

%.o : %.cpp
	g++ $(CXXFLAGS) -c $^ -o $@ -I . -I /usr/include/qt5 -I /usr/include/qt5/QtCore

clean :
	rm -f $(BIN) $(OBJS)
