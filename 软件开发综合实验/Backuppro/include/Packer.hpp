#pragma once

#include "Filter.hpp"
#include <unordered_map>

#define FILE_SUFFIX_PACK ".pak"

class Packer
{
private:
    std::filesystem::path src_path_;
    std::filesystem::path dest_path_;
    std::unordered_map<ino_t, std::string> inode_table_;

    Filter filter_;
    void dfs_file(File &bak_file, std::filesystem::path curpath);

public:
    Packer(std::string src_path, std::string dest_path, const Filter filter);
    ~Packer();

    bool pack();
    bool unpack(bool restore_metadata);

private:
    void calculate_MD5(std::string file_path, unsigned char *md5);
};