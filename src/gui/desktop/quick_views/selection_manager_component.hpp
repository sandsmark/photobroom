
#ifndef SELECTIONMANAGERCOMPONENT_HPP
#define SELECTIONMANAGERCOMPONENT_HPP


#include <QObject>
#include <QList>
#include <set>


class SelectionManagerComponent: public QObject
{
        Q_OBJECT

    public:
        explicit SelectionManagerComponent(QObject * = nullptr);

        Q_INVOKABLE void toggleIndexSelection(int);
        Q_INVOKABLE void clearSelection();
        Q_INVOKABLE void selectTo(int);
        Q_INVOKABLE bool isIndexSelected(int) const;
        Q_INVOKABLE QList<int> selected() const;

    private:
        std::set<int> m_selected;
        int m_previouslySelected;

        void calculateChange(const std::set<int> &, const std::set<int> &);

    signals:
        void selectionChanged(const QList<int>& unselected,
                              const QList<int>& selected) const;
};

#endif // SELECTIONMANAGERCOMPONENT_HPP
