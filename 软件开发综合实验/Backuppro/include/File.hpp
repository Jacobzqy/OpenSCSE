#pragma once

#include <sys/stat.h>
#include <unistd.h>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstring>
#include <fcntl.h>
#include <openssl/md5.h>

#define FILE_TYPE_NORMAL 1
#define FILE_TYPE_DIRECTORY 2
#define FILE_TYPE_HARD_LINK 4
#define FILE_TYPE_SYMBOLIC_LINK 8
#define FILE_TYPE_FIFO 16
#define FILE_TYPE_OTHERS 32

#define MAX_FILE_PATH 256      // 文件最长路径长度
#define BLOCK_BUFFER_SIZE 4096 // 读写缓冲区大小

#define BACKUP_MODE_ENCRYPT 1

typedef unsigned char FileType;

struct BackupInfo
{
    time_t timestamp_;
    char backup_path_[MAX_FILE_PATH];
    unsigned char mode_;
};

struct FileHeader
{
    char name_[MAX_FILE_PATH];             // 文件名
    char linkname_[MAX_FILE_PATH];         // 链接文件名
    unsigned char md5_[MD5_DIGEST_LENGTH]; // 文件的md5哈希值
    struct stat metadata_;                 // 文件元数据
};

class File : public std::fstream
{
private:
    FileHeader file_header_;
    std::filesystem::path file_path_;

public:
    File(FileHeader file_header);
    File(std::filesystem::path file_path);
    ~File();

    bool open_file(std::ios_base::openmode mode);

    FileHeader read_file_header();
    BackupInfo read_backup_info();
    void write_backup_info();
    void write_backup_info(BackupInfo info);

    size_t get_file_size();
    FileType get_file_type();
    static FileType get_file_type(const FileHeader file_header);
    FileHeader get_file_header();
    bool is_hardlink();

    void restore_metadata();

    void calculate_MD5(std::filesystem::path file_path, unsigned char *md5);
};