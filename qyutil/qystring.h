#ifndef __QY_STRING_H__
#define __QY_STRING_H__

class QyStringA;
class QyStringW;

#if defined(_UNICODE)
typedef  QyStringW QyString;
#else
typedef  QyStringA QyString;
#endif //_UNICODE

#include "qyutil/qymallocbase.h"

// declare QyStringW, et. al.
#include "qyutil/qystring-template-def-unichar.h"
#include "qyutil/qystringT.h"
#include "qyutil/qystring-template-undef.h"

  // declare QyStringA, et. al.
#include "qyutil/qystring-template-def-char.h"
#include "qyutil/qystringT.h"
#include "qyutil/qystring-template-undef.h"

#endif /* __qySTRING_H__ */
