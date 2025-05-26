#include "Filter.hpp"

Filter::Filter()
{
    type_ = 0;
}

Filter::~Filter()
{
}

void Filter::set_path_filter(std::string reg_path)
{
    reg_path_ = reg_path;
    type_ |= FILTER_FILE_PATH;
}

void Filter::set_name_filter(std::string reg_name)
{
    reg_name_ = reg_name;
    type_ |= FILTER_FILE_NAME;
}

void Filter::set_access_time(time_t atime_start, time_t atime_end)
{
    atime_start_ = atime_start;
    atime_end_ = atime_end;
    type_ |= FILTER_FILE_ATIME;
}

void Filter::set_modify_time(time_t mtime_start, time_t mtime_end)
{
    mtime_start_ = mtime_start;
    mtime_end_ = mtime_end;
    type_ |= FILTER_FILE_MTIME;
}

void Filter::set_change_time(time_t ctime_start, time_t ctime_end)
{
    ctime_start_ = ctime_start;
    ctime_end_ = ctime_end;
    type_ |= FILTER_FILE_CTIME;
}

bool Filter::check(const FileHeader &file_header)
{
    FileType cur_file_type = File::get_file_type(file_header);

    if (type_ & FILTER_FILE_PATH)
    {
        std::regex reg(reg_path_);
        std::string dir_path = file_header.name_;
        if (cur_file_type == FILE_TYPE_DIRECTORY)
        {
            std::filesystem::path file_path(file_header.name_);
            dir_path = file_path.parent_path().string();
        }
        if (!std::regex_search(dir_path, reg))
        {
            return false;
        }
    }

    if ((type_ & FILTER_FILE_NAME) && (cur_file_type != FILE_TYPE_DIRECTORY))
    {
        std::regex reg(reg_name_);
        std::filesystem::path file_path = file_header.name_;
        if (!std::regex_search(file_path.filename().string(), reg))
        {
            return false;
        }
    }

    if (type_ & FILTER_FILE_ATIME)
    {
        time_t cur_file_sec = file_header.metadata_.st_atim.tv_sec;
        if (cur_file_sec < atime_start_ || cur_file_sec > atime_end_)
        {
            return false;
        }
    }

    if (type_ & FILTER_FILE_MTIME)
    {
        time_t cur_file_sec = file_header.metadata_.st_mtim.tv_sec;
        if (cur_file_sec < mtime_start_ || cur_file_sec > mtime_end_)
        {
            return false;
        }
    }

    if (type_ & FILTER_FILE_CTIME)
    {
        time_t cur_file_sec = file_header.metadata_.st_ctim.tv_sec;
        if (cur_file_sec < ctime_start_ || cur_file_sec > ctime_end_)
        {
            return false;
        }
    }

    return true;
}
