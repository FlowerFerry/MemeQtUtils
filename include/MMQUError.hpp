
#ifndef MMQU_ERROR_HPP_INCLUDED
#define MMQU_ERROR_HPP_INCLUDED

#include <mego/err/ec.h>

#include <QString>
#include <QVariant>
#include <QMap>
#include <QSharedData>
#include <QSharedDataPointer>
#include <QSharedPointer>
#include <exception>
#include <memory>

class MMQUError; // forward declaration

namespace MMQU {

namespace Details {

    struct FunctionInfo {
        QString name_;
        QMap<int, QString> args_;
    };

    class ErrorPayload : public QSharedData {
    public:
        ErrorPayload() = default;

        ErrorPayload(const ErrorPayload& _other)
            : QSharedData(_other)
            , message_(_other.message_)
            , solution_(_other.solution_)
            , functionInfo_(_other.functionInfo_ ? std::make_unique<FunctionInfo>(*_other.functionInfo_) : nullptr)
            , next_(_other.next_)
            , userData_(_other.userData_)
        {}

        QString message_;
        QString solution_;
        std::unique_ptr<FunctionInfo> functionInfo_;
        QSharedPointer<MMQUError> next_;
        QVariant userData_;
    };

}; // namespace Details  

}; // namespace MMQU

class MMQUErrorCondition;

//! \brief Error category
class MMQUErrorCategory
{
public:
    virtual ~MMQUErrorCategory() = default;

    virtual const char* name() const noexcept = 0;
    virtual QString message(int _errVal) const noexcept = 0;
    
    virtual MMQUErrorCondition errorCondition(int _errVal) const noexcept;

    virtual bool equivalent(int _errVal, const MMQUErrorCondition& _cond) const noexcept;

    virtual bool equivalent(const MMQUError& _code, int _errVal) const noexcept;

    inline bool operator==(const MMQUErrorCategory& _rhs) const noexcept { return this == &_rhs; }
    inline bool operator!=(const MMQUErrorCategory& _rhs) const noexcept { return this != &_rhs; }
    inline bool operator< (const MMQUErrorCategory& _rhs) const noexcept { return this <  &_rhs; }
    inline bool operator> (const MMQUErrorCategory& _rhs) const noexcept { return this >  &_rhs; }
};

class MMQUGenericErrorCategory : public MMQUErrorCategory
{
public:
    const char* name() const noexcept override
    {
        return "generic";
    }

    QString message(int _errVal) const noexcept override
    {
        Q_UNUSED(_errVal);
        return QString();
    }
};

inline const MMQUErrorCategory* genericErrorCategory() noexcept
{
    static MMQUGenericErrorCategory cat;
    return &cat;
}

class MMQUErrorCondition
{
public:
    MMQUErrorCondition() noexcept
        : errVal_(0)
        , category_(genericErrorCategory())
    {}

    MMQUErrorCondition(int _errVal, const MMQUErrorCategory* _category) noexcept
        : errVal_(_errVal)
        , category_(_category)
    {}

    int value() const noexcept { return errVal_; }
    QString message() const noexcept { return category_->message(errVal_); }
    const MMQUErrorCategory* category() const noexcept { return category_; }

    bool operator==(const MMQUErrorCondition& _rhs) const noexcept
    {
        return errVal_ == _rhs.errVal_ && category_ == _rhs.category_;
    }

    bool operator!=(const MMQUErrorCondition& _rhs) const noexcept
    {
        return !(*this == _rhs);
    }

private:
    int errVal_;
    const MMQUErrorCategory* category_;
};

inline MMQUErrorCondition makeErrorCondition(int _errVal, const MMQUErrorCategory* _category) noexcept
{
    return MMQUErrorCondition{ _errVal, _category };
}

class MMQUError 
{
    Q_GADGET
    Q_PROPERTY(int code READ code)
    Q_PROPERTY(int userCode READ userCode)
    Q_PROPERTY(QString message READ message)
    Q_PROPERTY(QString solution READ solution)
    Q_PROPERTY(QVariant userData READ userData)
    Q_PROPERTY(bool hasFunctionInfo READ hasFunctionInfo)
    Q_PROPERTY(bool hasNext READ hasNext)
public:
    using FunctionInfo = MMQU::Details::FunctionInfo;

    MMQUError()
        : code_(MGEC__OK), userCode_(0), globalCategory_(genericErrorCategory())
    {}

    MMQUError(int32_t _code)
        : code_(_code), userCode_(0), globalCategory_(genericErrorCategory())
    {}

    MMQUError(int32_t _code, const QString& _message)
        : code_(_code), userCode_(0), globalCategory_(genericErrorCategory())
    {
        ensurePayload()->message_ = _message;
    }

    MMQUError(int32_t _code, const QString& _message, const QString& _solution)
        : code_(_code), userCode_(0), globalCategory_(genericErrorCategory())
    {
        ensurePayload()->message_ = _message;
        ensurePayload()->solution_ = _solution;
    }
    
    MMQUError(int32_t _code, int32_t _userCode)
        : code_(_code), userCode_(_userCode), globalCategory_(genericErrorCategory())
    {}

    MMQUError(int32_t _code, int32_t _userCode, const QString& _message)
        : code_(_code), userCode_(_userCode), globalCategory_(genericErrorCategory())
    {
        ensurePayload()->message_ = _message;
    }

    MMQUError(int32_t _code, int32_t _userCode, const QString& _message, const QString& _solution)
        : code_(_code), userCode_(_userCode), globalCategory_(genericErrorCategory())
    {
        ensurePayload()->message_ = _message;
        ensurePayload()->solution_ = _solution;
    }

