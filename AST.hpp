#pragma once

#include <QtCore>

#include "Markup/Highlight.hpp"
#include "Strings.hpp"

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
	Highlight::IncludeQuote,
	Highlight::LineNumbering,
	Highlight::NumberConstant,
	Highlight::Operator,
	Highlight::Preprocessor,
	Highlight::Standard,
	Highlight::String,
	Highlight::StringSubstitution,
	Highlight::Type,
};

const QSet <QString> Tag {
	Strings::Backslash,
	Strings::CodeTilde,
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
};

struct Node {
	enum class Type : quint8 {
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
