#pragma once

#include <cstdio>
#include <string>

typedef unsigned __int16 uint16;

//android asset manager, used to read asset in apk.
class AAssetManager;


// directory hierachy for apk.
// low memory cost, and run fast.
class ApkDirNode
{
public:
    ApkDirNode();
    ~ApkDirNode();

    char * load(char * begin, char * end);

    bool isfile() const { return nChildren_ == 0xffff; }
    bool isdir() const { return nChildren_ < 0xffff; }
    const char * getName() const { return pName_; }

    ApkDirNode * getChildren() const { return pChildren_; }
    size_t nChildren() const { return nChildren_; }

    //use binary search, to search the path in directory hierachy.
    ApkDirNode * find(const char * path) const;

    //return the bytes the who hierachy used.
    size_t bytes() const;

    //print the who directory directory to stdout.
    void print(const std::string & parent) const;

private:
    char *          pName_;
    ApkDirNode *    pChildren_;
    size_t          nChildren_;
};


class ApkDirectoryCache
{
public:
    ApkDirectoryCache();
    ~ApkDirectoryCache();

    //load the directory hierachy from file.
    bool load(const std::string & path);

    //load the directory hierachy from file. used for android.
    bool load(AAssetManager *mgr, const std::string & path);

    ApkDirNode * find(const char * path) const;

    bool exist(const char * path) const
    {
        return this->find(path) != NULL;
    }

    bool isdir(const char * path) const
    {
        ApkDirNode * p = this->find(path);
        if (p) return p->isdir();
        return false;
    }

    bool isfile(const char * path) const
    {
        ApkDirNode * p = this->find(path);
        if (p) return p->isfile();
        return false;
    }

    ApkDirNode * getRoot() const { return pRoot_; }
    size_t bytes() const;

private:
    bool init();
    void release();

    // cache the file content.
    char *          pBuffer_;
    size_t          bufferLength_;
    // root for directory hierachy
    ApkDirNode *    pRoot_;
};