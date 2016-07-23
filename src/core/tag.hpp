
#ifndef TAG_HPP
#define TAG_HPP

#include <assert.h>

#include <deque>
#include <map>
#include <set>
#include <memory>

#include <QString>
#include <QVariant>

#include "core_export.h"


struct CORE_EXPORT TagNameInfo
{
        enum Type
        {
            //indexed, as those values will be stored in db.
            Invalid = 0,
            Text    = 1,
            Date    = 2,
            Time    = 3,
            List    = 4,
        };

        TagNameInfo();
        TagNameInfo(const QString& name, const Type, const QString& displayName);
        TagNameInfo(const QString& name, const Type);
        TagNameInfo(const QString& name, int type);
        TagNameInfo(const TagNameInfo& other);

        operator QString() const;
        bool operator==(const TagNameInfo& other) const;
        bool operator<(const TagNameInfo& other) const;
        bool operator>(const TagNameInfo& other) const;
        TagNameInfo& operator=(const TagNameInfo& other);

        const QString& getName() const;
        const QString& getDisplayName() const;
        Type getType() const;

    private:
        QString name;
        QString displayName;
        Type type;

        QString dn(const QString &) const;
};


class CORE_EXPORT TagValue
{
    public:
        enum class Type
        {
            Empty,
            Date,
            List,          // List of TagValue
            String,
            Time,
        };

        TagValue();
        TagValue(const TagValue &);
        explicit TagValue(const QVariant &);

        ~TagValue();

        TagValue& operator=(const TagValue &);

        [[deprecated]] void set(const QVariant &);
        void set(const QDate &);
        void set(const QTime &);
        void set(const std::deque<TagValue> &);
        void set(const QString &);

        [[deprecated]] QVariant get() const;
        QDate getDate() const;
        std::deque<TagValue> getList() const;
        QString getString() const;
        QTime getTime() const;

        Type type() const;

        bool operator==(const TagValue &) const;
        bool operator!=(const TagValue &) const;
        bool operator<(const TagValue &) const;

    private:
        Type m_type;
        void* m_value;

        void destroyValue();
        void copy(const TagValue &);

        template<typename T>
        bool validate() const;

        template<typename T>
        T* get() const
        {
            assert( validate<T>() );

            T* v = static_cast<T *>(m_value);

            return v;
        }

        QString string() const;
};

template<TagValue::Type T>
struct TagValueTraits {};

template<>
struct TagValueTraits<TagValue::Type::Date>
{
    typedef QDate StorageType;
};

template<>
struct TagValueTraits<TagValue::Type::List>
{
    typedef std::deque<TagValue> StorageType;
};

template<>
struct TagValueTraits<TagValue::Type::String>
{
    typedef QString StorageType;
};

template<>
struct TagValueTraits<TagValue::Type::Time>
{
    typedef QTime StorageType;
};


namespace Tag
{
    typedef std::map<TagNameInfo, TagValue> TagsList;

    struct CORE_EXPORT Info
    {
            Info(const TagsList::const_iterator &);
            Info(const std::pair<const TagNameInfo, TagValue> &data);
            Info(const TagNameInfo &, const QVariant &);

            Info& operator=(const std::pair<TagNameInfo, TagValue> &data);

            const QString& name() const;
            const QString& displayName() const;

            const TagNameInfo& getTypeInfo() const;
            const TagValue& value() const;

            void setValue(const QVariant &);

        private:
            TagNameInfo m_name;
            TagValue m_value;
    };

}

Q_DECLARE_METATYPE(TagNameInfo)

#endif
