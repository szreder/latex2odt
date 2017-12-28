#include "AST.hpp"

Node::Type Node::typeFromName(const QString &name)
{
	if (Environment.contains(name))
		return Type::Environment;
	if (Fragment.contains(name))
		return Type::Fragment;
	if (Tag.contains(name))
		return Type::Tag;

	qCritical() << QString{"Unknown type for '%1'"}.arg(name);
	std::exit(1);
	return Type::Invalid;
}

QString Node::toString() const
{
	QString typeString = "Invalid";
	switch (type) {
		case Type::Environment:
			typeString = "Environment";
			break;
		case Type::Fragment:
			typeString = "Fragment";
			break;
		case Type::Tag:
			typeString = "Tag";
			break;
		case Type::Text:
			typeString = "Text";
			break;
		default:
			break;
	}
	return QString{"type = %1, value = _%2_, endParagraph = %3"}.arg(typeString).arg(value).arg(endParagraph);
}
