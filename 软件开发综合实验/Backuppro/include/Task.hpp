#pragma once

#include "Packer.hpp"
#include "Aes.hpp"
#include "Compare.hpp"

class Task
{
private:
    std::filesystem::path src_path_;
    std::filesystem::path dest_path_;

    BackupInfo info_;
    Filter filter_;

    bool restore_metadata_;

public:
    Task(std::string src_path, std::string dest_path);
    ~Task();

    bool backup(std::string password);
    void set_mode(unsigned char mode);
    void set_filter(const Filter filter);

    bool restore(std::string password);
    void restore_metadata(bool restore_metadata);
    bool get_backup_info();
    unsigned char get_backup_mode();

    static bool get_backup_info(std::string file_path, BackupInfo &info);

    bool compare();
};