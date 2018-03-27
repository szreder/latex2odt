#pragma once

#include "Parser/Parser.hpp"

class MarkdownParser : public Parser {
private:
	std::optional <Document> doParse(const QString &data) override;

	struct {
		bool inCode;
	} parseCtx;

	void parseSource(const QString &data, int &idx, Node &node, const QString &endMarker);
};
