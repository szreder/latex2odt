#pragma once

#include "AST.hpp"

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
