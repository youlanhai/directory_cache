#pragma once

#include <cstdio>
#include <string>

typedef unsigned __int16 uint16;

//directory for apk.
//low memory cost, and run fast.
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

    ApkDirNode * find(const char * name) const;

    void print(const std::string & parent) const;
    size_t bytes() const;

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

    bool load(const std::string & filename);

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
    std::string     buffer_;
    ApkDirNode *    pRoot_;
};