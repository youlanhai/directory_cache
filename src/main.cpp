#include "dir_cache.h"

#include <iostream>

int main()
{
    const char * filename = "ora.dir";

    ApkDirectoryCache cache;
    if (!cache.load(filename))
    {
        std::cout << "error: can't open file " << filename << std::endl;
        return 0;
    }

    std::cout << "total bytes: " << cache.bytes() / 1024  << "kb" << std::endl;

    //cache.getRoot()->print("");

    const char * testfile = "shaders/formats/instanced_skinned_data.xml";

    std::cout << "test file: " << testfile << std::endl;
    std::cout << "is exist: " << cache.exist(testfile) << std::endl;
    std::cout << "is  file: " << cache.isfile(testfile) << std::endl;
    std::cout << "is   dir: " << cache.isdir(testfile) << std::endl;

    return 0;
}
