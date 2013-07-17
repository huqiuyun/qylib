// Common.cpp : ∂®“Â DLL ”¶”√≥Ã–Úµƒ»Îø⁄µ„°£
//

#include "stable.h"
#include "qyconfig.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

#ifdef COMMON_DLL
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					)
{
    return TRUE;
}
#endif

#ifdef _MANAGED
#pragma managed(pop)
#endif


DEFINE_NAMESPACE(qy)

BEGIN_EXTERN_C

/** ≥ı ºªØ */
QYUTIL_API void qyutil_initialize(void)
{

}

/** ∑¥≥ı º */
QYUTIL_API void qyutil_unInitialize(void){}

END_EXTERN_C

END_NAMESPACE(qy)
