#include <QtCore>

#include "Markup/Cpp.hpp"
#include "Markup/Highlight.hpp"
#include "AST.hpp"
#include "Document.hpp"
#include "Strings.hpp"
#include "XmlGen.hpp"

int main(int argc, char *argv[])
{
	QTextStream input{stdin};
	QString data = input.readAll();

	Document doc;

	if (argc > 1 && strcmp(argv[1], "-M") == 0) {
		doc.parseSourceMarkdown(data);
	} else {
		int idx = 0;
		doc.extract(data, idx, &doc.title, Strings::Title);
		doc.extract(data, idx, &doc.documentRoot, Strings::Document);
	}

	doc.output();

	return 0;
}
