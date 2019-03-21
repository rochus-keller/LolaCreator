#ifndef LLMODELMANAGER_H
#define LLMODELMANAGER_H

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

#include <QObject>
#include <QHash>
#include <Lola/LlFileCache.h>
#include <Lola/LlCrossRefModel.h>

namespace Ll
{
    class ModelManager : public QObject
    {
        Q_OBJECT
    public:
        explicit ModelManager(QObject *parent = 0);
        ~ModelManager();

        CrossRefModel* getModelForFile(const QString& fileName );
        CrossRefModel* getModelForDir(const QString& dirPath , bool initIfEmpty = false);
        CrossRefModel* getModelForCurrentProject();
        CrossRefModel* getModelForCurrentProjectOrDirPath(const QString& dirPath , bool initIfEmpty = false);
        CrossRefModel* getLastUsed() const { return d_lastUsed; }
        QString getPathOf(CrossRefModel*) const;

        FileCache* getFileCache() const { return d_fcache; }

        static ModelManager* instance();

    protected slots:
        void onModelUpdated();

    private:
        static ModelManager* d_inst;
        QHash<QString,CrossRefModel*> d_models; // Project File -> Code Model
        QHash<CrossRefModel*,QString> d_paths;
        CrossRefModel* d_lastUsed;
        FileCache* d_fcache;
    };
}

#endif // LLMODELMANAGER_H
