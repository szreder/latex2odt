#include <QtCore>

#include "Markup/Cpp.hpp"
#include "Markup/Highlight.hpp"
#include "AST.hpp"
#include "Strings.hpp"
#include "XmlGen.hpp"

namespace {
QTextStream debug_output{stderr};

bool isBlock(const QString &keyword)
{
	static const QVector <QString> Keywords {
		Strings::CodeLine,
		Strings::Paragraph,
		Strings::Section,
		Strings::Subsection,
		Strings::Title,
	};

	return Keywords.contains(keyword);
}

bool isList(const QString &keyword)
{
	static const QVector <QString> Keywords {
		Strings::Enumerate,
		Strings::Itemize,
	};

	return Keywords.contains(keyword);
}

bool ignore(const QString &keyword)
{
	static const QVector <QString> Keywords {
		Strings::Input,
		Strings::SourceCode,
	};

	return Keywords.contains(keyword);
}

inline void addEntities(QString &text)
{
	text.replace("&", "&amp;");
	text.replace("<", "&lt;");
	text.replace(">", "&gt;");
}

}

inline QString generateBegin(const QString &s)
{
	if (Environment.contains(s))
		return QString{"\\%1{%2}"}.arg(Strings::Begin).arg(s);
	if (Fragment.contains(s))
		return QString{"\\%1{"}.arg(s);
	if (Tag.contains(s))
		return QString{"\\%1"}.arg(s);

	qCritical() << "generateBegin() - unknown element:" << s;
	std::exit(1);
	return QString{};
}

inline QString generateEnd(const QString &s)
{
	if (Environment.contains(s))
		return QString{"%1{%2}"}.arg(Strings::End).arg(s);
	if (Fragment.contains(s))
		return QString{"}"};

	if (!Tag.contains(s)) {
		qCritical() << "generateEnd() - unknown element:" << s;
		std::exit(1);
	}

	return QString{};
}

struct Document {
	Document() = default;

	Node * addNode(Node *parent, Node::Type type, const QString &value);
	void ensureData(const QString &data, int idx, int needBytes) const;
	void extract(const QString &data, int &idx, Node *root, const QString &token);
	QString getToken(const QString &data, int &idx);
	void parseSource(const QString &data, int &idx, Node *node, const QString &endMarker);
	void parseSourceMarkdown(const QString &data);
	void parseSourceMarkdown(const QString &data, int &idx, Node *node, const QString &endMarker);

	void output() const;

	Node title;
	Node documentRoot;
};

Node * Document::addNode(Node *parent, Node::Type type, const QString &value)
{
	Node *child = new Node{type, value};
	parent->children.append(child);
	return child;
}

inline void Document::ensureData(const QString &data, int idx, int needBytes) const
{
	if (idx + needBytes >= data.size()) {
		qCritical() << QString{"End of data, data.length() = %1, idx = %2, needBytes = %3"}.arg(data.length()).arg(idx).arg(needBytes);
		std::exit(1);
	}
}

void Document::extract(const QString &data, int &idx, Node *root, const QString &token)
{
	const QString Pattern = generateBegin(token);
	idx = data.indexOf(Pattern, idx);
	if (idx == -1) {
		qWarning() << QString{"extract: pattern '%1' not found"}.arg(Pattern);
		return;
	}
	idx += Pattern.length();

	*root = Node{Node::typeFromName(token), token};
	parseSource(data, idx, root, generateEnd(token));
}

static const QVector <QChar> SpecialChars {
	'\'',
	'{',
	'}',
	'\\',
	'#',
	'$',
	'%',
	'_',
	'&',
	'^',
};

QString Document::getToken(const QString &data, int &idx)
{
	ensureData(data, idx, 1);
	auto endSymbol = [](QChar c){
		return c.isSpace() || SpecialChars.contains(c);
	};

	if (endSymbol(data[idx])) {
		++idx;
		return QString{data[idx - 1]};
	}

	QString token;
	do {
		token += data[idx];
		++idx;
	} while (idx != data.length() && !endSymbol(data[idx]));

	if (idx == data.length()) {
		qInfo() << "EOF reached in getToken()";
		return token;
	}

	if (data[idx] == '{' || data[idx] == '}')
		++idx;

	return token;
}

