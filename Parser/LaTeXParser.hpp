#pragma once

#include "Parser/Parser.hpp"

class LaTeXParser : public Parser {
public:
	static std::optional <Document> parse(const QString &data);

private:
	std::optional <Document> doParse(const QString &data) override;

	struct ParseContext {
		ParseContext() = default;
		ParseContext(ParseContext &&) = default;
		ParseContext & operator = (ParseContext &&) = default;

		QString data;
		bool inCode = false;
		bool inMathMode = false;
		int idx = 0, braceCnt = 0;

		void reset(QString data = QString{});
		bool eof() const;
		QChar current() const;
		QChar previous() const;
		void advance(int steps = 1);
		bool advanceUntil(QChar c);
		QString getToken();
	} parseCtx;

	bool extract(Node &root, const QString &token);
	bool parseSource(Node &node, const QString &endMarker);
};
