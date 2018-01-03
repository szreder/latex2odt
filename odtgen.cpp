#include <QtCore>

#include "Markup/Cpp.hpp"
#include "Markup/Highlight.hpp"
#include "AST.hpp"
#include "Document.hpp"
#include "Strings.hpp"
#include "XmlGen.hpp"

namespace {
QTextStream debug_output{stderr};

}

void debugOutput(const Node *n, int indent = 0)
{
	for (int i = 0; i < indent; ++i)
		debug_output << '\t';
	debug_output << n->toString() << '\n';
	for (const Node *child : n->children)
		debugOutput(child, indent + 1);
}

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