    MMQUError(int32_t _code, MMQUErrorCategory* _category)
        : code_(_code), userCode_(0), globalCategory_(_category)
    {}

    MMQUError(int32_t _code, MMQUErrorCategory* _category, const QString& _message)
        : code_(_code), userCode_(0), globalCategory_(_category)
    {
        ensurePayload()->message_ = _message;
    }

    MMQUError(int32_t _code, MMQUErrorCategory* _category, const QString& _message, const QString& _solution)
        : code_(_code), userCode_(0), globalCategory_(_category)
    {
        ensurePayload()->message_ = _message;
        ensurePayload()->solution_ = _solution;
    }

    MMQUError(const QVariant& _data)
        : code_(MGEC__OK), userCode_(0), globalCategory_(genericErrorCategory())
    {
        ensurePayload()->userData_ = _data;
    }

    MMQUError(int32_t _code, const QVariant& _data)
        : code_(_code), userCode_(0), globalCategory_(genericErrorCategory())
    {
        ensurePayload()->userData_ = _data;
    }

    MMQUError(const MMQUError& _other) = default;
    MMQUError(MMQUError&& _other) noexcept = default;
    MMQUError& operator=(const MMQUError& _other) = default;
    MMQUError& operator=(MMQUError&& _other) noexcept = default;

    bool operator==(const MMQUError& _other) const noexcept
    {
        return category() == _other.category() && code() == _other.code() && userCode() == _other.userCode();
    }
    
    bool operator!=(const MMQUError& _other) const noexcept
    {
        return !(*this == _other);
    }

    constexpr mgec_t  code() const noexcept { return code_; }
    constexpr int32_t userCode() const noexcept { return userCode_; }

    QString message() const noexcept
    {
        return payload_ ? payload_->message_ : QString();
    }

    QString solution() const noexcept
    {
        return payload_ ? payload_->solution_ : QString();
    }

    bool hasFunctionInfo() const noexcept
    {
        return payload_ && payload_->functionInfo_ != nullptr;
    }

    const FunctionInfo* functionInfo() const noexcept
    {
        return payload_ ? payload_->functionInfo_.get() : nullptr;
    }

    QVariant userData() const noexcept
    {
        return payload_ ? payload_->userData_ : QVariant();
    }

    MMQUError copy() const { return MMQUError{ *this }; }

    bool hasNext() const noexcept
    {
        return payload_ && !payload_->next_.isNull();
    }

    MMQUError next() const
    {
        if (!payload_ || payload_->next_.isNull())
            return makeOk();
        return *payload_->next_;
    }

    void setNext(const MMQUError& _nextError)
    {
        ensurePayload()->next_ = QSharedPointer<MMQUError>::create(_nextError);
    }

    bool isOk() const noexcept { return code() == MGEC__OK; }
    explicit operator bool() const noexcept { return !isOk(); }

    void setMessage(const QString& _message)
    {
        ensurePayload()->message_ = _message;
    }

    void setSolution(const QString& _solution)
    {
        ensurePayload()->solution_ = _solution;
    }

    void setFunctionInfo(const FunctionInfo& _info)
    {
        ensurePayload()->functionInfo_ = std::make_unique<FunctionInfo>(_info);
    }

    void setUserData(const QVariant& _data)
    {
        ensurePayload()->userData_ = _data;
    }

    MMQUErrorCondition errorCondition() const noexcept
    {
        return MMQUErrorCondition{ code_, globalCategory_ };
    }

    inline const MMQUErrorCategory* category() const noexcept { return globalCategory_; }

    inline static MMQUError makeOk()
    {
        static MMQUError e;
        return e;
    }

    inline static MMQUError makeUnknown()
    {
        static MMQUError e { MGEC__ERR, QStringLiteral("unknown") };
        return e;
    }

private:
    MMQU::Details::ErrorPayload* ensurePayload() {
        if (!payload_) {
            payload_ = new MMQU::Details::ErrorPayload();
        }
        return payload_.data();
    }

    int32_t code_;
    int32_t userCode_;
    const MMQUErrorCategory* globalCategory_;
    QSharedDataPointer<MMQU::Details::ErrorPayload> payload_;
};

Q_DECLARE_METATYPE(MMQUError)

inline MMQUErrorCondition MMQUErrorCategory::errorCondition(int _errVal) const noexcept
{
    return MMQUErrorCondition{ _errVal, this };
}

inline bool MMQUErrorCategory::equivalent(int _errVal, const MMQUErrorCondition& _cond) const noexcept
{
    return errorCondition(_errVal) == _cond;
}

inline bool MMQUErrorCategory::equivalent(const MMQUError& _code, int _errVal) const noexcept
{
    return _code.code() == _errVal && _code.category() == this;
}

inline bool operator==(const MMQUError& _lhs, const MMQUErrorCondition& _rhs) noexcept
{
    return _lhs.category() == _rhs.category() && _lhs.code() == _rhs.value();
}

inline bool operator==(const MMQUErrorCondition& _lhs, const MMQUError& _rhs) noexcept
{
    return _rhs == _lhs;
}

inline bool operator!=(const MMQUError& _lhs, const MMQUErrorCondition& _rhs) noexcept
{
    return !(_lhs == _rhs);
}

inline bool operator!=(const MMQUErrorCondition& _lhs, const MMQUError& _rhs) noexcept
{
    return !(_lhs == _rhs);
}

#endif // !MMQU_ERROR_HPP_INCLUDED
