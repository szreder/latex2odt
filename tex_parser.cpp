#include <QtCore>

namespace {
QTextStream debug_output{stderr};

namespace Strings {
	const QString Backslash = "backslash";
	const QString Begin = "begin";
	const QString BoldFace = "textbf";
	const QString CodeEnd = "CodeEnd";
	const QString CodeLine = "CodeLine";
	const QString CodeStart = "CodeStart";
	const QString Document = "document";
	const QString End = "end";
	const QString Enumerate = "enumerate";
	const QString Hspace = "hspace";
	const QString Input = "input";
	const QString Italic = "textit";
	const QString Item = "item";
	const QString Itemize = "itemize";
	const QString Ldots = "ldots";
	const QString MakeTitle = "maketitle";
	const QString NormalFont = "normalfont";
	const QString Paragraph = "paragraph";
	const QString Quote = "dq";
	const QString Section = "section";
	const QString SourceCode = "sourcecodefile";
	const QString Subsection = "subsection";
	const QString TextBackslash = "textbackslash";
	const QString Textbar = "textbar";
	const QString TextTT = "texttt";
	const QString TTFamily = "ttfamily";
	const QString Tilde = "textasciitilde";
	const QString Title = "title";
	const QString Underscore = "textunderscore";
	const QString Verbatim = "verbatim";
}

namespace Highlight {
	const QString CommentBlock = "hlcom";
	const QString CommentCpp = "hlslc";
	const QString Escape = "hlesc";
	const QString KeywordA = "hlkwa";
	const QString KeywordB = "hlkwb";
	const QString KeywordC = "hlkwc";
	const QString NumberConstant = "hlnum";
	const QString LineNumbering = "hllin";
	const QString Operator = "hlopt";
	const QString Preprocessor = "hlppc";
	const QString Standard = "hlstd";
	const QString String = "hlstr";
	const QString Type = "hlkwd";
}

namespace Cpp {
	const QString Cpp = "cpp";
	const QString Decrement = "cppDec";
	const QString Increment = "cppInc";
}

namespace Unicode {
	const char *NoSpaceDontBreak = u8"\u2060";
	const char *NonBreakingSpace = u8"\u00a0";
}

const QSet <QString> Environment {
	Strings::Document,
	Strings::Enumerate,
	Strings::Itemize,
	Strings::Verbatim,
	"center",
};

const QSet <QString> Fragment {
	Strings::BoldFace,
	Strings::Hspace,
	Strings::Input,
	Strings::Italic,
	Strings::Section,
	Strings::SourceCode,
	Strings::Subsection,
	Strings::TextTT,
	Strings::Title,
	"hspace*",
	"mbox",
	"textsf",

	Highlight::CommentBlock,
	Highlight::CommentCpp,
	Highlight::Escape,
	Highlight::KeywordA,
	Highlight::KeywordB,
	Highlight::KeywordC,
	Highlight::LineNumbering,
	Highlight::NumberConstant,
	Highlight::Operator,
	Highlight::Preprocessor,
	Highlight::Standard,
	Highlight::String,
	Highlight::Type,
};

const QSet <QString> Tag {
	Strings::Backslash,
	Strings::Item,
	Strings::Ldots,
	Strings::MakeTitle,
	Strings::NormalFont,
	Strings::Quote,
	Strings::Textbar,
	Strings::TextBackslash,
	Strings::Tilde,
	Strings::TTFamily,
	Strings::Underscore,
	"fill",
	"indent",
	"noindent",
	"normalsize",

	Cpp::Cpp,
	Cpp::Decrement,
	Cpp::Increment,
};

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

struct Node {
	enum class Type {
		Invalid,
		Environment,
		Fragment,
		Tag,
		Text,
	};

	Node() = default;
	Node(Type type, const QString &value) : type{type}, value{value} {}

	static Type typeFromName(const QString &name);
	QString toString() const;

	Type type;
	QString value;
	bool endParagraph = false;
	QVector <Node *> children;
};

Node::Type Node::typeFromName(const QString &name)
{
	if (Environment.contains(name))
		return Type::Environment;
	if (Fragment.contains(name))
		return Type::Fragment;
	if (Tag.contains(name))
		return Type::Tag;

	qCritical() << QString{"Unknown type for '%1'"}.arg(name);
	std::exit(1);
	return Type::Invalid;
}

QString Node::toString() const
{
	QString typeString = "Invalid";
	switch (type) {
		case Type::Environment:
			typeString = "Environment";
			break;
		case Type::Fragment:
			typeString = "Fragment";
			break;
		case Type::Tag:
			typeString = "Tag";
			break;
		case Type::Text:
			typeString = "Text";
			break;
		default:
			break;
	}
	return QString{"type = %1, value = _%2_, endParagraph = %3"}.arg(typeString).arg(value).arg(endParagraph);
}

struct Document {
	Document() = default;

	Node * addNode(Node *parent, Node::Type type, const QString &value);
	void ensureData(const QString &data, int idx, int needBytes) const;
	void extract(const QString &data, int &idx, Node *root, const QString &token);
	QString getToken(const QString &data, int &idx);
	void parseSource(const QString &data, int &idx, Node *node, const QString &endMarker);

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
		content.replace("&", "&amp;");
		content.replace("<", "&lt;");
		content.replace(">", "&gt;");

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
				qCritical() << QString{"Unhandled token: %1"}.arg(token);
				std::exit(1);
			}
		} else {
			if (data[idx] == '{')
				++parseCtx.braceCnt;
			else if ((data[idx] != ' ' && data[idx] != '$') || !parseCtx.inCode) //ignore math mode for now
				content += data[idx];
			++idx;
		}
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

