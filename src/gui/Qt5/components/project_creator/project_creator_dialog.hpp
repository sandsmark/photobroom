/*
 * Dialog for new projects creation
 * Copyright (C) 2014  Michał Walenciak <MichalWalenciak@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef PROJECTCREATOR_H
#define PROJECTCREATOR_H

#include <QDialog>

class QComboBox;
class QtExtChooseFile;
class QLineEdit;
class QGroupBox;

struct IPluginLoader;
struct IProjectManager;

namespace Database
{
    struct IPlugin;
}

class ProjectCreatorDialog: public QDialog
{
    public:
        ProjectCreatorDialog();
        ProjectCreatorDialog(const ProjectCreatorDialog& other) = delete;
        ~ProjectCreatorDialog();

        ProjectCreatorDialog& operator=(const ProjectCreatorDialog& other) = delete;

        void set(IPluginLoader *);

        // getters:
        QString getPrjPath() const;
        Database::IPlugin* getEnginePlugin() const;

    private:
        QtExtChooseFile* m_chooseDialog;
        QLineEdit* m_prjLocation;
        QComboBox* m_engines;
        QGroupBox* m_engineOptions;
        IPluginLoader* m_pluginLoader;
        std::map<QString, Database::IPlugin *> m_plugins;

        void initEngines();
        Database::IPlugin* getSelectedPlugin() const;
};


//runs ProjectCreator and adds project to IProjectManager
class ProjectCreator
{
    public:
        ProjectCreator();

        bool create(IProjectManager *, IPluginLoader *);
        QString prjName() const;

    private:
        QString m_prjName;
};

#endif // PROJECTCREATOR_H
