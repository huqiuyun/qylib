#ifndef QY_QySocketAddressPAIR_H__
#define QY_QySocketAddressPAIR_H__

#include "qySocketAddress.h"

namespace qy {

// Records a pair (source,destination) of socket addresses.  The two addresses
// identify a connection between two machines.  (For UDP, this "connection" is
// not maintained explicitly in a socket.)
class QySocketAddressPair {
public:
  QySocketAddressPair() {}
  QySocketAddressPair(const QySocketAddress& srs, const QySocketAddress& dest);

  const QySocketAddress& source() const { return mSource; }
  const QySocketAddress& destination() const { return mDest; }

  bool operator ==(const QySocketAddressPair& r) const;
  bool operator <(const QySocketAddressPair& r) const;

  size_t hash() const;

private:
  QySocketAddress mSource;
  QySocketAddress mDest;
};

} // namespace qy

#endif // QY_QySocketAddressPAIR_H__
