#include "dir_cache.h"

#include <iostream>
#include <cstring>

static int ora_strxcmp(const char * s1, int l1, const char * s2, int l2)
{
    if (l1 < 0) l1 = strlen(s1);
    if (l2 < 0) l2 = strlen(s2);

    for(int i = 0; i < l1 && i < l2; ++i)
    {
        if (s1[i] < s2[i]) return -1;
        if (s1[i] > s2[i]) return 1;
    }

    return l1 - l2;
}


ApkDirNode::ApkDirNode()
    : pName_(NULL)
    , pChildren_(NULL)
    , nChildren_(0)
{}

ApkDirNode::~ApkDirNode()
{
    if (NULL != pChildren_)
        delete[] pChildren_;
}

char * ApkDirNode::load(char * begin, char * end)
{
    if (begin + 2 > end) return NULL;

    size_t nName = *(uint16*) begin;
    begin += 2;

    if (begin + nName + 2 > end) return NULL;

    pName_ = begin;
    begin += nName;

    nChildren_ = *(uint16*)begin;
    *begin = '\0';
    begin += 2;

    if (nChildren_ < 0xffff && nChildren_ != 0)
    {
        pChildren_ = new ApkDirNode[nChildren_];
        for (size_t i = 0; i < nChildren_; ++i)
        {
            begin = pChildren_[i].load(begin, end);
            if (begin == NULL) return NULL;
        }
    }
    return begin;
}

ApkDirNode * ApkDirNode::find(const char * name) const
{
    if (!isdir()) return NULL;

    const char * end = name;
    while (*end != '/' && *end != 0)
        ++end;
    size_t nameLen = end - name;

    int left = 0;
    int right = int(nChildren_);
    int half;
    int ret = -1;

    while (left < right)
    {
        half = (left + right) >> 1;
        ret = ora_strxcmp(name, nameLen, pChildren_[half].pName_, -1);

        if (ret == 0) break;
        else if (ret < 0) right = half;
        else left = half + 1;
    }

    if (ret != 0) return NULL;

    if (*end == '/')
        return pChildren_[half].find(end + 1);

    return pChildren_ + half;
}

void ApkDirNode::print(const std::string & parent) const
{
    if (!pChildren_)
    {
        std::cout << parent << "/" << pName_ << std::endl;
    }
    else
    {
        for (size_t i = 0; i < nChildren_; ++i)
            pChildren_[i].print(parent + "/" + pName_);
    }
}

size_t ApkDirNode::bytes() const
{
    size_t size = sizeof(*this);
    if (pChildren_)
    {
        for (size_t i = 0; i < nChildren_; ++i)
            size += pChildren_[i].bytes();
    }
    return size;
}

ApkDirectoryCache::ApkDirectoryCache()
    : pRoot_(NULL)
{}

ApkDirectoryCache::~ApkDirectoryCache()
{
    delete pRoot_;
}

bool ApkDirectoryCache::load(const std::string & filename)
{
    if (pRoot_)
    {
        delete pRoot_;
        pRoot_ = NULL;
    }

    FILE *pFile = fopen(filename.c_str(), "rb");
    if (NULL == pFile) return false;

    fseek(pFile, 0, SEEK_END);
    long length = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    buffer_.resize(length);
    if (length > 0)
    {
        if (length != fread(&buffer_[0], 1, length, pFile))
        {
            buffer_.clear();
            fclose(pFile);
            return false;
        }
    }

    fclose(pFile);
    
    pRoot_ = new ApkDirNode();
    char * begin = &buffer_[0];
    return NULL != pRoot_->load(begin, begin + buffer_.length());
}

ApkDirNode * ApkDirectoryCache::find(const char * path) const
{
    if (pRoot_) return pRoot_->find(path);
    return NULL;
}

size_t ApkDirectoryCache::bytes() const
{
    size_t size = sizeof(*this) + buffer_.capacity();
    if (pRoot_) size += pRoot_->bytes();
    return size;
}
