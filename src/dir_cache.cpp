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

//////////////////////////////////////////////////////////////////////////
/// class ApkDirNode
//////////////////////////////////////////////////////////////////////////

ApkDirNode::ApkDirNode()
    : pName_(NULL)
    , pChildren_(NULL)
    , nChildren_(0)
{}

ApkDirNode::~ApkDirNode()
{
    //we don't need to delete pName_ here.

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

    //binary search
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

//////////////////////////////////////////////////////////////////////////
/// class ApkDirectoryCache
//////////////////////////////////////////////////////////////////////////

ApkDirectoryCache::ApkDirectoryCache()
    : pBuffer_(NULL)
    , bufferLength_(0)
    , pRoot_(NULL)
{}

ApkDirectoryCache::~ApkDirectoryCache()
{
    release();
}

bool ApkDirectoryCache::load(const std::string & filename)
{
    release();

    FILE *pFile = fopen(filename.c_str(), "rb");
    if (NULL == pFile) return false;

    fseek(pFile, 0, SEEK_END);
    bufferLength_ = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    if (bufferLength_ > 0)
    {
        pBuffer_ = new char[bufferLength_];
        if (bufferLength_ != fread(pBuffer_, 1, bufferLength_, pFile))
        {
            fclose(pFile);
            return false;
        }
    }
    fclose(pFile);

    return init();
}


bool ApkDirectoryCache::init()
{
    pRoot_ = new ApkDirNode();
    return NULL != pRoot_->load(pBuffer_, pBuffer_ + bufferLength_);
}

void ApkDirectoryCache::release()
{
    if (pRoot_)
    {
        delete pRoot_;
        pRoot_ = NULL;
    }

    if (pBuffer_)
    {
        delete [] pBuffer_;
        pBuffer_ = NULL;
    }
}

ApkDirNode * ApkDirectoryCache::find(const char * path) const
{
    if (pRoot_) return pRoot_->find(path);
    return NULL;
}

size_t ApkDirectoryCache::bytes() const
{
    size_t size = sizeof(*this) + bufferLength_;
    if (pRoot_) size += pRoot_->bytes();
    return size;
}


#ifdef ANDROID

#include <android/asset_manager_jni.h>

bool ApkDirectoryCache::load(AAssetManager *mgr, const std::string & path)
{
    release();

    AAsset* pAsset = AAssetManager_open(mgr, path.c_str(), AASSET_MODE_BUFFER);
    if (!pAsset) return false;

    bufferLength_ = AAsset_getLength(pAsset);
    if (bufferLength_ <= 0)
    {
        AAsset_close(pAsset);
        return false;
    }

    const void * pBuffer = AAsset_getBuffer(pAsset);

    pBuffer_ = new char[bufferLength_];
    memcpy(pBuffer_, pBuffer, bufferLength_)

    AAsset_close(pAsset);

    return init();
}

#endif //ANDROID
