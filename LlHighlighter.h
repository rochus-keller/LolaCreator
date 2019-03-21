#ifndef LLHIGHLIGHTER_H
#define LLHIGHLIGHTER_H

/*
* Copyright 2019 Rochus Keller <mailto:me@rochus-keller.ch>
*
* This file is part of the LolaCreator plugin.
*
* The following is the license that applies to this copy of the
* plugin. For a license to use the plugin under conditions
* other than those described here, please email to me@rochus-keller.ch.
*
* GNU General Public License Usage
* This file may be used under the terms of the GNU General Public
* License (GPL) versions 2.0 or 3.0 as published by the Free Software
* Foundation and appearing in the file LICENSE.GPL included in
* the packaging of this file. Please review the following information
* to ensure GNU General Public Licensing requirements will be met:
* http://www.fsf.org/licensing/licenses/info/GPLv2.html and
* http://www.gnu.org/copyleft/gpl.html.
*/

#include <texteditor/textdocumentlayout.h>
#include <texteditor/syntaxhighlighter.h>
#include <texteditor/codeassist/keywordscompletionassist.h>
#include <Lola/LlLexer.h>

namespace Ll
{
    class Highlighter1 : public TextEditor::SyntaxHighlighter
    {
    public:
        enum { TokenProp = QTextFormat::UserProperty };
        explicit Highlighter1(QTextDocument *parent = 0);

    protected:
        QTextCharFormat formatForCategory(int) const;

        // overrides
        void highlightBlock(const QString &text);

    private:
        enum Category { C_Num, C_Str, C_Kw, C_Type, C_Ident, C_Op, C_Pp, C_Cmt, C_Section, C_Brack, C_Max };
        QTextCharFormat d_format[C_Max];
    };

    class Highlighter2 : public TextEditor::SyntaxHighlighter
    {
    public:
        enum ProfileFormats {
            ProfileVariableFormat,
            ProfileFunctionFormat,
            ProfileCommentFormat,
            ProfileVisualWhitespaceFormat,
            NumProfileFormats
        };

        explicit Highlighter2(const TextEditor::Keywords &keywords);
        void highlightBlock(const QString &text);

    private:
        const TextEditor::Keywords m_keywords;
    };
}

#endif // LLHIGHLIGHTER_H
