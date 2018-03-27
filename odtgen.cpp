#include <memory>
#include <QtCore>

#include "Parser/LaTeXParser.hpp"
#include "Parser/MarkdownParser.hpp"
#include "Document.hpp"

int main(int argc, char *argv[])
{
	QTextStream input{stdin};
	QString data = input.readAll();

	std::unique_ptr <Parser> parser;

	if (argc > 1 && strcmp(argv[1], "-M") == 0)
		parser = std::make_unique<MarkdownParser>();
	else
		parser = std::make_unique<LaTeXParser>();

	auto doc = parser->parse(data);
	if (doc)
		doc->output();

	return 0;
}
