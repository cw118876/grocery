#include "count_new.h"



MemCounter &globalMemCounter = *getGlobalMemCounter();

#ifdef DISABLE_NEW_COUNT
  const bool MemCounter::disable_checking = true;
#else
  const bool MemCounter::disable_checking = false;
#endif