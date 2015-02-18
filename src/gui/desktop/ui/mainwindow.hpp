
#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <memory>

#include <QMainWindow>

class StagedPhotosDataModel;
class PhotosCollector;
class PhotosAnalyzer;
struct ITaskExecutor;
struct IPluginLoader;
struct IProject;
struct IProjectManager;
struct IConfiguration;
struct IView;

class DBDataModel;
struct ProjectInfo;

namespace Ui
{
    class MainWindow;
}

class MainWindow final: public QMainWindow
{
        Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = 0);
        MainWindow(const MainWindow &) = delete;
        virtual ~MainWindow();

        MainWindow operator=(const MainWindow &) = delete;

        void set(IProjectManager *);
        void set(IPluginLoader *);
        void set(ITaskExecutor *);
        void set(IConfiguration *);

    private:
        Ui::MainWindow*           ui;
        IProjectManager*          m_prjManager;
        IPluginLoader*            m_pluginLoader;
        std::unique_ptr<IProject> m_currentPrj;
        DBDataModel*              m_imagesModel;
        StagedPhotosDataModel*    m_stagedImagesModel;
        IConfiguration*           m_configuration;
        PhotosCollector*          m_photosCollector;
        std::vector<IView *>      m_views;
        std::unique_ptr<PhotosAnalyzer> m_photosAnalyzer;

        void closeEvent(QCloseEvent *);
        void openProject(const ProjectInfo &);
        void closeProject();
        void setupView();
        void createMenus();
        void updateMenus();
        void updateGui();
        void updateTools();
        void viewChanged();

    private slots:
        // album menu
        void on_actionNew_project_triggered();
        void on_actionOpen_project_triggered();
        void on_actionClose_triggered();
        void on_actionQuit_triggered();
        
        // photos menu
        void on_actionAdd_photos_triggered();
        
        // window menu
        void activateWindow(QAction *);
        
        // help menu
        void on_actionHelp_triggered();
        void on_actionAbout_triggered();
        void on_actionAbout_Qt_triggered();
};

#endif // MAINWINDOW_HPP