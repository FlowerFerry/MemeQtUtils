
#ifndef MMQU_BYTEARRAYPOOL_HPP_INCLUDED
#define MMQU_BYTEARRAYPOOL_HPP_INCLUDED

#include "MMQUPooledByteArray.hpp"

#include <QStack>

#include <mutex>

struct MMQUByteArrayPoolCore
{
    QStack<QByteArray*> pool;
    std::mutex mutex;
    bool isAlive = true;

    ~MMQUByteArrayPoolCore()
    {
        while (!pool.empty())
            delete pool.pop();
    }
};

class MMQUByteArrayPool
{
public:
    MMQUByteArrayPool();
    ~MMQUByteArrayPool();

    MMQUPooledByteArray acquire(size_t _reserveSize = 0);

    void clear();

    static MMQUByteArrayPool& instance()
    {
        static MMQUByteArrayPool pool;
        return pool;
    }

private:
    std::shared_ptr<MMQUByteArrayPoolCore> core_;
};

inline MMQUByteArrayPool::MMQUByteArrayPool()
    : core_(std::make_shared<MMQUByteArrayPoolCore>())
{
}

inline MMQUByteArrayPool::~MMQUByteArrayPool()
{
    std::lock_guard locker(core_->mutex);
    core_->isAlive = false;
}

inline MMQUPooledByteArray MMQUByteArrayPool::acquire(size_t _reserveSize)
{
    std::unique_lock locker(core_->mutex);
    QByteArray* byteArray = nullptr;
    if (!core_->pool.empty())
        byteArray = core_->pool.pop();
    else
        byteArray = new QByteArray;
    auto core = core_;
    locker.unlock();

    if (_reserveSize > 0 && byteArray->capacity() < _reserveSize)
        byteArray->reserve(_reserveSize);

    return MMQUPooledByteArray(byteArray, [core](QByteArray* _byteArray) 
    {
        _byteArray->resize(0);

        std::unique_lock locker(core->mutex);
        if (core->isAlive)
            core->pool.push(_byteArray);
        else {
            locker.unlock();
            delete _byteArray;
        }
    });
}

inline void MMQUByteArrayPool::clear()
{
    std::unique_lock locker(core_->mutex);
    auto pool = std::move(core_->pool);
    locker.unlock();
    while (!pool.empty())
        delete pool.pop();
}

#endif // !MMQU_BYTEARRAYPOOL_HPP_INCLUDED
