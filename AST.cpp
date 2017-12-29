#include "AST.hpp"

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
	static const QHash <Node::Type, QString> TypeHash {
		{Type::Environment, "Environment"},
		{Type::Fragment, "Fragment"},
		{Type::Tag, "Tag"},
		{Type::Text, "Text"},
	};

	QString typeString = TypeHash.value(type, "Invalid");
	return QString{"type = %1, value = _%2_, endParagraph = %3"}.arg(typeString).arg(value).arg(endParagraph);
}
