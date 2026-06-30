#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C"
{
#endif

  uint32_t Core_EnterCritical(void);

  void Core_ExitCritical(uint32_t primask);

  void Core_EnableDebug(void);

#ifdef __cplusplus
}
#endif
