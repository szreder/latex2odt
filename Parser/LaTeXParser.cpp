#include <cassert>

#include "Fold.hpp"
#include "Markup/Cpp.hpp"
#include "Parser/LaTeXParser.hpp"

namespace {

const Vector <QChar> SpecialChars {
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

}

void LaTeXParser::ParseContext::reset(QString data)
{
	this->data = data;
	inCode = false;
	inMathMode = false;
	idx = 0;
	braceCnt = 0;
}

bool LaTeXParser::ParseContext::eof() const
{
	return idx == data.size();
}

QChar LaTeXParser::ParseContext::previous() const
{
	assert(idx > 0);
	return data[idx - 1];
}

QChar LaTeXParser::ParseContext::current() const
{
	return data[idx];
}

void LaTeXParser::ParseContext::advance(int steps)
{
	idx += steps;
}

bool LaTeXParser::ParseContext::advanceUntil(QChar c)
{
	while (!eof()) {
		if (current() == c)
			return true;
		advance();
	}

	return false;
}

QString LaTeXParser::ParseContext::getToken()
{
	auto endSymbol = [](QChar c){
		return c.isSpace() || SpecialChars.contains(c);
	};

	if (endSymbol(current())) {
		advance();
		return QString{previous()};
	}

	if (eof()) {
		qCritical() << "EOF with empty token";
		std::exit(1);
	}

	QString token;
	do {
		token += current();
		advance();
	} while (!eof() && !endSymbol(current()));

	if (eof()) {
		qInfo() << "EOF reached in getToken()";
		return token;
	}

	if (any_of(current(), '{', '}'))
		++idx;

	return token;
}

std::optional <Document> LaTeXParser::doParse(const QString &data)
{
	Document result;
	parseCtx.reset(data);
	if (!extract(result.title, Strings::Title) || !extract(result.documentRoot, Strings::Document))
		return {};
	return std::move(result);
}

bool LaTeXParser::extract(Node &root, const QString &token)
{
	const QString Pattern = generateBegin(token);
	parseCtx.idx = parseCtx.data.indexOf(Pattern, parseCtx.idx);
	if (parseCtx.idx == -1) {
		qWarning() << QString{"extract: pattern '%1' not found"}.arg(Pattern);
		return false;
	}
	parseCtx.advance(Pattern.length());

	root = Node{Node::typeFromName(token), QString{token}};
	return parseSource(root, generateEnd(token));
}

bool LaTeXParser::parseSource(Node &node, const QString &endMarker)
{
	QString content;

	auto addText = [this, &content, &node](bool paragraph = false) {
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
			Node &child = node.appendNode(Node::Type::Text, contentList[i].replace('\n', ' '));
			child.endParagraph = true;
		}

		Node &child = node.appendNode(Node::Type::Text, contentList.back().replace('\n', ' '));
		child.endParagraph = paragraph;
		content.clear();
	};

	while (!parseCtx.eof()) {
		if (parseCtx.current() == '}') {
			if (parseCtx.braceCnt != 0) {
				--parseCtx.braceCnt;
				parseCtx.advance();
			} else if (endMarker == "}") {
				addText();
				parseCtx.advance();
				return true;
			} else {
				qCritical() << QString{"unexpected closing brace"};
				return false;
			}
		} else if (parseCtx.current() == '\\') {
			parseCtx.advance();
			QString token = parseCtx.getToken();
			qDebug() << "token = " << token;
			qDebug() << QString{"data[idx] = %1, braceCnt = %2"}.arg(parseCtx.current()).arg(parseCtx.braceCnt);
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
					node.appendNode(Node::Type::Tag, token);
				}

				if (any_of(parseCtx.previous(), ' ', '{', '}'))
					parseCtx.advance(-1);
			} else if (Fragment.contains(token)) {
				if (parseCtx.current() == '}') {
					parseCtx.advance();
					if (token == "mbox" && parseCtx.inCode && parseCtx.current() == '\n')
						parseCtx.advance();
					continue;
				}

				if (token == Strings::Hspace) {
					parseCtx.advanceUntil('}');
					parseCtx.advance();
					content += Unicode::NoSpaceDontBreak;
					continue;
				}

				addText();
				Node &child = node.appendNode(Node::Type::Fragment, token);
				parseSource(child, "}");
				if (token == Strings::SourceCode) {
					if (child.children.count() != 1) {
						qCritical() << QString{"sourcecodefile node has %1 descendants, expected 1"}.arg(node.children.count());
						return false;
					}
					const QString &filename = child.children.front().value + ".tex";
					QFile sourceFile{filename};
					if (!sourceFile.open(QIODevice::ReadOnly)) {
						qCritical() << QString{"unable to open sourcecodefile: %1"}.arg(filename);
						return false;
					}

					QTextStream sourceStream{&sourceFile};
					ParseContext prevCtx = std::move(parseCtx);
					parseCtx.reset(sourceStream.readAll());
					parseCtx.inCode = true;

					node.appendNode(Node::Type::Tag, Strings::CodeStart);
					parseSource(node, QString{});
					node.appendNode(Node::Type::Tag, Strings::CodeEnd);
					parseCtx = std::move(prevCtx);
				}
			} else if (token == Strings::Begin || token == Strings::End) {
				addText();
				const bool isBegin = (token == Strings::Begin);
				const QString envName = parseCtx.getToken();
				if (!Environment.contains(envName)) {
					qCritical() << QString{"Unknown environment: %1"}.arg(envName);
					return false;
				}

				if (envName == Strings::Verbatim) {
					parseCtx.inCode = isBegin;
					continue;
				}

				if (isBegin) {
					Node &child = node.appendNode(Node::Type::Environment, envName);
					if (!parseSource(child, generateEnd(envName)))
						return false;
				} else {
					token = QString{"%1{%2}"}.arg(token).arg(envName);
					if (token != endMarker) {
						qCritical() << QString{"Expected endMarker %1, got %2"}.arg(endMarker).arg(token);
						return false;
					}
					return true;
				}
			} else {
				const QString &text = Cpp::markup(token);
				if (text.isEmpty()) {
					qCritical() << QString{"Unhandled token: %1"}.arg(token);
					return false;
				}

				addText();
				node.appendNode(Node::Type::Text, text);

				if (parseCtx.current() == '}')
					parseCtx.advance();
			}
		} else {
			switch (parseCtx.current().toLatin1()) {
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
						parseCtx.advance();
						Node &child = node.appendNode(Node::Type::Fragment, Strings::Superscript);
						if (parseCtx.current() == '{') {
							parseCtx.advance();
							parseSource(child, "}");
							parseCtx.advance(-1);
						} else {
							child.appendNode(Node::Type::Text, parseCtx.current());
						}
					} else {
						content += parseCtx.current();
					}
					break;
				case ' ':
					if (!parseCtx.inCode)
						content += parseCtx.current();
					break;
				default:
					content += parseCtx.current();
			}
			parseCtx.advance();
		}
	}

	return true;
}
