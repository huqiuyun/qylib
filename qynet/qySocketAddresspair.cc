#include "qySocketAddresspair.h"

namespace qy {

QySocketAddressPair::QySocketAddressPair(
    const QySocketAddress& src, const QySocketAddress& dest)
    : mSource(src), mDest(dest) {
}


bool QySocketAddressPair::operator ==(const QySocketAddressPair& p) const {
  return (mSource == p.mSource) && (mDest == p.mDest);
}

bool QySocketAddressPair::operator <(const QySocketAddressPair& p) const {
  if (mSource < p.mSource)
    return true;
  if (p.mSource < mSource)
    return false;
  if (mDest < p.mDest)
    return true;
  if (p.mDest < mDest)
    return false;
  return false;
}

size_t QySocketAddressPair::hash() const {
  return mSource.hash() ^ mDest.hash();
}

} // namespace qy
