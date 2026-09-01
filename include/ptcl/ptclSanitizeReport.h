#pragma once

#include "typedefs.h"

#include <QString>
#include <QStringList>

#include <type_traits>
#include <utility>


namespace Ptcl {


// ========================================================================== //


namespace detail {

template<typename T>
concept EnumValue = std::is_enum_v<T>;

template<EnumValue T>
constexpr s64 rawValue(T value) {
    return static_cast<s64>(std::to_underlying(value));
}

} // namespace detail


// ========================================================================== //


template<detail::EnumValue EnumType>
struct SanitizedValue {
    EnumType value{};
    bool wasInvalid{false};
    QString issue{};
};


// ========================================================================== //


template<detail::EnumValue EnumType>
SanitizedValue<EnumType> sanitizeEnum(EnumType raw, EnumType maxValue, EnumType fallback, const QString& fieldName) {
    const s64 value = detail::rawValue(raw);
    const s64 max = detail::rawValue(maxValue);

    if (value >= 0 && value <= max) {
        return {static_cast<EnumType>(value), false, {}};
    }

    SanitizedValue<EnumType> result;
    result.value = fallback;
    result.wasInvalid = true;
    result.issue = QStringLiteral("field '%1' has invalid value %2; clamped to %3.")
       .arg(fieldName)
       .arg(value)
       .arg(detail::rawValue(fallback));
    return result;
}

template<detail::EnumValue EnumType>
SanitizedValue<EnumType> sanitizeEnum(EnumType raw, EnumType maxEnum, QString fieldName) {
    return sanitizeEnum(raw, maxEnum, maxEnum, fieldName);
}


// ========================================================================== //


class PtclSanitizeReport {
public:
    bool hasIssues() const { return !mIssues.isEmpty(); }
    s32 count() const { return static_cast<s32>(mIssues.size()); }
    const QStringList& issues() const { return mIssues; }

    void setContext(QString context) { mContext = std::move(context); }
    const QString& context() const { return mContext; }

    void add(QString issue) { mIssues.push_back(std::move(issue)); }

    template<detail::EnumValue EnumType>
    EnumType sanitize(EnumType raw, EnumType maxValue, QString fieldName) {
        return sanitize(raw, maxValue, maxValue, fieldName);
    }

    template<detail::EnumValue EnumType>
    EnumType sanitize(EnumType raw, EnumType maxValue, EnumType fallback, QString fieldName) {
        const auto result = sanitizeEnum(raw, maxValue, fallback, fieldName);
        if (!result.wasInvalid) {
            return result.value;
        }

        add(mContext.isEmpty()
            ? result.issue
            : mContext + QStringLiteral(": ") + result.issue);

        return result.value;
    }

private:
    QStringList mIssues;
    QString mContext;
};


// ========================================================================== //


} // namespace Ptcl