struct {
	bool inCode = false;
	bool inMathMode = false;
	int braceCnt = 0;
} parseCtx;

void Document::parseSource(const QString &data, int &idx, Node *node, const QString &endMarker)
{
	qDebug() << QString{"parseSource called, idx = %1, endMarker = %2"}.arg(idx).arg(endMarker);
	QString content;

	auto addText = [this, &content, node](bool paragraph = false) {
		qDebug() << QString{"addText: parseCtx.inCode = %1, content = _%2_"}.arg(parseCtx.inCode).arg(content);
		if (!parseCtx.inCode && content.trimmed().isEmpty() && paragraph == false) {
			content.clear();
			return;
		}
		content.replace('~', Unicode::NonBreakingSpace);

		if (!parseCtx.inCode) {
			content.replace("---", "–");
			content.replace("--", "–");
		}
		addEntities(content);

		QStringList contentList = content.split("\n\n");

		if (parseCtx.inCode) {
			for (QString &s : contentList)
				s = s.remove('\n');
		}

		for (int i = 0; i < contentList.count() - 1; ++i) {
			Node *child = addNode(node, Node::Type::Text, contentList[i].replace('\n', ' '));
			child->endParagraph = true;
		}

		Node *child = addNode(node, Node::Type::Text, contentList.back().replace('\n', ' '));
		child->endParagraph = paragraph;
		content.clear();
	};

	while (idx != data.length()) {
		if (data[idx] == '}') {
			if (parseCtx.braceCnt != 0) {
				--parseCtx.braceCnt;
				++idx;
			} else if (endMarker == "}") {
				addText();
				++idx;
				return;
			} else {
				qCritical() << QString{"unexpected closing brace"};
				std::exit(1);
			}
		} else if (data[idx] == '\\') {
			++idx;
			QString token = getToken(data, idx);
			qDebug() << "token = " << token;
			qDebug() << QString{"data[idx] = %1, braceCnt = %2"}.arg(data[idx]).arg(parseCtx.braceCnt);
			if (SpecialChars.contains(token[0]) || token[0].isSpace()) {
				if (token[0] == '\\') {
					addText(true);
				} else if (token[0] != '\n') {
					content += token;
				}
			} else if (Tag.contains(token)) {
				addText();
				if (token == Strings::Quote) {
					content += '"';
				} else if (token == Strings::Backslash || token == Strings::TextBackslash) {
					content += '\\';
				} else {
					addNode(node, Node::Type::Tag, token);
				}

				if (data[idx - 1] == ' ' || data[idx - 1] == '{' || data[idx - 1] == '}')
					--idx;
			} else if (Fragment.contains(token)) {
				if (data[idx] == '}') {
					++idx;
					if (token == "mbox" && parseCtx.inCode && data[idx] == '\n')
						++idx;
					continue;
				}

				if (token == Strings::Hspace) {
					while (data[idx++] != '}');
					content += Unicode::NoSpaceDontBreak;
					continue;
				}

				addText();
				Node *child = addNode(node, Node::Type::Fragment, token);
				qDebug() << QString{"token = %1, calling parseSource with endMarker = }, data[idx] = %2"}.arg(token).arg(data[idx]);
				parseSource(data, idx, child, "}");
				if (token == Strings::SourceCode) {
					if (child->children.count() != 1) {
						qCritical() << QString{"sourcecodefile node has %1 descendants, expected 1"}.arg(node->children.count());
						std::exit(1);
					}
					const QString &filename = child->children[0]->value + ".tex";
					QFile sourceFile{filename};
					if (!sourceFile.open(QIODevice::ReadOnly)) {
						qCritical() << QString{"unable to open sourcecodefile: %1"}.arg(filename);
						std::exit(1);
					}

					QTextStream sourceStream{&sourceFile};
					const QString sourceText = sourceStream.readAll();
					int idx = 0;
					qDebug() << QString{"token = %1, calling parseSource on sourcecode with endMarker = QString{}"}.arg(token);
					parseCtx.inCode = true;
					addNode(node, Node::Type::Tag, Strings::CodeStart);
					parseSource(sourceText, idx, node, QString{});
					addNode(node, Node::Type::Tag, Strings::CodeEnd);
					parseCtx.inCode = false;
				}
			} else if (token == Strings::Begin || token == Strings::End) {
				addText();
				const bool isBegin = (token == Strings::Begin);
				const QString envName = getToken(data, idx);
				if (!Environment.contains(envName)) {
					qCritical() << QString{"Unknown environment: %1"}.arg(envName);
					std::exit(1);
				}

				if (envName == Strings::Verbatim) {
					parseCtx.inCode = isBegin;
					continue;
				}

				if (isBegin) {
					Node *child = addNode(node, Node::Type::Environment, envName);
					qDebug() << QString{"environ = %1, calling parseSource with endMarker = %2"}.arg(envName).arg(generateEnd(envName));
					parseSource(data, idx, child, generateEnd(envName));
				} else {
					token = QString{"%1{%2}"}.arg(token).arg(envName);
					if (token != endMarker) {
						qCritical() << QString{"Expected endMarker %1, got %2"}.arg(endMarker).arg(token);
						std::exit(1);
					}
					return;
				}

			} else {
				const QString &text = Cpp::markup(token);
				if (text.isEmpty()) {
					qCritical() << QString{"Unhandled token: %1"}.arg(token);
					std::exit(1);
				}

				addText();
				addNode(node, Node::Type::Text, text);
				if (data[idx] == '}')
					++idx;
			}
		} else {
			switch (data[idx].toLatin1()) {
				case '{':
					++parseCtx.braceCnt;
					break;
				case '$':
					if (!parseCtx.inCode)
						parseCtx.inMathMode = !parseCtx.inMathMode;
					break;
				case '^':
					if (parseCtx.inMathMode) {
						addText();
						++idx;
						Node *child = addNode(node, Node::Type::Fragment, Strings::Superscript);
						if (data[idx] == '{') {
							++idx;
							parseSource(data, idx, child, "}");
							--idx;
						} else {
							addNode(child, Node::Type::Text, data[idx]);
						}
					} else {
						content += data[idx];
					}
					break;
				case ' ':
					if (!parseCtx.inCode)
						content += data[idx];
					break;
				default:
					content += data[idx];
			}
			++idx;
		}
	}
}

