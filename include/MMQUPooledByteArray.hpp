
#ifndef MMQU_POOLEDBYTEARRAY_HPP_INCLUDED
#define MMQU_POOLEDBYTEARRAY_HPP_INCLUDED

#include <QMetaType>
#include <QByteArray>

#include <memory>

using MMQUPooledByteArray = std::shared_ptr<QByteArray>;

Q_DECLARE_METATYPE(MMQUPooledByteArray)

#endif // !MMQU_POOLEDBYTEARRAY_HPP_INCLUDED
