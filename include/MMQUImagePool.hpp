
#ifndef MMQU_IMAGEPOOL_HPP_INCLUDED
#define MMQU_IMAGEPOOL_HPP_INCLUDED

#include "MMQUPooledImage.hpp"

#include <QHash>
#include <QStack>

struct MMQUImageKey
{
    int width;
    int height;
    QImage::Format format;

    bool operator==(const MMQUImageKey& other) const
    {
        return width == other.width && height == other.height && format == other.format;
    }
};

inline uint qHash(const MMQUImageKey& key, uint seed = 0)
{
    return qHash(key.width, seed) ^ qHash(key.height, seed) ^ qHash(static_cast<int>(key.format), seed);
}

struct MMQUImagePoolCore
{
    QHash<MMQUImageKey, QStack<QImage*>> pool;
    std::mutex mutex;
    bool isAlive = true;

    ~MMQUImagePoolCore()
    {
        for (auto& stack : pool) {
            while (!stack.empty())
                delete stack.pop();
        }
    }
};

class MMQUImagePool
{
public:
    MMQUImagePool();
    ~MMQUImagePool();

    MMQUPooledImage acquire(int _width, int _height, QImage::Format _format);

    void clear();

    static MMQUImagePool& instance()
    {
        static MMQUImagePool pool;
        return pool;
    }

private:
    std::shared_ptr<MMQUImagePoolCore> core_;
};

MMQUImagePool::MMQUImagePool()
    : core_(std::make_shared<MMQUImagePoolCore>())
{
}

MMQUImagePool::~MMQUImagePool()
{
    std::lock_guard locker(core_->mutex);
    core_->isAlive = false;
}

inline MMQUPooledImage MMQUImagePool::acquire(int _width, int _height, QImage::Format _format)
{
    MMQUImageKey key{ _width, _height, _format };
    std::unique_lock locker(core_->mutex);
    QImage* image = nullptr;
    if (core_->pool.contains(key) && !core_->pool[key].empty())
        image = core_->pool[key].pop();
    else
        image = new QImage(_width, _height, _format);
    auto core = core_;
    locker.unlock();

    return MMQUPooledImage(image, [core](QImage* img) {
        std::lock_guard locker(core->mutex);
        if (core->isAlive) {
            MMQUImageKey key{ img->width(), img->height(), img->format() };
            core->pool[key].push(img);
        }
        else {
            locker.unlock();
            delete img;
        }
    });
}

inline void MMQUImagePool::clear()
{
    std::lock_guard locker(core_->mutex);
    auto pool = std::move(core_->pool);
    locker.unlock();
    for (auto& stack : pool) {
        while (!stack.empty())
            delete stack.pop();
    }
}

#endif // !MMQU_IMAGEPOOL_HPP_INCLUDED
