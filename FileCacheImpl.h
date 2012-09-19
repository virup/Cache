#include "file_cache.h"
#include <map>
#include <queue>
#include <pthread.h>
#include <time.h>

class CacheData
{
public:
	CacheData()
	{
		isPinned = 1;
		isDirty = false;
		lastWritten = clock();
	        isDeleted = false;
		totalWriters= 0; // can be 0 or 1. 1==> thread writing the cache
                


	}

	int isPinned;
	bool isDirty;
	clock_t lastWritten;
	bool isDeleted;	
	int totalWriters;
	pthread_cond_t readWriteCond;
	pthread_mutex_t readWriteMutex;
	pthread_mutex_t writeMutex;
};

class FileCacheImpl :public FileCache
{
public:
	FileCacheImpl(int max_cache_entries, int dirty_time_secs);
	virtual ~FileCacheImpl() {stopWriteBackThread = 1; pthread_join(writebackThread, NULL); }
	void PinFiles(const std::vector<std::string>& file_vec);
	void UnpinFiles(const std::vector<std::string>& file_vec);
	const char *FileData(const std::string& file_name);
	char *MutableFileData(const std::string& file_name);
	void DeleteFile(const std::string& file_name);
	friend void* checkFileforWriteBack(void *);
	void PrintPinned();



private:
	int totalEntries;
	int stopWriteBackThread;
	pthread_t  writebackThread;

	bool isInCache(std::string filename); // returns true if the file is presented in cache
	void putInCache(std::string filename); //read file from disk and update cache


	pthread_cond_t totalEntriesCond;
	pthread_mutex_t pinMutex;

	
	void deleteFileOnDisk(const std::string& file_name);
	char* readDiskFile(std::string filename);
	void writeBack(std::string filename);
	
	clock_t TimeDiff(clock_t, clock_t);
	void startWBThread();
	
	std::queue<std::string > pinQueue; // queue to put a list of files pending to be pinned
	std::map<const std::string , char*> cache; // store pointer to actual files in cache
	std::map<const std::string , CacheData *> cacheDetails; // store details of the cached file entry
};


void *checkFileforWriteBack(void *);
