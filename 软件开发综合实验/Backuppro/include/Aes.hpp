#pragma once

#include "File.hpp"
#include <openssl/aes.h>
#include <openssl/md5.h>

#define FILE_SUFFIX_ENCRYPT ".ept"

class Aes
{
private:
    unsigned char key_[MD5_DIGEST_LENGTH];
    std::filesystem::path file_path_;

public:
    Aes(std::string file_path, std::string password);
    ~Aes();

    bool encrypt();
    int decrypt(); // 0：成功 -1：密码错误 -2：文件错误
};