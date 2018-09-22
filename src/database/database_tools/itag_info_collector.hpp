
#ifndef ITAG_INFO_COLLECTOR_HPP
#define ITAG_INFO_COLLECTOR_HPP

#include <functional>

#include <QObject>

#include <core/tag.hpp>


struct ITagInfoCollector: public QObject
{
    virtual ~ITagInfoCollector() = default;

    virtual const std::vector<TagValue>& get(const TagNameInfo &) const = 0;

    Q_OBJECT

signals:
    void setOfValuesChanged(const TagNameInfo &);
};

#endif
