#include "AST.hpp"

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

static inline constexpr uint qHash(const Node::Type &t)
{
	return ::qHash(static_cast<typename std::underlying_type<std::decay_t<decltype(t)> >::type>(t));
}

Node::Type Node::typeFromName(const QString &name)
{
	using TypePair = QPair <const QSet <QString> *, Node::Type>;
	static const QVector <TypePair> TypeVector {
		TypePair{&Environment, Type::Environment},
		{&Fragment, Type::Fragment},
		{&Tag, Type::Tag},
	};

	for (auto p : TypeVector) {
		if (p.first->contains(name))
			return p.second;
	}

	qCritical() << QString{"Unknown type for '%1'"}.arg(name);
	std::exit(1);
	return Type::Invalid;
}

QString Node::toString() const
{
	static const QHash <Node::Type, const char *> TypeHash {
		{Type::Environment, "Environment"},
		{Type::Fragment, "Fragment"},
		{Type::Tag, "Tag"},
		{Type::Text, "Text"},
	};

	QString typeString = TypeHash.value(type, "Invalid");
	return QString{"type = %1, value = _%2_, endParagraph = %3"}.arg(typeString).arg(value).arg(endParagraph);
}
