/*
    TagEntry class which is responsible for displaying information
    about single tag.
    Copyright (C) 2013  Michał Walenciak <MichalWalenciak@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#ifndef TAG_ENTRY_H
#define TAG_ENTRY_H

#include <QString>
#include <QWidget>

#include <core/types.hpp>

class QLabel;
class QLineEdit;
class IValueWidget;


struct TagInfo
{
    TagInfo(const std::initializer_list<QString>& data);
    explicit TagInfo(const ITagData::TagInfo &);
    TagInfo(const QString &, const std::string &);
    
    QString name;
    std::string typeInfo;

    static std::string defaultType();
    
    bool operator==(const TagInfo& other) const;    
    bool operator<(const TagInfo& other) const;
};


class TagEntry: public QWidget
{
        Q_OBJECT
            
    public:
        friend class EntriesManager;
        
        virtual ~TagEntry();

        TagEntry(const TagEntry &) = delete;
        void operator=(const TagEntry &) = delete;
        
        //void selectTag(const QString &name);
        void setTagValue(const QString &value);
        void clear();
        
        TagInfo getTagInfo() const;
        QString getTagValue() const;

    private:
        QLabel       *m_tagName;
        IValueWidget *m_tagValue;
        TagInfo      m_tagInfo;
        
        explicit TagEntry(const TagInfo &, QWidget *, Qt::WindowFlags f = 0);
    
    signals:
        void tagEdited();
};

#endif // TAG_ENTRY_H
