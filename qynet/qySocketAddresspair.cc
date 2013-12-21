#include "qySocketAddresspair.h"

namespace qy {

QySocketAddressPair::QySocketAddressPair(
    const QySocketAddress& src, const QySocketAddress& dest)
    : src_(src), dest_(dest) {
}


bool QySocketAddressPair::operator ==(const QySocketAddressPair& p) const {
  return (src_ == p.src_) && (dest_ == p.dest_);
}

bool QySocketAddressPair::operator <(const QySocketAddressPair& p) const {
  if (src_ < p.src_)
    return true;
  if (p.src_ < src_)
    return false;
  if (dest_ < p.dest_)
    return true;
  if (p.dest_ < dest_)
    return false;
  return false;
}

size_t QySocketAddressPair::hash() const {
  return src_.hash() ^ dest_.hash();
}

} // namespace qy
