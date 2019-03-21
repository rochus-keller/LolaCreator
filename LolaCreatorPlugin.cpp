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

#include "LolaCreatorPlugin.h"
#include "LolaCreatorConstants.h"
#include "LlModelManager.h"
#include "LlEditor.h"
#include "LlEditorWidget.h"
#include "LlModuleLocator.h"
#include "LlSymbolLocator.h"
#include "LlProject.h"

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <projectexplorer/taskhub.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/projecttree.h>
#include <texteditor/texteditorsettings.h>
#include <utils/mimetypes/mimedatabase.h>
#include <coreplugin/editormanager/editormanager.h>
#include <texteditor/texteditorconstants.h>

#include <QAction>
#include <QMessageBox>
#include <QMainWindow>
#include <QMenu>

#include <QtPlugin>
#include <QtDebug>

using namespace LolaCreator::Internal;

LolaCreatorPlugin* LolaCreatorPlugin::d_instance = 0;

LolaCreatorPlugin::LolaCreatorPlugin()
{
    d_instance = this;
}

LolaCreatorPlugin::~LolaCreatorPlugin()
{
    d_instance = 0;
}

bool LolaCreatorPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    Q_UNUSED(errorString);
    Q_UNUSED(arguments);

    Ll::ModelManager::instance();

    initializeToolsSettings();

    addAutoReleasedObject(new Ll::EditorFactory1);
    addAutoReleasedObject(new Ll::EditorFactory2);
    addAutoReleasedObject(new Ll::ModuleLocator);
    addAutoReleasedObject(new Ll::SymbolLocator);
    addAutoReleasedObject(new Ll::ProjectManager);
    /*
    addAutoReleasedObject(new Vl::OutlineWidgetFactory);
    addAutoReleasedObject(new Vl::MakeStepFactory);
    addAutoReleasedObject(new Vl::BuildConfigurationFactory);
    addAutoReleasedObject(new Vl::RunConfigurationFactory);
    */

    Core::Context context(LolaCreator::Constants::EditorId1);
    Core::ActionContainer *contextMenu1 = Core::ActionManager::createMenu(LolaCreator::Constants::EditorContextMenuId1);
    Core::ActionContainer * contextMenu2 = Core::ActionManager::createMenu(LolaCreator::Constants::EditorContextMenuId2);
    Core::Command *cmd;
    Core::ActionContainer * toolsMenu = Core::ActionManager::createMenu(LolaCreator::Constants::ToolsMenuId);
    toolsMenu->menu()->setTitle(tr("&Lola-2"));
    Core::ActionManager::actionContainer(Core::Constants::M_TOOLS)->addMenu(toolsMenu);


    cmd = Core::ActionManager::command(TextEditor::Constants::FOLLOW_SYMBOL_UNDER_CURSOR);
    contextMenu1->addAction(cmd);
    toolsMenu->addAction(cmd);

    d_findUsagesAction = new QAction(tr("Find Usages"), this);
    cmd = Core::ActionManager::registerAction(d_findUsagesAction, LolaCreator::Constants::FindUsagesCmd, context);
    cmd->setKeySequence(QKeySequence(tr("Ctrl+Shift+U")));
    connect(d_findUsagesAction, SIGNAL(triggered()), this, SLOT(onFindUsages()));
    contextMenu1->addAction(cmd);
    toolsMenu->addAction(cmd);

    d_gotoOuterBlockAction = new QAction(tr("Go to Outer Block"), this);
    cmd = Core::ActionManager::registerAction(d_gotoOuterBlockAction, LolaCreator::Constants::GotoOuterBlockCmd, context);
    cmd->setDefaultKeySequence(QKeySequence(tr("ALT+Up")));
    connect(d_gotoOuterBlockAction, SIGNAL(triggered()), this, SLOT(onGotoOuterBlock()));
    contextMenu1->addAction(cmd);
    toolsMenu->addAction(cmd);

    Core::Command *sep = contextMenu1->addSeparator();

    cmd = Core::ActionManager::command(TextEditor::Constants::AUTO_INDENT_SELECTION);
    contextMenu1->addAction(cmd);
    contextMenu2->addAction(cmd);

    cmd = Core::ActionManager::command(TextEditor::Constants::UN_COMMENT_SELECTION);
    contextMenu1->addAction(cmd);
    contextMenu2->addAction(cmd);

    ProjectExplorer::TaskHub::addCategory(LolaCreator::Constants::TaskId, tr("Lola Parser"));


    Core::ActionContainer *mproject = Core::ActionManager::actionContainer(ProjectExplorer::Constants::M_PROJECTCONTEXT);
    d_reloadProject = new QAction(tr("Reload Project"), this);
    cmd = Core::ActionManager::registerAction(d_reloadProject, LolaCreator::Constants::ReloadProjectCmd,
                                              Core::Context(ProjectExplorer::Constants::C_PROJECT_TREE));
    connect(d_reloadProject, SIGNAL(triggered()), this, SLOT(onReloadProject()));
    mproject->addAction(cmd, ProjectExplorer::Constants::G_PROJECT_FILES);

    return true;
}

void LolaCreatorPlugin::extensionsInitialized()
{
}

ExtensionSystem::IPlugin::ShutdownFlag LolaCreatorPlugin::aboutToShutdown()
{
    delete Ll::ModelManager::instance();
    return SynchronousShutdown;
}

void LolaCreatorPlugin::onFindUsages()
{
    if (Ll::EditorWidget1 *editorWidget = currentEditorWidget())
        editorWidget->onFindUsages();

}

void LolaCreatorPlugin::onGotoOuterBlock()
{
    if (Ll::EditorWidget1 *editorWidget = currentEditorWidget())
        editorWidget->onGotoOuterBlock();
}

void LolaCreatorPlugin::onReloadProject()
{
    Ll::Project* currentProject = dynamic_cast<Ll::Project*>( ProjectExplorer::ProjectTree::currentProject() );
    if( currentProject )
    {
        currentProject->reload();
    }
}

Ll::EditorWidget1*LolaCreatorPlugin::currentEditorWidget()
{
    return qobject_cast<Ll::EditorWidget1*>(Core::EditorManager::currentEditor()->widget());
}

void LolaCreatorPlugin::initializeToolsSettings()
{
    Utils::MimeDatabase::addMimeTypes(QLatin1String(":/Editor/Editor.mimetypes.xml"));

    TextEditor::TextEditorSettings::registerMimeTypeForLanguageId(LolaCreator::Constants::MimeType,
                                                                  LolaCreator::Constants::LangLola);
    TextEditor::TextEditorSettings::registerMimeTypeForLanguageId(LolaCreator::Constants::ProjectMimeType,
                                                                  LolaCreator::Constants::LangQmake);

}



