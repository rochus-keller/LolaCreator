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

#include "LlEditor.h"
#include "LlEditorWidget.h"
#include "LlHoverHandler.h"
#include "LlIndenter.h"
#include "LolaCreatorConstants.h"
#include <QtDebug>
#include <Lola/LlCrossRefModel.h>
#include <coreplugin/idocument.h>
#include <utils/fileutils.h>
#include "LlModelManager.h"
#include "LlHighlighter.h"
#include "LlAutoCompleter.h"
#include "LlCompletionAssistProvider.h"
#include <texteditor/texteditoractionhandler.h>
#include <texteditor/texteditorsettings.h>
#include <texteditor/codeassist/keywordscompletionassist.h>
using namespace Ll;

static const int s_processIntervalMs = 150;

Editor1::Editor1()
{
    addContext(LolaCreator::Constants::LangLola);
}

Editor2::Editor2()
{
    addContext(LolaCreator::Constants::LangQmake);
}

EditorDocument1::EditorDocument1():d_opening(false)
{
    setId(LolaCreator::Constants::EditorId1);

    connect( this, SIGNAL(contentsChanged()), this, SLOT(onChangedContents()) );
    d_processorTimer.setSingleShot(true);
    d_processorTimer.setInterval(s_processIntervalMs);
    connect(&d_processorTimer, SIGNAL(timeout()), this, SLOT(onProcess()));

}

EditorDocument1::~EditorDocument1()
{
    ModelManager::instance()->getFileCache()->removeFile( filePath().toString() );
}

TextEditor::TextDocument::OpenResult EditorDocument1::open(QString* errorString, const QString& fileName, const QString& realFileName)
{
    d_opening = true;
    const TextDocument::OpenResult res = TextDocument::open(errorString, fileName, realFileName );
    // wird nach EditorWidget::finalizeInitialization aufgerufen!
    d_opening = false;
    return res;
}

bool EditorDocument1::save(QString* errorString, const QString& fileName, bool autoSave)
{
    const bool res = TextDocument::save(errorString,fileName, autoSave);
    if( !autoSave )
        ModelManager::instance()->getFileCache()->removeFile( filePath().toString() );
    return res;
}

void EditorDocument1::onChangedContents()
{
    if( d_opening )
        emit sigLoaded();
    else
        d_processorTimer.start(s_processIntervalMs);
}

void EditorDocument1::onProcess()
{
    emit sigStartProcessing();
    const QString file = filePath().toString();
    ModelManager::instance()->getFileCache()->addFile( file, plainText().toLatin1() );
    CrossRefModel* mdl = ModelManager::instance()->getModelForCurrentProject();
    if( mdl == 0 )
        mdl = ModelManager::instance()->getModelForDir(file);
    mdl->updateFiles( QStringList() << file );
}

EditorDocument2::EditorDocument2()
{
    setId(LolaCreator::Constants::EditorId2);
}

EditorFactory1::EditorFactory1()
{
    setId(LolaCreator::Constants::EditorId1);
    setDisplayName(qApp->translate("OpenWith::Editors", LolaCreator::Constants::EditorDisplayName1));
    addMimeType(LolaCreator::Constants::MimeType);

    setDocumentCreator([]() { return new EditorDocument1; });
    setIndenterCreator([]() { return new Indenter; });
    setEditorWidgetCreator([]() { return new EditorWidget1; });
    setEditorCreator([]() { return new Editor1; });
    setAutoCompleterCreator([]() { return new AutoCompleter; });
    setCompletionAssistProvider(new CompletionAssistProvider);
    setSyntaxHighlighterCreator([]() { return new Highlighter1; });
    setCommentStyle(Utils::CommentDefinition::CppStyle);
    setParenthesesMatchingEnabled(true);
    setCodeFoldingSupported(true);
    setMarksVisible(true);
    addHoverHandler(new HoverHandler);

    setEditorActionHandlers(TextEditor::TextEditorActionHandler::Format
                          | TextEditor::TextEditorActionHandler::UnCommentSelection
                          | TextEditor::TextEditorActionHandler::UnCollapseAll
                          | TextEditor::TextEditorActionHandler::FollowSymbolUnderCursor
                            );
}

EditorFactory2::EditorFactory2()
{
    setId(LolaCreator::Constants::EditorId2);
    setDisplayName(qApp->translate("OpenWith::Editors", LolaCreator::Constants::EditorDisplayName2));
    addMimeType(LolaCreator::Constants::ProjectMimeType);

    setDocumentCreator([]() { return new EditorDocument2; });
    setIndenterCreator([]() { return new Indenter; });
    setEditorWidgetCreator([]() { return new EditorWidget2; });
    setEditorCreator([]() { return new Editor2; });
    //setAutoCompleterCreator([]() { return new AutoCompleter; });
    //setCompletionAssistProvider(new CompletionAssistProvider);

    QStringList vars;
    vars << "INCDIRS" << "DEFINES" << "LIBDIRS" << "LIBFILES" << "LIBEXT"
               << "SRCDIRS" << "SRCFILES" << "SRCEXT" << "CONFIG" << "TOPMOD"
               << "BUILD_UNDEFS" << "VLTR_UNDEFS" << "VLTR_ARGS" << "YOSYS_UNDEFS" << "YOSYS_CMDS";
    vars.sort();
    QStringList funcs;
    funcs << "member" << "first" << "last" << "cat" << "fromfile" << "eval" << "list" << "sprintf"
                  << "join" << "split" << "basename" << "dirname" << "section" << "find" << "system"
                  << "unique" << "quote" << "escape_expand" << "upper" << "lower" << "re_escape"
                  << "files" << "prompt" << "replace" << "requires" << "greaterThan" << "lessThan"
                  << "equals" << "isEqual" << "exists" << "export" << "clear" << "unset" << "eval"
                  << "CONFIG" << "if" << "isActiveConfig" << "system" << "return" << "break"
                  << "next" << "defined" << "contains" << "infile" << "count" << "isEmpty"
                  << "include" << "load" << "debug" << "error" << "message" << "warning";
    funcs.sort();

    TextEditor::Keywords keywords( vars, funcs , QMap<QString, QStringList>() );
    setSyntaxHighlighterCreator([keywords]() { return new Highlighter2(keywords); });
    setCommentStyle(Utils::CommentDefinition::HashStyle);
    setParenthesesMatchingEnabled(true);
    setCodeFoldingSupported(true);
    setMarksVisible(true);
    addHoverHandler(new HoverHandler);

    setEditorActionHandlers(TextEditor::TextEditorActionHandler::Format
                          | TextEditor::TextEditorActionHandler::UnCommentSelection
                          | TextEditor::TextEditorActionHandler::UnCollapseAll
                          | TextEditor::TextEditorActionHandler::FollowSymbolUnderCursor
                            );
}
