#pragma once

#include "AST.hpp"

struct Document {
	Document() = default;
	Document(Node &&title, Node &&root) : title{std::move(title)}, documentRoot{std::move(root)} {}
	Document(Document &&other) : title{std::move(other.title)}, documentRoot{std::move(other.documentRoot)} {}

	void output() const;

	Node title;
	Node documentRoot;
};
