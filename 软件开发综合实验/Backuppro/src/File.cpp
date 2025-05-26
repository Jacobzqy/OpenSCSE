#include "File.hpp"

File::File(FileHeader file_header)
{
    file_header_ = file_header;
    file_path_.assign(file_header.name_);

    if (is_hardlink())
    {
        std::filesystem::remove_all(file_header_.name_);
        std::filesystem::create_hard_link(file_header_.linkname_, file_header_.name_);
        return;
    }

    switch (get_file_type())
    {
    case FILE_TYPE_NORMAL:
        if (file_path_.parent_path().string().length() == 0)
            break;
        std::filesystem::create_directories(file_path_.parent_path());
        break;
    case FILE_TYPE_DIRECTORY:
        std::filesystem::create_directories(file_path_.string());
        break;
    case FILE_TYPE_SYMBOLIC_LINK:
        std::filesystem::remove_all(file_header_.name_);
        std::filesystem::create_symlink(file_header_.linkname_, file_header_.name_);
        break;
    case FILE_TYPE_FIFO:
        std::filesystem::remove_all(file_header_.name_);
        mkfifo(file_header_.name_, file_header_.metadata_.st_mode);
        break;
    default:
        break;
    }
}

File::File(std::filesystem::path file_path)
{
    file_path_ = file_path;

    memset(&file_header_, 0, sizeof file_header_);

    lstat(file_path_.string().c_str(), &(file_header_.metadata_));
    strcpy(file_header_.name_, file_path_.string().c_str());

    if (get_file_type() != FILE_TYPE_FIFO)
    {
        calculate_MD5(file_path_, file_header_.md5_);
    }

    if (get_file_type() == FILE_TYPE_SYMBOLIC_LINK)
    {
        char link_buf[MAX_FILE_PATH] = {0};
        readlink(file_path_.c_str(), link_buf, MAX_FILE_PATH);
        strcpy(file_header_.linkname_, link_buf);
    }
}

File::~File()
{
}

bool File::open_file(std::ios_base::openmode mode)
{
    open(file_path_.string(), mode);
    return is_open();
}

FileHeader File::read_file_header()
{
    read((char *)&file_header_, sizeof(FileHeader));
    return file_header_;
}

BackupInfo File::read_backup_info()
{
    BackupInfo info;
    read((char *)&info, sizeof info);
    return info;
}

void File::write_backup_info()
{
    BackupInfo info = {0};
    write_backup_info(info);
}

void File::write_backup_info(BackupInfo info)
{
    write((char *)&info, sizeof info);
}

size_t File::get_file_size()
{
    return file_header_.metadata_.st_size;
}

FileType File::get_file_type()
{
    return File::get_file_type(file_header_);
}

FileType File::get_file_type(const FileHeader file_header)
{
    switch (file_header.metadata_.st_mode & S_IFMT)
    {
    // 普通文件
    case S_IFREG:
        return FILE_TYPE_NORMAL;
    // 目录文件
    case S_IFDIR:
        return FILE_TYPE_DIRECTORY;
    // 符号链接
    case S_IFLNK:
        return FILE_TYPE_SYMBOLIC_LINK;
    // 通道文件
    case S_IFIFO:
        return FILE_TYPE_FIFO;
    default:
        return FILE_TYPE_OTHERS;
    }
}

FileHeader File::get_file_header()
{
    return file_header_;
}

bool File::is_hardlink()
{
    return file_header_.metadata_.st_nlink > 1;
}

void File::restore_metadata()
{
    // 如果是符号链接，只恢复符号链接自身的元数据
    struct stat *metadata = &(file_header_.metadata_);

    // 还原文件权限信息
    chmod(file_header_.name_, metadata->st_mode);

    // 还原文件属主
    lchown(file_header_.name_, metadata->st_uid, metadata->st_gid);

    // 还原时间戳
    timespec tim[2] = {metadata->st_atim, metadata->st_mtim};
    utimensat(AT_FDCWD, file_header_.name_, tim, AT_SYMLINK_NOFOLLOW);
}

void File::calculate_MD5(std::filesystem::path file_path, unsigned char *md5)
{
    open_file(std::ios::in | std::ios::binary);

    MD5_CTX md5Context;
    MD5_Init(&md5Context);

    unsigned char data[1024];
    while (read((char *)&data, sizeof data))
    {
        MD5_Update(&md5Context, data, sizeof data);
    }
    int size = gcount();
    read((char *)&data, size);
    MD5_Update(&md5Context, data, size);

    MD5_Final(md5, &md5Context);

    close();
}