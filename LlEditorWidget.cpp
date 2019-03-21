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

#include "LlEditorWidget.h"
#include "LolaCreatorConstants.h"
#include "LlEditor.h"
#include "LlModelManager.h"
#include "LlOutlineMdl.h"
#include <Lola/LlCrossRefModel.h>
#include <Lola/LlErrors.h>
#include <Lola/LlSynTree.h>
#include <utils/fileutils.h>
#include <projectexplorer/projecttree.h>
#include <projectexplorer/project.h>
#include <projectexplorer/taskhub.h>
#include <texteditor/textdocumentlayout.h>
#include <texteditor/texteditorconstants.h>
#include <texteditor/fontsettings.h>
#include <coreplugin/find/searchresultwindow.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/editormanager/editormanager.h>
#include <QtDebug>
#include <QTime>
#include <QTextBlock>
#include <QMenu>
using namespace Ll;

// TODO const Core::IDocument *currentDocument = Core::EditorManager::currentDocument();

typedef QList<QTextEdit::ExtraSelection> ExtraSelections;

EditorWidget1::EditorWidget1():d_outline(0)
{
}

EditorWidget1::~EditorWidget1()
{
}

void EditorWidget1::finalizeInitialization()
{
    // d_document = qobject_cast<EditorDocument*>(textDocument());
    setLanguageSettingsId(LolaCreator::Constants::SettingsId);

    connect( textDocument(), SIGNAL(filePathChanged(Utils::FileName,Utils::FileName)), this, SLOT(onDocReady()) );
    connect( this, SIGNAL(cursorPositionChanged()), this, SLOT(onCursor()) );
    connect( qobject_cast<EditorDocument1*>(textDocument()), SIGNAL(sigStartProcessing()),
             this, SLOT(onStartProcessing()) );

    // edit textChanged wie doc chantedContents
    // edit undoAvailable wie doc changed

    d_outline = new Utils::TreeViewComboBox();
    d_outline->setMinimumContentsLength(22);
    QSizePolicy policy = d_outline->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Expanding);
    d_outline->setSizePolicy(policy);
    d_outline->setMaxVisibleItems(40);

    d_outline->setContextMenuPolicy(Qt::ActionsContextMenu);
    connect(d_outline, SIGNAL(activated(int)), this, SLOT(gotoSymbolInEditor()));
    connect(d_outline, SIGNAL(currentIndexChanged(int)), this, SLOT(updateToolTip()));

    OutlineMdl1* outline = new OutlineMdl1(this);
    d_outline->setModel(outline);
    connect( outline, SIGNAL(modelReset()), this, SLOT(onCursor()) );

    insertExtraToolBarWidget(TextEditorWidget::Left, d_outline );

}

static bool lessThan1(const CrossRefModel::SymRef &s1, const CrossRefModel::SymRef &s2)
{
    return s1->tok().d_lineNr < s2->tok().d_lineNr ||
            ( !(s2->tok().d_lineNr < s1->tok().d_lineNr) && s1->tok().d_colNr < s2->tok().d_colNr );
}

void EditorWidget1::onFindUsages()
{
    QTextCursor cur = textCursor();
    const QString file = textDocument()->filePath().toString();
    CrossRefModel* mdl = ModelManager::instance()->getModelForCurrentProjectOrDirPath(file);
    Q_ASSERT( mdl != 0);

    const int line = cur.blockNumber() + 1;
    const int col = cur.columnNumber() + 1;

    CrossRefModel::TreePath path = mdl->findSymbolBySourcePos( file, line, col );
    if( path.isEmpty() )
        return;

    const CrossRefModel::IdentDecl* id = path.first()->toIdentDecl();
    if( id == 0 )
        // es wurde auf normalen Ident geklickt, nicht auf Declaration; wir suchen also die Decl dazu
        id = mdl->findDeclarationOfSymbol( path.first().data() ).data();
    if( id == 0 )
        return;
    CrossRefModel::SymRefList res = mdl->findAllReferencingSymbols( id ); // hier ohne file filter!
    res.append(CrossRefModel::SymRef(id));
    std::sort(res.begin(), res.end(), lessThan1 );

    Core::SearchResult *search = Core::SearchResultWindow::instance()->startNewSearch(tr("Lola Usages:"),
                                                QString(),
                                                CrossRefModel::qualifiedName(path),
                                                Core::SearchResultWindow::SearchOnly,
                                                Core::SearchResultWindow::PreserveCaseDisabled,
                                                QLatin1String("LolaEditor"));

    FileCache* fcache = ModelManager::instance()->getFileCache();
    foreach( const CrossRefModel::SymRef& st, res )
    {
        search->addResult( st->tok().d_sourcePath,
                          st->tok().d_lineNr,
                          fcache->fetchTextLineFromFile( st->tok().d_sourcePath, st->tok().d_lineNr, st->tok().d_val ), // TODO: ev. effizienter
                          st->tok().d_colNr - 1,
                          st->tok().d_len);
    }
    search->finishSearch(false);
    connect(search, SIGNAL(activated(Core::SearchResultItem)),
            this, SLOT(onOpenEditor(Core::SearchResultItem)));

    Core::SearchResultWindow::instance()->popup(Core::IOutputPane::ModeSwitch | Core::IOutputPane::WithFocus);
    search->popup();
}

