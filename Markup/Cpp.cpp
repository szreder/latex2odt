#include "Markup/Cpp.hpp"
#include "Strings.hpp"
#include "XmlGen.hpp"

namespace Cpp {

const QString AddAssign = "cppAddAssign";
const QString And = "cppAnd";
const QString Cpp = "cpp";
const QString Decrement = "cppDec";
const QString Equal = "cppEqual";
const QString GreaterEqual = "cppGreaterEqual";
const QString Increment = "cppInc";
const QString LeftShift = "cppLeftShift";
const QString LessEqual = "cppLessEqual";
const QString NotEqual = "cppNotEqual";
const QString Or = "cppOr";
const QString PtrAccess = "cppPtrAccess";
const QString RightShift = "cppRightShift";
const QString Scope = "cppScope";

const QString & markup(const QString &tag)
{
	static const QString Empty{};
	static const QMap <QString, QString> Markup = [](){
		QMap <QString, QString> result;

		result[AddAssign] = entryText(Strings::TextTT)
			+ '+' + Unicode::NoSpaceDontBreak + '='
			+ exitText(Strings::TextTT);

		result[And] = entryText(Strings::TextTT)
			+ "&amp;" + Unicode::NoSpaceDontBreak + "&amp;"
			+ exitText(Strings::TextTT);

		result[Cpp] = entryText(Strings::BoldFace) + 'C' + exitText(Strings::BoldFace)
			+ entryText(Strings::TextTT) +
			+ '+' + Unicode::NoSpaceDontBreak + '+'
			+ exitText(Strings::TextTT);

		result[Decrement] = entryText(Strings::TextTT)
			+ '-' + Unicode::NoSpaceDontBreak + '-'
			+ exitText(Strings::TextTT);

		result[Equal] = entryText(Strings::TextTT)
			+ '=' + Unicode::NoSpaceDontBreak + '='
			+ exitText(Strings::TextTT);

		result[GreaterEqual] = entryText(Strings::TextTT)
			+ "&gt;" + Unicode::NoSpaceDontBreak + '='
			+ exitText(Strings::TextTT);

		result[Increment] = entryText(Strings::TextTT)
			+ '+' + Unicode::NoSpaceDontBreak + '+'
			+ exitText(Strings::TextTT);

		result[LeftShift] = entryText(Strings::TextTT)
			+ "&lt;" + Unicode::NoSpaceDontBreak + "&lt;"
			+ exitText(Strings::TextTT);

		result[LessEqual] = entryText(Strings::TextTT)
			+ "&lt;" + Unicode::NoSpaceDontBreak + '='
			+ exitText(Strings::TextTT);

		result[NotEqual] = entryText(Strings::TextTT)
			+ '!' + Unicode::NoSpaceDontBreak + '='
			+ exitText(Strings::TextTT);

		result[Or] = entryText(Strings::TextTT)
			+ '|' + Unicode::NoSpaceDontBreak + '|'
			+ exitText(Strings::TextTT);

		result[PtrAccess] = entryText(Strings::TextTT)
			+ '-' + Unicode::NoSpaceDontBreak + "&gt;"
			+ exitText(Strings::TextTT);

		result[RightShift] = entryText(Strings::TextTT)
			+ "&gt;" + Unicode::NoSpaceDontBreak + "&gt;"
			+ exitText(Strings::TextTT);

		result[Scope] = entryText(Strings::TextTT)
			+ ':' + Unicode::NoSpaceDontBreak + ':'
			+ exitText(Strings::TextTT);

		return result;
	}();

	auto iter = Markup.find(tag);
	if (iter == Markup.end())
		return Empty;
	return *iter;
}

} // Cpp
