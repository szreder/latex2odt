#include <unordered_set>

#include "Document.hpp"
#include "Strings.hpp"
#include "Vector.hpp"
#include "XmlGen.hpp"

namespace std {

template <>
struct hash<QString> {
	size_t operator () (const QString &s) const noexcept { return qHash(s); }
};

}

namespace {

bool isBlock(const QString &keyword)
{
	static const std::unordered_set <QString> Keywords {
		Strings::CodeLine,
		Strings::Paragraph,
		Strings::Section,
		Strings::Subsection,
		Strings::Title,
	};

	return Keywords.find(keyword) != Keywords.end();
}

bool isList(const QString &keyword)
{
	static const std::unordered_set <QString> Keywords {
		Strings::Enumerate,
		Strings::Itemize,
	};

	return Keywords.find(keyword) != Keywords.end();
}

bool ignore(const QString &keyword)
{
	static const std::unordered_set <QString> Keywords {
		Strings::Input,
		Strings::SourceCode,
	};

	return Keywords.find(keyword) != Keywords.end();
}

}

void Document::output() const
{
	QTextStream output{stdout};
	struct {
		void addText(const QString &text)
		{
			if (text.isEmpty())
				return;

			if (inCode && allWhitespace(text))
				paragraph.append(QString{"<text:s text:c=\"%1\"/>"}.arg(text.length()));
			else
				paragraph.append(text);
		}

		decltype(auto) operator << (const QString &text)
		{
			addText(text);
			return *this;
		}

		QString push(const QString &env, int *level = nullptr)
		{
			if (level != nullptr)
				*level = inside.count();
			return doPush(env);
		}

		QString pop()
		{
			if (inside.empty())
				return QString{};

			return doPop();
		}

		QString pop(int level)
		{
			QString result;
			while (inside.count() != level)
				result += doPop();

			return result;
		}

		bool inParagraph() const
		{
			if (inside.count() == 0)
				return false;
			return isBlock(inside.back());
		}

		bool empty() const
		{
			return inside.empty();
		}

		void reset()
		{
			listLevels.clear();
			inside.clear();
			paragraph.clear();
			inCode = false;
		}

		QString startCodeFrame()
		{
			QString result;
			if (inParagraph())
				result = pop();
			inCode = true;
			return result;
		}

		QString endCodeFrame()
		{
			QString result;
			if (inParagraph()) {
				if (paragraph.trimmed().isEmpty())
					pop();
				else
					result = pop();
			}
			inCode = false;
			return result;
		}

		QVector <int> listLevels;
		QVector <QString> inside;
		QString paragraph;
		bool inCode = false;

	private:
		QString doPush(const QString &env)
		{
			if (isList(env))
				listLevels.push_back(inside.count());

			if (inCode && env == Strings::Paragraph) {
				inside.push_back(Strings::CodeLine);
			} else {
				inside.push_back(env);
			}

			if (isBlock(env))
				return QString{};
			return entryText(env);
		}

		QString doPop()
		{
			QString result;
			if (inParagraph()) {
				if (!paragraph.isEmpty() || inCode) {
					if (inCode)
						paragraph.replace("    ", "<text:s text:c=\"4\"/>");

					result = QString{"%1%2%3"}.arg(entryText(inside.back())).arg(paragraph).arg(exitText(inside.back()));
					paragraph.clear();
				}
			} else {
				result = exitText(inside.back());
			}

			if (isList(inside.back()))
				listLevels.pop_back();
			inside.pop_back();

			return result;
		}

		bool allWhitespace(const QString &text) const
		{
			for (const QChar &c : text) {
				if (!c.isSpace())
					return false;
			}
			return true;
		}

	} context;

	std::function <void (const Node &)> doOutput = [this, &output, &context, &doOutput](const Node &n){
		if (n.type != Node::Type::Text && ignore(n.value))
			return;

		auto outputMode = n.type;
		if (n.type == Node::Type::Fragment && n.value == Strings::Title)
			outputMode = Node::Type::Environment;

		switch (outputMode) {
			case Node::Type::Environment: {
				if (context.inParagraph())
					output << context.pop();
				int level;
				output << context.push(n.value, &level);
				for (const Node &child : n.children)
					doOutput(child);
				output << context.pop(level);
				break;
			}
			case Node::Type::Fragment: {
				const bool inParagraph = context.inParagraph();
				const bool isBlock = ::isBlock(n.value);

				if (inParagraph && isBlock)
					output << context.pop();
				else if (!inParagraph && !isBlock)
					output << context.push(Strings::Paragraph);

				if (isBlock)
					output << context.push(n.value);
				else
					context.addText(entryText(n.value));

				for (const Node &child : n.children)
					doOutput(child);

				if (isBlock)
					output << context.pop();
				else
					context.addText(exitText(n.value));

				break;
			}
			case Node::Type::Tag:
				if (n.value == Strings::MakeTitle) {
					auto oldctx = context;
					context.reset();
					doOutput(this->title);
					context = oldctx;
				} else if (n.value == Strings::Item) {
					output << context.pop(context.listLevels.back() + 1);
					output << context.push(Strings::Item);
				} else if (n.value == Strings::Underscore) {
					context.addText("_");
				} else if (n.value == Strings::CodeStart) {
					output << context.startCodeFrame();
				} else if (n.value == Strings::CodeEnd) {
					output << context.endCodeFrame();
				} else if (n.value == Strings::Ldots) {
					context.addText("…");
				} else if (n.value == Strings::Tilde || n.value == Strings::CodeTilde) {
					context.addText("~");
				} else if (n.value == Strings::Textbar) {
					context.addText("|");
				}

				break;
			case Node::Type::Text:
				if (!context.inParagraph())
					output << context.push(Strings::Paragraph);

				context.addText(n.value);

				if (n.endParagraph)
					output << context.pop();
				break;
			default:
				qCritical() << QString{"Uknown type of node: %1"}.arg(static_cast<int>(n.type));
				std::exit(1);
		}
	};

	doOutput(documentRoot);

	while (!context.empty())
		output << context.pop();
}