void EditorWidget1::onGotoOuterBlock()
{
    QTextCursor cur = textCursor();
    const QString file = textDocument()->filePath().toString();
    CrossRefModel* mdl = ModelManager::instance()->getModelForCurrentProjectOrDirPath(file);
    if( mdl == 0 )
        return;

    const int line = cur.blockNumber() + 1;
    const int col = cur.columnNumber() + 1;

    CrossRefModel::TreePath path = mdl->findSymbolBySourcePos( file, line, col, false );
//    for( int i = 0; i < path.size(); i++ )
//        qDebug() << "path" << i << path[i]->getTypeName() << SynTree::rToStr(path[i]->tok().d_type) << path[i]->tok().d_val;
    for( int i = 1; i < path.size(); i++ )
    {
        if( !( path[i]->tok().d_lineNr == path[0]->tok().d_lineNr &&
                path[i]->tok().d_colNr == path[0]->tok().d_colNr ) &&
                //path[i]->tok().d_type != SynTree::R_module_or_udp_instantiation &&
                path[i]->tok().d_sourcePath == path[i]->tok().d_sourcePath )
        {
            Core::EditorManager::cutForwardNavigationHistory();
            Core::EditorManager::addCurrentPositionToNavigationHistory();
            gotoLine( path[i]->tok().d_lineNr, path[i]->tok().d_colNr - 1, false );
            break;
        }
    }
}

void EditorWidget1::onFileUpdated(const QString& path)
{
    const QString file = textDocument()->filePath().toString();
    if( file == path )
    {
        onUpdateCodeWarnings();
    }
}

void EditorWidget1::onStartProcessing()
{
    setExtraSelections( TextEditor::TextEditorWidget::CodeSemanticsSelection, ExtraSelections() );
    setExtraSelections( TextEditor::TextEditorWidget::CodeWarningsSelection, ExtraSelections() );
}

static bool lessThan2(const QTextEdit::ExtraSelection &s1, const QTextEdit::ExtraSelection &s2)
{
    return s1.cursor.position() < s2.cursor.position();
}

void EditorWidget1::onUpdateCodeWarnings()
{
    const QString file = textDocument()->filePath().toString();
    CrossRefModel* mdl = ModelManager::instance()->getModelForCurrentProjectOrDirPath(file);
    Q_ASSERT( mdl != 0 );
    QTextDocument* doc = textDocument()->document();

    Errors::EntryList errs = mdl->getErrs()->getErrors(file);

    QTextCharFormat errorFormat;
    errorFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
    errorFormat.setUnderlineColor(Qt::red);

    QTextCharFormat warningFormat;
    warningFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
    warningFormat.setUnderlineColor(Qt::darkYellow);

    ExtraSelections result;

    foreach (const Errors::Entry& e, errs)
    {
        QTextCursor c( doc->findBlockByNumber(e.d_line - 1) );

        c.setPosition( c.position() + e.d_col - 1 );
        c.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);

        QTextEdit::ExtraSelection sel;
        sel.format = errorFormat;
        sel.cursor = c;
        QString what;
        switch( e.d_source )
        {
        case Errors::Lexer:
            what = tr("lexical error");
            break;
        case Errors::Syntax:
            what = tr("syntax error");
            break;
        case Errors::Semantics:
            what = tr("semantic error");
            break;
        case Errors::Elaboration:
            what = tr("elaboration error");
            break;
        }

        sel.format.setToolTip(QString("%1: %2").arg(what).arg(e.d_msg));

        result.append(sel);
    }

    std::sort(result.begin(), result.end(), lessThan2);

    setExtraSelections( TextEditor::TextEditorWidget::CodeWarningsSelection, result );
}

