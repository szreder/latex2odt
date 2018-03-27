#include "Parser/MarkdownParser.hpp"

void MarkdownParser::parseSource(const QString &data, int &idx, Node &node, const QString &endMarker)
{
	static const QHash <QChar, QString> FragmentMap {
		{'*', Strings::BoldFace},
		{'`', Strings::TextTT},
	};

	QString content;

	auto addText = [this, &content, &node]() {
		if (content.isEmpty())
			return;
		content.replace("&", "&amp;");
		content.replace("<", "&lt;");
		content.replace(">", "&gt;");

		node.appendNode(Node::Type::Text, std::move(content));
		content.clear();
	};

	while (idx != data.length()) {
		QChar current = data[idx++];

		if (endMarker == current) {
			addText();
			return;
		}

		if (!parseCtx.inCode && current == '[') { //skip URLs
			addText();
			do {
				ensureData(data, idx, 1);
				current = data[idx++];
			} while (current != '(');

			int endUrl = idx;
			do {
				ensureData(data, endUrl, 1);
				++endUrl;
			} while (data[endUrl] != ')');

			Node &urlNode = node.appendNode(Node::Type::Fragment, Strings::TextTT);
			urlNode.appendNode(Node::Type::Text, data.mid(idx, endUrl - idx));

			idx = endUrl + 1;
			continue;
		}

		if (!parseCtx.inCode && FragmentMap.contains(current)) {
			addText();
			Node &child = node.appendNode(Node::Type::Fragment, FragmentMap[current]);
			if (current == '`')
				parseCtx.inCode = true;
			parseSource(data, idx, child, current);
			if (current == '`')
				parseCtx.inCode = false;
		} else {
			content += current;
		}
	}

	addText();
}

Document MarkdownParser::doParse(const QString &data)
{
	Document doc;
	Node &root = doc.documentRoot;

	QStringList lines = data.split('\n');

	{
		Node &t = root.appendNode(Node::Type::Fragment, Strings::Title);
		int idx = 1;
		parseSource(lines[0], idx, t, QString{});
		root.appendNode(Node::Type::Tag, Strings::MakeTitle);
	}

	int line = 1;
	while (line != lines.count()) {
		QString s = lines[line++];
		if (!parseCtx.inCode && s.simplified().isEmpty())
			continue;

		if (s == "---") //TODO maybe? (horizontal rule)
			continue;

		if (s.startsWith("-")) {
			Node &list = root.appendNode(Node::Type::Environment, Strings::Itemize);

			while (s.startsWith("-")) {
				list.appendNode(Node::Type::Tag, Strings::Item);
				int idx = 1;
				parseSource(s, idx, list, QString{});

				if (line == lines.count())
					break;
				s = lines[line++];
			}

			continue;
		}

		if (s.startsWith("```")) {
			root.appendNode(Node::Type::Tag, Strings::CodeStart);

			QString language = s.right(s.length() - 3);
			QStringList codeLines;

			bool end = false;
			do {
				if (line == lines.count()) {
					qCritical() << "Unexpected EOF";
					abort();
				}
				s = lines[line++];
				end = s.startsWith("```");
				if (!end)
					codeLines.append(s);
			} while (!end);

			if (language.isEmpty()) {
				for (QString &l : codeLines) {
					Node &codeLine = root.appendNode(Node::Type::Environment, Strings::CodeLine);
					Node &lineContent = codeLine.appendNode(Node::Type::Fragment, Strings::TextTT); //temporary hack until syntax coloring for markdown is added
					addEntities(l);
					lineContent.appendNode(Node::Type::Text, l);
				}
			} else {
				QProcess highlight;
				highlight.start("highlight", QString{"-O latex --replace-quotes -j 3 -z -V -f -t 4 --encoding=utf-8 --syntax=%1"}.arg(language).split(' '));
				for (const QString &l : codeLines) {
					highlight.write(l.toUtf8());
					highlight.write("\n");
				}
				highlight.closeWriteChannel();
				highlight.waitForFinished();
				QByteArray output = highlight.readAllStandardOutput();
				parseCtx.inCode = true;
				int idx = 0;
				parseSource(QString{output}, idx, root, QString{});
				parseCtx.inCode = false;
			}
			root.appendNode(Node::Type::Tag, Strings::CodeEnd);
			continue;
		}

		const char *envName;
		int hashSymbolCnt = 0;
		while (hashSymbolCnt < s.length() && s[hashSymbolCnt] == '#')
			++hashSymbolCnt;

		if (hashSymbolCnt == 1 || hashSymbolCnt == 2) {
			envName = Strings::Section;
		} else if (hashSymbolCnt == 3 || hashSymbolCnt == 4) {
			envName = Strings::Subsection;
		} else {
			envName = Strings::Paragraph;
		}

		Node &n = root.appendNode(Node::Type::Environment, envName);
		int idx = hashSymbolCnt;
		parseSource(s, idx, n, QString{});
	}

	return doc;
}
