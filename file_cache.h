// The problem is to implement a file cache in C++ that derives the interface
// given below in class Cache. Cache is a thread-safe object that can
// be simultaneously accessed by multiple threads. The typical usage is for
// a thread to call 'PinFiles()' to pin a bunch of files in the cache and then
// either read or write to their in-memory contents in the cache. Writing to
// a cache entry makes that entry 'dirty'. Before a dirty entry can be evicted
// from the cache, it must be unpinned and has to be cleaned by writing the
// corresponding data to disk.


#include <string>
#include <vector>

class Cache {
    public:
        // Constructor. 'max_cache_entries' is the maximum number of files that can
        // be cached at any time.  A disk flush should be scheduled if an unpinned
        // file in the cache is dirty and hasn't been written to in time greater-than
        // 'dirty_time_secs'. Such a flush should be done as a background activity.
        Cache(int max_cache_entries, int dirty_time_secs) :
            max_cache_entries_(max_cache_entries),
            dirty_time_secs_(dirty_time_secs) {
            }

        // Destructor. Implementation should flush all dirty buffers and stop the
        // background thread.
        virtual ~Cache() {}

        // Pins the given files in vector 'file_vec' in the cache. If any of these
        // files is not already cached, they are first read from the disk. If the
        // cache is full, then some existing cache entries may be evicted. If no
        // entries can be evicted (e.g., if they are all pinned, or dirty), then
        // this method will block until a suitable number of cache entries become
        // available. It is fine if more than one thread pins the same file, however
        // the file should not become unpinned until both threads have unpinned the
        // file.
        //
        // Correct usage of PinFiles() (to avoid deadlocks) is assumed to be the
        // caller's responsibility, and therefore doesn't need to be handled. The
        // caller is assumed to pin all the files it wants using one PinFiles() call
        // and not call multiple PinFiles() calls without unpinning those files in
        // the interim.
        //
        // The PinFiles() calls should be starvation free, but doesn't necessarily
        // need to be fair. Thus, it should be guaranteed that all PinFiles() calls
        // would ultimately succeed. However, it is ok to satisfy PinFiles() calls
        // in an order different than they were made.
        virtual void PinFiles(const std::vector<std::string>& file_vec) = 0;

        // Unpin one or more files that were previously pinned. It is ok to unpin
        // only a subset of the files that were previously pinned using PinFiles().
        // It is undefined behavior to unpin a file that wasn't pinned.
        virtual void UnpinFiles(const std::vector<std::string>& file_vec) = 0;

        // Provide read-only access to a pinned file's data in the cache. This call
        // should never block (other than temporarily while contending on a lock).
        //
        // It is undefined behavior if the file is not pinned, or to access the
        // buffer when the file isn't pinned.
        virtual const char *FileData(const std::string& file_name) = 0;

        // Provide write access to a pinned file's data in the cache. This call marks
        // the file's data as 'dirty'. The caller may update the contents of the file
        // by writing to the memory pointed by the returned value. This call should
        // never block (other than temporarily while contending on a lock).
        //
        // It is undefined behavior if the file is not pinned, or to access the
        // buffer when the file is not pinned.
        virtual char *MutableFileData(const std::string& file_name) = 0;

        // Mark a file for deletion from the filesystem. It may or may not be pinned.
        // If it is pinned, then the deletion is delayed until after the file is
        // unpinned. This call should never block (other than temporarily while
        // contending on a lock).
        virtual void DeleteFile(const std::string& file_name) = 0;
        virtual void PrintPinned()=0;
    protected:
        // Maximum number of files that can be cached at any time.
        const int max_cache_entries_;

        // A background disk flush should be scheduled if an unpinned file in the
        // cache is dirty and hasn't been written to in time > dirty_time_secs_.
        const int dirty_time_secs_;

    private:
        // Disallow copies and assignment. Do *not* implement!
        Cache(const Cache&);
        Cache& operator=(const Cache&);
};

#endif  
