
#ifndef MMQU_POOLEDIMAGE_HPP_INCLUDED
#define MMQU_POOLEDIMAGE_HPP_INCLUDED

#include <QMetaType>
#include <QImage>

#include <memory>

using MMQUPooledImage = std::shared_ptr<QImage>;

Q_DECLARE_METATYPE(MMQUPooledImage)

#endif // !MMQU_POOLEDIMAGE_HPP_INCLUDED
