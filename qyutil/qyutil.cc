#include "stable.h"
#include "qyconfig.h"
#include <time.h>
#ifdef _MANAGED
#pragma managed(push, off)
#endif

#ifdef QYUTIL_DLL
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

QYUTIL_API void qyutil_initialize(void)
{
}

QYUTIL_API void qyutil_unInitialize(void)
{

}

END_EXTERN_C

END_NAMESPACE(qy)