TextEditor::TextEditorWidget::Link EditorWidget1::findLinkAt(const QTextCursor& cur, bool resolveTarget, bool inNextSplit)
{
    Q_UNUSED(resolveTarget);
    Q_UNUSED(inNextSplit);

    const QString file = textDocument()->filePath().toString();
    CrossRefModel* mdl = ModelManager::instance()->getModelForCurrentProjectOrDirPath(file);
    Q_ASSERT( mdl != 0 );

    const int line = cur.blockNumber() + 1;
    const int col = cur.columnNumber() + 1;

    int tokPos;
    QList<Token> toks = CrossRefModel::findTokenByPos( cur.block().text(), col, &tokPos,
                                                       mdl->getFcache()->supportSvExt(file) );
    if( tokPos != -1 )
    {
        const Token& t = toks[tokPos];
        if( t.d_type == Tok_identifier )
        {
            CrossRefModel::IdentDeclRef id = mdl->findDeclarationOfSymbolAtSourcePos( file, line, col );
            if( id.data() == 0 )
                return Link();
            Link l( id->tok().d_sourcePath, id->tok().d_lineNr, id->tok().d_colNr - 1 );
            const int off = col - t.d_colNr;
            l.linkTextStart = cur.position() - off;
            l.linkTextEnd = cur.position() - off + t.d_len;
            return l;
        }
    }
    return Link();
}

void EditorWidget1::contextMenuEvent(QContextMenuEvent* e)
{
    QPointer<QMenu> menu(new QMenu(this));

    Core::ActionContainer *mcontext = Core::ActionManager::actionContainer(LolaCreator::Constants::EditorContextMenuId1);
    QMenu *contextMenu = mcontext->menu();

    foreach (QAction *action, contextMenu->actions()) {
        menu->addAction(action);
//        if (action->objectName() == QLatin1String(Constants::M_REFACTORING_MENU_INSERTION_POINT))
//            menu->addMenu(quickFixMenu);
    }

    appendStandardContextMenuActions(menu);

    menu->exec(e->globalPos());
    if (!menu)
        return;
    //d->m_quickFixes.clear();
    delete menu;
}

static ExtraSelections toExtraSelections(const CrossRefModel::SymRefList& uses,
                                                           TextEditor::TextStyle style, TextEditor::TextDocument* document )
{
    ExtraSelections result;

    QTextDocument* doc = document->document();

    foreach (const CrossRefModel::SymRef& use, uses)
    {
        const int position = doc->findBlockByNumber(use->tok().d_lineNr - 1).position() +
                use->tok().d_colNr - 1;
        const int anchor = position + use->tok().d_len;

        QTextEdit::ExtraSelection sel; //
        sel.format = document->fontSettings().toTextCharFormat(style);
        sel.cursor = QTextCursor(doc);
        sel.cursor.setPosition(anchor);
        sel.cursor.setPosition(position, QTextCursor::KeepAnchor);

        result.append(sel);
    }

    std::sort(result.begin(), result.end(), lessThan2);
    return result;
}

