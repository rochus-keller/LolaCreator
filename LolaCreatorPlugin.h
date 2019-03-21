#ifndef LOLACREATOR_H
#define LOLACREATOR_H

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

#include "LolaCreatorGlobal.h"

#include <extensionsystem/iplugin.h>

namespace Ll { class EditorWidget1; }
class QAction;

namespace LolaCreator {
    namespace Internal {

        class LolaCreatorPlugin : public ExtensionSystem::IPlugin
        {
            Q_OBJECT
            Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "LolaCreator.json")

        public:
            LolaCreatorPlugin();
            ~LolaCreatorPlugin();

            static LolaCreatorPlugin* instance();

            bool initialize(const QStringList &arguments, QString *errorString);
            void extensionsInitialized();
            ShutdownFlag aboutToShutdown();

        public slots:
            void onFindUsages();
            void onGotoOuterBlock();
            void onReloadProject();

        protected:
            Ll::EditorWidget1* currentEditorWidget();
            void initializeToolsSettings();
            static LolaCreatorPlugin* d_instance;
        private:
            QAction* d_findUsagesAction;
            QAction* d_gotoOuterBlockAction;
            QAction* d_reloadProject;
        };

    } // namespace Internal
} // namespace LolaCreator

#endif // LOLACREATOR_H