void Document::parseSourceMarkdown(const QString &data, int &idx, Node *node, const QString &endMarker)
{
	static const QHash <QChar, QString> FragmentMap {
		{'*', Strings::BoldFace},
		{'`', Strings::TextTT},
	};

	QString content;

	auto addText = [this, &content, node]() {
		if (content.isEmpty())
			return;
		content.replace("&", "&amp;");
		content.replace("<", "&lt;");
		content.replace(">", "&gt;");

		addNode(node, Node::Type::Text, content);
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

			Node *urlNode = addNode(node, Node::Type::Fragment, Strings::TextTT);
			addNode(urlNode, Node::Type::Text, data.mid(idx, endUrl - idx));

			idx = endUrl + 1;
			continue;
		}

		if (!parseCtx.inCode && FragmentMap.contains(current)) {
			addText();
			Node *child = addNode(node, Node::Type::Fragment, FragmentMap[current]);
			if (current == '`')
				parseCtx.inCode = true;
			parseSourceMarkdown(data, idx, child, current);
			if (current == '`')
				parseCtx.inCode = false;
		} else {
			content += current;
		}
	}

	addText();
}

void Document::parseSourceMarkdown(const QString &data)
{
	QStringList lines = data.split('\n');

	{
		Node *t = addNode(&documentRoot, Node::Type::Fragment, Strings::Title);
		int idx = 1;
		parseSourceMarkdown(lines[0], idx, t, QString{});
		addNode(&documentRoot, Node::Type::Tag, Strings::MakeTitle);
	}

	int line = 1;
	while (line != lines.count()) {
		QString s = lines[line++];
		if (!parseCtx.inCode && s.simplified().isEmpty())
			continue;

		if (s == "---") //TODO maybe? (horizontal rule)
			continue;

		if (s.startsWith("-")) {
			Node *list = addNode(&documentRoot, Node::Type::Environment, Strings::Itemize);

			while (s.startsWith("-")) {
				addNode(list, Node::Type::Tag, Strings::Item);
				int idx = 1;
				parseSourceMarkdown(s, idx, list, QString{});

				if (line == lines.count())
					break;
				s = lines[line++];
			}

			continue;
		}

		if (s.startsWith("```")) {
			addNode(&documentRoot, Node::Type::Tag, Strings::CodeStart);

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
					Node *codeLine = addNode(&documentRoot, Node::Type::Environment, Strings::CodeLine);
					Node *lineContent = addNode(codeLine, Node::Type::Fragment, Strings::TextTT); //temporary hack until syntax coloring for markdown is added
					addEntities(l);
					addNode(lineContent, Node::Type::Text, l);
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
				parseSource(QString{output}, idx, &documentRoot, QString{});
				parseCtx.inCode = false;
			}
			addNode(&documentRoot, Node::Type::Tag, Strings::CodeEnd);
			continue;
		}

		Node *n;
		const QString *envName;
		int hashSymbolCnt = 0;
		while (hashSymbolCnt < s.length() && s[hashSymbolCnt] == '#')
			++hashSymbolCnt;

		if (hashSymbolCnt == 1 || hashSymbolCnt == 2) {
			envName = &Strings::Section;
		} else if (hashSymbolCnt == 3 || hashSymbolCnt == 4) {
			envName = &Strings::Subsection;
		} else {
			envName = &Strings::Paragraph;
		}

		n = addNode(&documentRoot, Node::Type::Environment, *envName);
		int idx = hashSymbolCnt;
		parseSourceMarkdown(s, idx, n, QString{});
	}
}

