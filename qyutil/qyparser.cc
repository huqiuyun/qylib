#include "qyparser.h"
#include <assert.h>
#include <vector>
#include <string>

#pragma warning(disable:4267)

// declare CParserW_lo, et. al.
#include "qyparser-template-def-unichar.h"
#include "qyparserT.cc"
#include "qyparser-template-undef.h"

  // declare CParserA_lo, et. al.
#include "qyparser-template-def-char.h"
#include "qyparserT.cc"
#include "qyparser-template-undef.h"