void EditorWidget1::onCursor()
{
    QTextCursor cur = textCursor();
    const QString file = textDocument()->filePath().toString();
    CrossRefModel* mdl = ModelManager::instance()->getModelForCurrentProjectOrDirPath(file);
    if( mdl == 0 )
        return;

    const int line = cur.blockNumber() + 1;
    const int col = cur.columnNumber() + 1;

    CrossRefModel::TreePath path = mdl->findSymbolBySourcePos( file, line, col );
    if( !path.isEmpty() )
    {
        // mark all symbol references in text
        CrossRefModel::IdentDeclRef id( path.first()->toIdentDecl() );
        if( id.data() == 0 )
            // es wurde auf normalen Ident geklickt, nicht auf Declaration; wir suchen also die Decl dazu
            id = mdl->findDeclarationOfSymbol( path.first().data() );
        if( id.data() != 0 )
        {
            CrossRefModel::SymRefList res = mdl->findReferencingSymbolsByFile( id.data(), file );
            if( id->tok().d_sourcePath == file )
                res.append(id);
            // qDebug() << "******* hit on" << id->tok().d_val;
//            foreach( const CrossRefModel::SymRef& ref, res )
//                qDebug() << QFileInfo(ref->tok().d_sourcePath).fileName() << ref->tok().d_lineNr << ref->tok().d_colNr;
            setExtraSelections(TextEditor::TextEditorWidget::CodeSemanticsSelection,
                               toExtraSelections(res, TextEditor::C_OCCURRENCES, textDocument() ) );

        }else
            setExtraSelections(TextEditor::TextEditorWidget::CodeSemanticsSelection, ExtraSelections() );
    }

//    path = mdl->findSymbolBySourcePos( file, line, col, false, true );
//    if( !path.isEmpty() )
//        qDebug() << "***** hit area in" << SynTree::rToStr(path.first()->tok().d_type) << path.first()->tok().d_val;

    if( d_outline && d_outline->model() )
    {
        // update outline menu selection
        OutlineMdl1* mdl2 = static_cast<OutlineMdl1*>( d_outline->model() );
        QModelIndex i;
        foreach( const CrossRefModel::SymRef& s, path )
        {
            i = mdl2->findSymbol( s.data() );
            if( i.isValid() )
            {
                emit sigGotoSymbol( s->tok().d_lineNr, s->tok().d_colNr );
                break;
            }
        }
        if( !i.isValid() )
            i = mdl2->index(0,0);
        const bool blocked = d_outline->blockSignals(true);
        d_outline->setCurrentIndex(i);
        updateToolTip();
        d_outline->blockSignals(blocked);
    }
}

void EditorWidget1::onOpenEditor(const Core::SearchResultItem& item)
{
    Core::EditorManager::openEditorAt( item.path.first(), item.lineNumber, item.textMarkPos);
}

void EditorWidget1::onDocReady()
{
    const QString fileName = textDocument()->filePath().toString();
    CrossRefModel* mdl = ModelManager::instance()->getModelForCurrentProjectOrDirPath( fileName, true);
                // in case there is no project create one with current file path and parse each Verilog file
                // found there
    Q_ASSERT(mdl != 0 );
    connect( mdl, SIGNAL(sigFileUpdated(QString)), this, SLOT(onFileUpdated(QString)) );

    OutlineMdl1* outline = static_cast<OutlineMdl1*>( d_outline->model() );
    outline->setFile(fileName);

    if( !mdl->isEmpty() )
    {
        onUpdateCodeWarnings();
    }
}

void EditorWidget1::gotoSymbolInEditor()
{
    OutlineMdl1* mdl = static_cast<OutlineMdl1*>( d_outline->model() );
    const QModelIndex modelIndex = d_outline->view()->currentIndex();

    const CrossRefModel::Symbol* sym = mdl->getSymbol(modelIndex);
    if( sym )
    {
        Core::EditorManager::cutForwardNavigationHistory();
        Core::EditorManager::addCurrentPositionToNavigationHistory();
        gotoLine( sym->tok().d_lineNr, sym->tok().d_colNr - 1 );
        emit sigGotoSymbol( sym->tok().d_lineNr, sym->tok().d_colNr );
        activateEditor();
    }
}

void EditorWidget1::updateToolTip()
{
    d_outline->setToolTip(d_outline->currentText());
}

EditorWidget2::EditorWidget2()
{

}

void EditorWidget2::contextMenuEvent(QContextMenuEvent* e)
{
    QPointer<QMenu> menu(new QMenu(this));

    Core::ActionContainer *mcontext = Core::ActionManager::actionContainer(LolaCreator::Constants::EditorContextMenuId2);
    QMenu *contextMenu = mcontext->menu();

    foreach (QAction *action, contextMenu->actions()) {
        menu->addAction(action);
    }

    appendStandardContextMenuActions(menu);

    menu->exec(e->globalPos());
    if (!menu)
        return;
    delete menu;
}
