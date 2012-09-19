#include "file_cache.h"
#include <map>
#include <pthread.h>

struct CacheData
{
	bool isPinned;
	bool isDirty;
	int lastWritten;
	bool isMarkedForDelete;
};


