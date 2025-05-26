#include "Task.hpp"

Task::Task(std::string src_path, std::string dest_path)
{
    src_path_ = std::filesystem::absolute(src_path);
    dest_path_ = std::filesystem::absolute(dest_path);
    memset(&info_, 0, sizeof info_);
    info_.mode_ = BACKUP_MODE_ENCRYPT;
    info_.timestamp_ = std::time(nullptr);
    memcpy(info_.backup_path_, src_path.c_str(), std::min(src_path.length(), sizeof info_.backup_path_));

    restore_metadata_ = false;
}

Task::~Task()
{
}

bool Task::backup(std::string password)
{
    // 判断源路径是否存在
    if (!std::filesystem::exists(src_path_))
    {
        std::cout << "[error]no such file or directory: " << src_path_.string() << std::endl;
        return false;
    }

    // 判断目标文件名是否符合要求
    std::string name = dest_path_.filename();
    std::regex reg("^[.]*[\\w]+[\\w.-]*$");
    if (!std::regex_match(name, reg))
    {
        std::cout << "[error]invalid file name: " << name << std::endl;
        return false;
    }

    // 输出执行过程
    std::cout << "PACKING..." << std::endl;

    // 打包
    Packer packer(src_path_, dest_path_, filter_);

    if (!packer.pack())
    {
        std::cout << "[error]failed to pack file" << std::endl;
        return false;
    }

    dest_path_ += FILE_SUFFIX_PACK;

    // 加密
    if (info_.mode_ & BACKUP_MODE_ENCRYPT)
    {
        std::cout << "ENCRYPTING..." << std::endl;
        Aes aes(dest_path_, password);
        if (!aes.encrypt())
        {
            std::cout << "[error]faild to encrypt file" << std::endl;
            return false;
        }
        std::filesystem::remove_all(dest_path_);
        dest_path_ += FILE_SUFFIX_ENCRYPT;
    }

    File file(dest_path_);
    file.open_file(std::ios::in | std::ios::out | std::ios::binary);
    file.write_backup_info(info_);
    file.close();

    return true;
}

void Task::set_mode(unsigned char mode)
{
    info_.mode_ = mode;
}

void Task::set_filter(const Filter filter)
{
    filter_ = filter;
}

bool Task::restore(std::string password)
{
    // 判断源路径是否存在
    if (!std::filesystem::exists(src_path_))
    {
        std::cout << "[error]no such file or directory: " << src_path_.string() << std::endl;
        return false;
    }

    // 解密
    if (info_.mode_ & BACKUP_MODE_ENCRYPT)
    {
        std::cout << "DECRYPTING..." << std::endl;

        Aes aes(src_path_, password);
        int status = aes.decrypt();
        if (status == -2)
        {
            std::cout << "[error]faild to decrypt file" << std::endl;
            return false;
        }
        else if (status == -1)
        {
            std::cout << "[error]wrong password" << std::endl;
            return false;
        }
        src_path_.replace_extension("");
    }

    // 解包
    std::cout << "UNPACKING..." << std::endl;
    Packer packer(src_path_, dest_path_, filter_);
    if (!packer.unpack(restore_metadata_))
    {
        std::cout << "[error]failed to unpack file" << std::endl;
        return false;
    }

    if (info_.mode_ & BACKUP_MODE_ENCRYPT)
    {
        std::filesystem::remove_all(src_path_);
    }
    return true;
}

void Task::restore_metadata(bool restore_metadata)
{
    restore_metadata_ = restore_metadata;
}

bool Task::get_backup_info()
{
    return get_backup_info(src_path_, info_);
}

unsigned char Task::get_backup_mode()
{
    return info_.mode_;
}

bool Task::get_backup_info(std::string file_path, BackupInfo &info)
{
    File file(file_path);
    if (file.open_file(std::ios::in | std::ios::binary))
    {
        info = file.read_backup_info();
        file.close();
        return true;
    }
    std::cout << "[error]failed to open file: " << file_path << std::endl;
    return false;
}

bool Task::compare()
{
    std::cout << "COMPARING..." << std::endl;

    Compare comparative(src_path_);

    if (!comparative.compare())
    {
        std::cout << "[error]failed to compare file" << std::endl;
        return false;
    }

    return true;
}