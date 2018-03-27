#pragma once

#include "Document.hpp"

class Parser {

public:
	std::optional <Document> parse(const QString &data) { return doParse(data); }

protected:
	void ensureData(const QString &data, int idx, int needBytes) const
	{
		if (idx + needBytes >= data.size()) {
			qCritical() << QString{"End of data, data.length() = %1, idx = %2, needBytes = %3"}.arg(data.length()).arg(idx).arg(needBytes);
			std::exit(1);
		}
	}

	//FIXME this shouldn't be in the Parser and better be a one-pass transform
	void addEntities(QString &text) const
	{
		text.replace("&", "&amp;");
		text.replace("<", "&lt;");
		text.replace(">", "&gt;");
	}

private:
	virtual std::optional <Document> doParse(const QString &data) = 0;
};