/*
void Document::output() const
{
	QTextStream output{stdout};

	std::function <void (const Node *)> doOutput = [this, &output, &doOutput](const Node *n){
		for (const Node *child : n->children) {
			switch (child->type) {
				case Node::Type::Environment:
					output << QString{"<env class='%1'>"}.arg(child->value);
					doOutput(child);
					output << QString{"</env>"}.arg(child->value);
					break;
				case Node::Type::Fragment:
					output << QString{"<%1>"}.arg(child->value);
					doOutput(child);
					output << QString{"</%1>"}.arg(child->value);
					break;
				case Node::Type::Tag:
					output << QString{"<%1 />"}.arg(child->value);
					break;
				case Node::Type::Text:
					output << child->value;
					break;
				default:
					qCritical() << QString{"Uknown type of node: %1"}.arg(static_cast<int>(child->type));
					std::exit(1);
			}
		}
	};

	doOutput(&root);
}
*/

void Document::output() const
{
	QTextStream output{stdout};
	struct {
		void addText(const QString &text)
		{
			if (text.isEmpty())
				return;

			if (inCode && allWhitespace(text))
				paragraph.append(QString{"<text:s text:c=\"%1\"/>"}.arg(text.length()));
			else
				paragraph.append(text);
		}

		decltype(auto) operator << (const QString &text)
		{
			addText(text);
			return *this;
		}

		QString push(const QString &env, int *level = nullptr)
		{
			if (level != nullptr)
				*level = inside.count();
			return doPush(env);
		}

		QString pop()
		{
			if (inside.empty())
				return QString{};

			return doPop();
		}

		QString pop(int level)
		{
			QString result;
			while (inside.count() != level)
				result += doPop();

			return result;
		}

		bool inParagraph() const
		{
			if (inside.count() == 0)
				return false;
			return isBlock(inside.back());
		}

		bool empty() const
		{
			return inside.empty();
		}

		void reset()
		{
			listLevels.clear();
			inside.clear();
			paragraph.clear();
			inCode = false;
		}

		QString startCodeFrame()
		{
			QString result;
			if (inParagraph())
				result = pop();
			inCode = true;
			return result;
		}

		QString endCodeFrame()
		{
			QString result;
			if (inParagraph()) {
				if (paragraph.trimmed().isEmpty())
					pop();
				else
					result = pop();
			}
			inCode = false;
			return result;
		}

		QVector <int> listLevels;
		QVector <QString> inside;
		QString paragraph;
		bool inCode = false;

	private:
		QString doPush(const QString &env)
		{
			if (isList(env))
				listLevels.push_back(inside.count());

			if (inCode && env == Strings::Paragraph) {
				inside.push_back(Strings::CodeLine);
			} else {
				inside.push_back(env);
			}

			if (isBlock(env))
				return QString{};
			return entryText(env);
		}

		QString doPop()
		{
			QString result;
			if (inParagraph()) {
				if (!paragraph.isEmpty() || inCode) {
					if (inCode)
						paragraph.replace("    ", "<text:s text:c=\"4\"/>");

					result = QString{"%1%2%3"}.arg(entryText(inside.back())).arg(paragraph).arg(exitText(inside.back()));
					paragraph.clear();
				}
			} else {
				result = exitText(inside.back());
			}

			if (isList(inside.back()))
				listLevels.pop_back();
			inside.pop_back();

			return result;
		}

		bool allWhitespace(const QString &text) const
		{
			for (const QChar &c : text) {
				if (!c.isSpace())
					return false;
			}
			return true;
		}

	} context;

	std::function <void (const Node *)> doOutput = [this, &output, &context, &doOutput](const Node *n){
		if (n->type != Node::Type::Text && ignore(n->value))
			return;

		auto outputMode = n->type;
		if (n->type == Node::Type::Fragment && n->value == Strings::Title)
			outputMode = Node::Type::Environment;

		switch (outputMode) {
			case Node::Type::Environment: {
				if (context.inParagraph())
					output << context.pop();
				int level;
				output << context.push(n->value, &level);
				for (const Node *child : n->children)
					doOutput(child);
				output << context.pop(level);
				break;
			}
			case Node::Type::Fragment: {
				const bool inParagraph = context.inParagraph();
				const bool isBlock = ::isBlock(n->value);

				if (inParagraph && isBlock)
					output << context.pop();
				else if (!inParagraph && !isBlock)
					output << context.push(Strings::Paragraph);

				if (isBlock)
					output << context.push(n->value);
				else
					context.addText(entryText(n->value));

				for (const Node *child : n->children)
					doOutput(child);

				if (isBlock)
					output << context.pop();
				else
					context.addText(exitText(n->value));

				break;
			}
			case Node::Type::Tag:
				if (n->value == Strings::MakeTitle) {
					auto oldctx = context;
					context.reset();
					doOutput(&this->title);
					context = oldctx;
				} else if (n->value == Strings::Item) {
					output << context.pop(context.listLevels.back() + 1);
					output << context.push(Strings::Item);
				} else if (n->value == Strings::Underscore) {
					context.addText("_");
				} else if (n->value == Strings::CodeStart) {
					output << context.startCodeFrame();
				} else if (n->value == Strings::CodeEnd) {
					output << context.endCodeFrame();
				} else if (n->value == Strings::Ldots) {
					context.addText("…");
				} else if (n->value == Strings::Tilde || n->value == Strings::CodeTilde) {
					context.addText("~");
				} else if (n->value == Strings::Textbar) {
					context.addText("|");
				}

				break;
			case Node::Type::Text:
				if (!context.inParagraph())
					output << context.push(Strings::Paragraph);

				context.addText(n->value);

				if (n->endParagraph)
					output << context.pop();
				break;
			default:
				qCritical() << QString{"Uknown type of node: %1"}.arg(static_cast<int>(n->type));
				std::exit(1);
		}
	};

	doOutput(&documentRoot);

	while (!context.empty())
		output << context.pop();
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
