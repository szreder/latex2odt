#pragma once

#include "AST.hpp"

struct Document {
	Document() = default;
	Document(Node &&title, Node &&root) : title{std::move(title)}, documentRoot{std::move(root)} {}

	void output() const;

	Node title;
	Node documentRoot;
};