QString entryText(const QString &s)
{
	const QHash <QString, QString> EntryText {
		{Strings::BoldFace, "<text:span text:style-name=\"Bold\">"},
		{Strings::CodeLine, "<text:p text:style-name=\"CodeLine\">"},
		{Strings::Enumerate, "<text:list text:style-name=\"Enumerate\">"},
		{Strings::Italic, "<text:span text:style-name=\"Italic\">"},
		{Strings::Item, "<text:list-item>"},
		{Strings::Itemize, "<text:list>"},
		{Strings::Paragraph, "<text:p text:style-name=\"Paragraph\">"},
		{Strings::Section, "<text:h text:style-name=\"Section\" text:outline-level=\"2\">"},
		{Strings::Subsection, "<text:h text:style-name=\"Subsection\" text:outline-level=\"2\">"},
		{Strings::Title, "<text:h text:style-name=\"Header_Logo\" text:outline-level=\"1\">"},
		{Strings::TextTT, "<text:span text:style-name=\"Monospace\">"},

		{Highlight::CommentBlock, "<text:span text:style-name=\"HighlightComment\">"},
		{Highlight::CommentCpp, "<text:span text:style-name=\"HighlightComment\">"},
		{Highlight::Escape, "<text:span text:style-name=\"HighlightEscape\">"},
		{Highlight::KeywordA, "<text:span text:style-name=\"HighlightKeywordA\">"},
		{Highlight::KeywordB, "<text:span text:style-name=\"HighlightKeywordB\">"},
		{Highlight::KeywordC, "<text:span text:style-name=\"HighlightKeywordC\">"},
		{Highlight::LineNumbering, "<text:span text:style-name=\"HighlightLineNumbering\">"},
		{Highlight::NumberConstant, "<text:span text:style-name=\"HighlightNumberConstant\">"},
		{Highlight::Operator, "<text:span text:style-name=\"HighlightOperator\">"},
		{Highlight::Preprocessor, "<text:span text:style-name=\"HighlightPreprocessor\">"},
		{Highlight::Standard, "<text:span text:style-name=\"HighlightStandard\">"},
		{Highlight::String, "<text:span text:style-name=\"HighlightString\">"},
		{Highlight::Type, "<text:span text:style-name=\"HighlightType\">"},
	};

	return EntryText.value(s);
}

QString exitText(const QString &s)
{
	static const QString HeaderEnd = "</text:h>";
	static const QString ListEnd = "</text:list>";
	static const QString ParagraphEnd = "</text:p>";
	static const QString SpanEnd = "</text:span>";

	const QHash <QString, QString> ExitText {
		{Strings::BoldFace, SpanEnd},
		{Strings::CodeLine, ParagraphEnd},
		{Strings::Italic, SpanEnd},
		{Strings::Enumerate, ListEnd},
		{Strings::Item, "</text:list-item>"},
		{Strings::Itemize, ListEnd},
		{Strings::Paragraph, ParagraphEnd},
		{Strings::Section, HeaderEnd},
		{Strings::Subsection, HeaderEnd},
		{Strings::Title, HeaderEnd},
		{Strings::TextTT, SpanEnd},

		{Highlight::CommentBlock, SpanEnd},
		{Highlight::CommentCpp, SpanEnd},
		{Highlight::Escape, SpanEnd},
		{Highlight::KeywordA, SpanEnd},
		{Highlight::KeywordB, SpanEnd},
		{Highlight::KeywordC, SpanEnd},
		{Highlight::LineNumbering, SpanEnd},
		{Highlight::NumberConstant, SpanEnd},
		{Highlight::Operator, SpanEnd},
		{Highlight::Preprocessor, SpanEnd},
		{Highlight::Standard, SpanEnd},
		{Highlight::String, SpanEnd},
		{Highlight::Type, SpanEnd},
	};

	return ExitText.value(s);
};

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
				if (!paragraph.isEmpty()) {
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
				} else if (n->value == Strings::Tilde) {
					context.addText("~");
				} else if (n->value == Strings::Textbar) {
					context.addText("|");
				} else if (n->value == Cpp::Cpp) {
					context << entryText(Strings::BoldFace) << "C" << exitText(Strings::BoldFace)
						<< entryText(Strings::TextTT) << "+"
						<< Unicode::NoSpaceDontBreak
						<< "+" << exitText(Strings::TextTT);
				} else if (n->value == Cpp::Decrement) {
					context << entryText(Strings::TextTT) << "-" << exitText(Strings::TextTT)
						<< Unicode::NoSpaceDontBreak
						<< entryText(Strings::TextTT) << "-" << exitText(Strings::TextTT);
				} else if (n->value == Cpp::Increment) {
					context << entryText(Strings::TextTT) << "+" << exitText(Strings::TextTT)
						<< Unicode::NoSpaceDontBreak
						<< entryText(Strings::TextTT) << "+" << exitText(Strings::TextTT);
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

int main()
{
	QTextStream input{stdin};
	QString data = input.readAll();

	Document doc;
	int idx = 0;
	doc.extract(data, idx, &doc.title, Strings::Title);
	doc.extract(data, idx, &doc.documentRoot, Strings::Document);

	doc.output();

	return 0;
}
