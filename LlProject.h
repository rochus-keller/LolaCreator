#ifndef LLPROJECT_H
#define LLPROJECT_H

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

#include <projectexplorer/project.h>
#include <projectexplorer/iprojectmanager.h>

#include <QFileSystemWatcher>

namespace TextEditor { class TextDocument; }
namespace ProjectExplorer { class FolderNode; }

namespace Ll
{
    class ProjectManager;
    class ProjectNode;

    class Project : public ProjectExplorer::Project
    {
        Q_OBJECT
    public:
        static const char* ID;

        explicit Project(ProjectManager *projectManager, const QString &fileName);

        const QStringList& getSrcFiles() const { return d_srcFiles; }
        const QStringList& getLibFiles() const { return d_libFiles; }
        QStringList getConfig( const QString& key ) const;
        QStringList getIncDirs() const;
        QString getTopMod() const;
        void reload();

        // overrides
        QString displayName() const Q_DECL_OVERRIDE;
        Core::IDocument* document() const Q_DECL_OVERRIDE;
        ProjectExplorer::IProjectManager *projectManager() const Q_DECL_OVERRIDE;
        ProjectExplorer::ProjectNode *rootProjectNode() const Q_DECL_OVERRIDE;
        QStringList files(FilesMode) const Q_DECL_OVERRIDE;
    protected:
        void populateDir( const QDir&, ProjectExplorer::FolderNode* );
        void loadProject( const QString& fileName );
        static void findFilesInDirs( const QStringList& dirs, const QStringList& filter, QSet<QString>& files );
        static void findFilesInDir(const QString& dir, const QStringList& filter, QStringList& files , bool recursive);
        static void fillNode( const QStringList& files, ProjectExplorer::FolderNode* );

        RestoreResult fromMap(const QVariantMap &map, QString *errorMessage) Q_DECL_OVERRIDE;
    protected slots:
        void onFileChanged(const QString& path);
    private:
        ProjectManager* d_projectManager;
        TextEditor::TextDocument* d_document;
        ProjectNode* d_root;
        QString d_name;
        QStringList d_srcFiles, d_libFiles, d_incDirs;
        QMap<QString, QStringList> d_config;
        QFileSystemWatcher d_watcher;
    };

    class ProjectManager : public ProjectExplorer::IProjectManager
    {
        Q_OBJECT
    public:
        explicit ProjectManager();

        virtual QString mimeType() const;
        virtual ProjectExplorer::Project *openProject(const QString &fileName, QString *errorString);

    };

}

#endif // LLPROJECT_H
