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

#include "LlHoverHandler.h"
#include <QTextCursor>
#include <texteditor/texteditor.h>
#include <projectexplorer/projecttree.h>
#include <projectexplorer/project.h>
#include "LlModelManager.h"
#include "LlEditor.h"
#include <Lola/LlCrossRefModel.h>
#include <utils/fileutils.h>
#include <QTextBlock>
#include <QTextStream>
#include <QtDebug>
using namespace Ll;

HoverHandler::HoverHandler()
{

}

static inline QByteArray escape( QByteArray str )
{
    return str.trimmed();
}

void HoverHandler::identifyMatch(TextEditor::TextEditorWidget* editorWidget, int pos)
{
    QString text = editorWidget->extraSelectionTooltip(pos);

    if( !text.isEmpty() )
    {
        setToolTip(text);
        return;
    }
    // else

    QTextCursor cur(editorWidget->document());
    cur.setPosition(pos);

    const QString file = editorWidget->textDocument()->filePath().toString();
    CrossRefModel* mdl = ModelManager::instance()->getModelForCurrentProjectOrDirPath(file);

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
            CrossRefModel::TreePath path = mdl->findSymbolBySourcePos( file, line, col );
            if( path.isEmpty() )
                return;
            CrossRefModel::IdentDeclRef decl = mdl->findDeclarationOfSymbol(path.first().data());
            if( decl.data() == 0 )
                return;
            // qDebug() << "declared" << decl->decl()->tok().d_sourcePath << decl->decl()->tok().d_lineNr;
            FileCache* fcache = ModelManager::instance()->getFileCache();
            const QByteArray line = fcache->fetchTextLineFromFile(
                        decl->decl()->tok().d_sourcePath, decl->decl()->tok().d_lineNr );
            QTextStream out(&text);
            QStringList parts = CrossRefModel::qualifiedNameParts(path,true);
            for( int l = 0; l < parts.size(); l++ )
            {
                if( l != 0 )
                    out << endl << QString(l*3,QChar(' ')) << QChar('.');
                out << parts[l];
            }
            int len = decl->decl()->tok().d_len;
            if( len == 0 )
                len = -1;
            // TODO: für Ports in alter Delkaration nicht optimal
            out << endl << QString::fromLatin1(line.mid(decl->decl()->tok().d_colNr-1,len).simplified())
                << " " << QString::fromLatin1(decl->tok().d_val);
            if( decl->decl()->tok().d_sourcePath != file )
                out << endl << tr("declared in ") << QFileInfo(decl->decl()->tok().d_sourcePath).fileName();
            setToolTip( text );
            // NOTE: mit html funktioniert es nicht!
        }
    }
}

