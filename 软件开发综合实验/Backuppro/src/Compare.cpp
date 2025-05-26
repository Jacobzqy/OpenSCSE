#include "Compare.hpp"

Compare::Compare(std::string bak_path)
{
    bak_path_ = bak_path;
}

Compare::~Compare()
{
}

bool Compare::compare()
{
    // 打开备份文件
    std::filesystem::path bak_path(bak_path_);

    File bak_file(bak_path);
    if (!bak_file.open_file(std::ios::in | std::ios::binary))
    {
        return false;
    }

    // 跳过开头
    bak_file.seekg(sizeof(BackupInfo), std::ios::beg);

    FileHeader file_header = {0};

    while (bak_file.peek() != EOF)
    {
        // 依次读出每个文件的哈希值，与现有文件进行比较
        memset(&file_header, 0, sizeof file_header);
        file_header = bak_file.read_file_header();

        std::cout << file_header.name_;

        std::filesystem::path cur_path(file_header.name_);
        File input_file(cur_path);
        if (strcmp((const char *)file_header.md5_, (const char *)input_file.get_file_header().md5_) == 0)
        {
            std::cout << "     Same" << std::endl;
        }
        else
        {
            std::cout << "     Different" << std::endl;
        }
        if (input_file.get_file_type() == FILE_TYPE_NORMAL)
        {
            size_t file_size = file_header.metadata_.st_size;
            bak_file.seekg(file_size, std::ios::cur);
        }
    }

    bak_file.close();
    return true;
}