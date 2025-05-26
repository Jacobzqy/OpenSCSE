#pragma once

#include "File.hpp"
#include <string>
#include <regex>

#define FILTER_FILE_PATH 1
#define FILTER_FILE_NAME 2
#define FILTER_FILE_ATIME 4
#define FILTER_FILE_MTIME 8
#define FILTER_FILE_CTIME 16

class Filter
{
private:
    unsigned char type_;
    std::string reg_path_;
    std::string reg_name_;

    time_t atime_start_, atime_end_;
    time_t mtime_start_, mtime_end_;
    time_t ctime_start_, ctime_end_;

public:
    Filter();
    ~Filter();

    void set_path_filter(std::string reg_path);
    void set_name_filter(std::string reg_name);
    void set_access_time(time_t atime_start, time_t atime_end);
    void set_modify_time(time_t mtime_start, time_t mtime_end);
    void set_change_time(time_t ctime_start, time_t ctime_end);

    bool check(const FileHeader &file_header);
};