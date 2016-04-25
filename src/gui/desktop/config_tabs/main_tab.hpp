#ifndef MAIN_TAB_HPP
#define MAIN_TAB_HPP

#include <QWidget>

#include <ui_utils/iconfig_dialog_manager.hpp>

class QCheckBox;
class QComboBox;

struct IConfiguration;

namespace Ui
{
    class MainTab;
}

class MainTab: public QWidget
{
        Q_OBJECT

    public:
        explicit MainTab(QWidget *parent = 0);
        MainTab(const MainTab &) = delete;
        ~MainTab();

        MainTab& operator=(const MainTab &) = delete;

        QCheckBox* updateCheckBox();
        QComboBox* updateFrequency();

    private:
        Ui::MainTab *ui;
};


class MainTabControler: public QObject, public IConfigTab
{
        Q_OBJECT

    public:
        MainTabControler();
        MainTabControler(const MainTabControler &) = delete;
        virtual ~MainTabControler();

        void set(IConfiguration *);

        MainTabControler& operator=(const MainTabControler &) = delete;

        int tabId() const override;
        QString tabName() const override;

        QWidget* constructTab() override;
        void applyConfiguration() override;
        void rejectConfiguration() override;

    private:
        IConfiguration* m_configuration;
        MainTab* m_tabWidget;
};


#endif // MAIN_TAB_HPP