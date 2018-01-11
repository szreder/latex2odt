#include <QtCore>

#include "Markup/Highlight.hpp"
#include "Strings.hpp"

QString entryText(const QString &s)
{
	const QHash <QString, const char *> EntryText {
		{Strings::BoldFace, "<text:span text:style-name=\"Bold\">"},
		{Strings::CodeLine, "<text:p text:style-name=\"CodeLine\">"},
		{Strings::Enumerate, "<text:list text:style-name=\"Enumerate\">"},
		{Strings::Italic, "<text:span text:style-name=\"Italic\">"},
		{Strings::Item, "<text:list-item>"},
		{Strings::Itemize, "<text:list>"},
		{Strings::Paragraph, "<text:p text:style-name=\"Paragraph\">"},
		{Strings::Section, "<text:h text:style-name=\"Section\" text:outline-level=\"2\">"},
		{Strings::Subsection, "<text:h text:style-name=\"Subsection\" text:outline-level=\"2\">"},
		{Strings::Superscript, "<text:span text:style-name=\"Superscript\">"},
		{Strings::Title, "<text:h text:style-name=\"Header_Logo\" text:outline-level=\"1\">"},
		{Strings::TextTT, "<text:span text:style-name=\"Monospace\">"},

		{Highlight::CommentBlock, "<text:span text:style-name=\"HighlightComment\">"},
		{Highlight::CommentCpp, "<text:span text:style-name=\"HighlightComment\">"},
		{Highlight::Escape, "<text:span text:style-name=\"HighlightEscape\">"},
		{Highlight::KeywordA, "<text:span text:style-name=\"HighlightKeywordA\">"},
		{Highlight::KeywordB, "<text:span text:style-name=\"HighlightKeywordB\">"},
		{Highlight::KeywordC, "<text:span text:style-name=\"HighlightKeywordC\">"},
		{Highlight::IncludeQuote, "<text:span text:style-name=\"HighlightPreprocessor\">"},
		{Highlight::LineNumbering, "<text:span text:style-name=\"HighlightLineNumbering\">"},
		{Highlight::NumberConstant, "<text:span text:style-name=\"HighlightNumberConstant\">"},
		{Highlight::Operator, "<text:span text:style-name=\"HighlightOperator\">"},
		{Highlight::Preprocessor, "<text:span text:style-name=\"HighlightPreprocessor\">"},
		{Highlight::Standard, "<text:span text:style-name=\"HighlightStandard\">"},
		{Highlight::String, "<text:span text:style-name=\"HighlightString\">"},
		{Highlight::StringSubstitution, "<text:span text:style-name=\"HighlightStringSubstitution\">"},
		{Highlight::Type, "<text:span text:style-name=\"HighlightType\">"},
	};

	return EntryText.value(s);
}

QString exitText(const QString &s)
{
	static const char * HeaderEnd = "</text:h>";
	static const char * ListEnd = "</text:list>";
	static const char * ParagraphEnd = "</text:p>";
	static const char * SpanEnd = "</text:span>";

	const QHash <QString, const char *> ExitText {
		{Strings::BoldFace, SpanEnd},
		{Strings::CodeLine, ParagraphEnd},
		{Strings::Italic, SpanEnd},
		{Strings::Enumerate, ListEnd},
		{Strings::Item, "</text:list-item>"},
		{Strings::Itemize, ListEnd},
		{Strings::Paragraph, ParagraphEnd},
		{Strings::Section, HeaderEnd},
		{Strings::Subsection, HeaderEnd},
		{Strings::Superscript, SpanEnd},
		{Strings::Title, HeaderEnd},
		{Strings::TextTT, SpanEnd},

		{Highlight::CommentBlock, SpanEnd},
		{Highlight::CommentCpp, SpanEnd},
		{Highlight::Escape, SpanEnd},
		{Highlight::KeywordA, SpanEnd},
		{Highlight::KeywordB, SpanEnd},
		{Highlight::KeywordC, SpanEnd},
		{Highlight::IncludeQuote, SpanEnd},
		{Highlight::LineNumbering, SpanEnd},
		{Highlight::NumberConstant, SpanEnd},
		{Highlight::Operator, SpanEnd},
		{Highlight::Preprocessor, SpanEnd},
		{Highlight::Standard, SpanEnd},
		{Highlight::String, SpanEnd},
		{Highlight::StringSubstitution, SpanEnd},
		{Highlight::Type, SpanEnd},
	};

	return ExitText.value(s);
};
