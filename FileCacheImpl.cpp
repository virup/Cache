#include "FileCacheImpl.h"
#include <fstream>
#include <pthread.h>
#include <cstring>
#include <time.h>
#include <iostream>

using namespace std;

FileCacheImpl::FileCacheImpl(int max_cache_entries, int dirty_time_secs) :
    FileCache(max_cache_entries,dirty_time_secs)
{
    cout<<"Creating object";
    totalEntries = 0;
    //pinMutex = PTHREAD_MUTEX_INITIALIZER;

    stopWriteBackThread = 0;
    startWBThread();  // start the writeback thread.

}


void FileCacheImpl::startWBThread()
{
    int res = pthread_create( &writebackThread, NULL, checkFileforWriteBack, this);
}



void FileCacheImpl::PinFiles(const std::vector<std::string>& file)
{
    cout<<"In PinFile"<<endl;
    std::vector<std::string> file_vec = file;
    // remove from file_vec filenames which are already in the cache
    for(std::vector<std::string>::iterator it = file_vec.begin(); it != file_vec.end(); ++it) 
    {
        if(isInCache(*it))
        {
            cout<<"File already present"<<endl;
            cacheDetails[*it]->isPinned +=1; 
            file_vec.erase(it);
        }
    }
    cout<<file_vec.size()<<endl;
    for( int i = 0; i < file_vec.size(); i++)
    {
        cout<<" i = " << i <<endl;
        if(totalEntries == max_cache_entries_)
        {
            cout<<"YES"<<endl;
            map<const std::string , CacheData *>::const_iterator it;
            for(it = cacheDetails.begin(); it != cacheDetails.end(); it++)
            {
                if(it->second->isPinned == 0)
                {
                    cout<<"Is Not Pinned"<<endl;
                    cache.erase(it->first);
                    cacheDetails.erase(it->first);


                    pthread_mutex_lock(&pinMutex);
                    putInCache(file_vec[i]);
                    pthread_mutex_unlock(&pinMutex);

                    break;
                }					
            }			


            // block here using totalEntries == max_cache_entries_
            if(totalEntries == max_cache_entries_)
            {
                cout<<"block here using totalEntries == max_cache_entries_"<<endl;
                pthread_cond_wait(&totalEntriesCond, &pinMutex);
            }

            pthread_mutex_lock(&pinMutex);
            putInCache(file_vec[i]);
            pthread_mutex_unlock(&pinMutex);

        }
        else
        {
            pthread_mutex_lock(&pinMutex);
            putInCache(file_vec[i]);
            pthread_mutex_unlock(&pinMutex);
        }
    }
}


void FileCacheImpl::UnpinFiles(const std::vector<std::string>& file_vec)
{
    for(int i = 0; i < file_vec.size(); i++)
    {
        cacheDetails[file_vec[i]]->isPinned--;
        if(cacheDetails[file_vec[i]]->isPinned==0)
        {
            if(cacheDetails[file_vec[i]]->isDeleted)
                DeleteFile(file_vec[i]);
            else if(cacheDetails[file_vec[i]]->isDirty)
                writeBack(file_vec[i]);
            pthread_cond_signal(&totalEntriesCond);
        }
    }
}


const char *FileCacheImpl::FileData(const std::string& file_name)
{
    string file = file_name;
    if(isInCache(file_name))
    {
        if(cacheDetails[file_name]->totalWriters != 0)
            pthread_cond_wait(&cacheDetails[file_name]->readWriteCond, &cacheDetails[file_name]->writeMutex);
        return cache[file];

    }
    return NULL;
}


char *FileCacheImpl::MutableFileData(const std::string& file_name)
{
    string file = file_name;
    if(isInCache(file_name))
    {
        char *newPtr = new char;
        if(cacheDetails[file_name]->totalWriters == 0)
        {
            pthread_mutex_lock(&cacheDetails[file_name]->writeMutex);
            cacheDetails[file_name]->totalWriters++;
            pthread_mutex_unlock(&cacheDetails[file_name]->writeMutex);
            pthread_cond_wait(&cacheDetails[file_name]->readWriteCond, &cacheDetails[file_name]->writeMutex);

            cacheDetails[file]->isDirty = true;
            newPtr = cache[file_name];	

            pthread_mutex_lock(&cacheDetails[file_name]->writeMutex);
            cacheDetails[file]->totalWriters--;
            pthread_mutex_unlock(&cacheDetails[file_name]->writeMutex);
            pthread_cond_signal(&cacheDetails[file_name]->readWriteCond);
            return newPtr;
        }
    }
    return NULL;
}

void FileCacheImpl::PrintPinned()
{
    for(std::map<const std::string , CacheData *>::iterator iter = cacheDetails.begin(); iter != cacheDetails.end(); ++iter)
    {

        cout<<iter->second->isPinned<<endl;
    }
}

void FileCacheImpl::writeBack(std::string filename)
{
    //perform writeback to disk
    cout<<"\n Performing writeback\n\n";
}
void FileCacheImpl::deleteFileOnDisk(const std::string& file_name)
{
    string file = file_name;
    cache.erase(file);
    cacheDetails.erase(file);
    //TODO: delete from file system
}


void FileCacheImpl::DeleteFile(const std::string& file_name)
{
    if(isInCache(file_name))
    {
        if(cacheDetails[file_name]->isPinned!=0)
            cacheDetails[file_name]->isDeleted = true;
        else
            deleteFileOnDisk(file_name);
    }
}


void* checkFileforWriteBack(void *i)
{
    FileCacheImpl *f = (FileCacheImpl *)i;
    while(1)
    {
        sleep(f->dirty_time_secs_);
        if(f->stopWriteBackThread == 1)
            break;
        map<const std::string , CacheData *>::const_iterator it;
        for(it = f->cacheDetails.begin(); it != f->cacheDetails.end(); it++)
        {
            if(it->second->isDirty == 0 && f->TimeDiff(clock(), it->second->lastWritten) > f->dirty_time_secs_ * CLOCKS_PER_SEC)
            {
                f->writeBack(it->first);
                it->second->lastWritten = clock();
            }					
        }
    }

}

clock_t FileCacheImpl::TimeDiff(clock_t now, clock_t then)
{	
    // STUB FUNCTION
    return now - then;
}

void FileCacheImpl::putInCache(string filename)
{
    cout<<"In putInCache"<<endl;
    //Read the file and update cache
    char *fileData = readDiskFile(filename);
    CacheData *cacheData = new CacheData();
    cache.insert (pair<string,char*>(filename,fileData));
    cacheDetails.insert(pair<string, CacheData*> (filename,cacheData));
    totalEntries++;

}
bool FileCacheImpl::isInCache(string filename)
{

    if(cache.find(filename) == cache.end())
        return false;
    return true;
}


char* FileCacheImpl::readDiskFile(string filename)
{
    fstream myReadFile;
    myReadFile.open(filename.c_str());
    char* output = new char[10*1024 + 1];
    string *fileData = new string();

    if (myReadFile.is_open()) 
    {
        while (!myReadFile.eof()) 
        {
            myReadFile >> output;
            fileData->append(output);
        }
    }
    strcpy(output,fileData->c_str());
    myReadFile.close();
    return output;
}
