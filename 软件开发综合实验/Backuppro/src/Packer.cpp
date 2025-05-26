#include "Packer.hpp"

Packer::Packer(std::string src_path, std::string dest_path, const Filter filter)
{
    src_path_.assign(src_path);
    dest_path_.assign(dest_path);
    filter_ = filter;
}

Packer::~Packer()
{
}

void Packer::dfs_file(File &pack_file, std::filesystem::path cur_path)
{
    File file(cur_path);
    FileHeader fileheader = file.get_file_header();

    // 判断是否满足过滤规则
    bool file_status = filter_.check(fileheader);

    // 输出执行过程
    std::cout << cur_path.string();
    if (file_status)
    {
        std::cout << "   True" << std::endl;
    }
    else
    {
        std::cout << "   False" << std::endl;
    }

    // 递归处理不满足过滤规则的目录
    if (!file_status)
    {
        if (file.get_file_type() == FILE_TYPE_DIRECTORY)
        {
            for (const auto &entry : std::filesystem::directory_iterator(cur_path))
            {
                dfs_file(pack_file, entry.path());
            }
        }
        return;
    }

    // 处理硬链接
    // 如果指向的inode已打包，只需记录文件路径
    // 如果指向的inode未打包，作为常规文件处理
    if (file.is_hardlink())
    {
        if (inode_table_.count(fileheader.metadata_.st_ino))
        {
            strcpy(fileheader.linkname_, inode_table_[fileheader.metadata_.st_ino].c_str());
            pack_file.write((const char *)&fileheader, sizeof fileheader);
            return;
        }
        else
        {
            fileheader.metadata_.st_nlink = 1;
            inode_table_[fileheader.metadata_.st_ino] = cur_path.string();
        }
    }

    // 具体处理逻辑
    // 如果是目录，递归处理下层
    // 如果是普通文件，直接写入即可
    // 如果是符号链接或者管道文件，写入文件头
    char buf[BLOCK_BUFFER_SIZE] = {0};
    switch (file.get_file_type())
    {
    case FILE_TYPE_DIRECTORY:
        pack_file.write((const char *)&fileheader, sizeof fileheader);
        for (const auto &entry : std::filesystem::directory_iterator(cur_path))
        {
            dfs_file(pack_file, entry.path());
        }
        break;

    case FILE_TYPE_NORMAL:
        pack_file.write((const char *)&fileheader, sizeof fileheader);

        file.open_file(std::ios::in | std::ios::binary);

        while (file.peek() != EOF)
        {
            file.read(buf, sizeof buf);
            pack_file.write(buf, file.gcount());
        }
        file.close();
        break;

    case FILE_TYPE_SYMBOLIC_LINK:
        pack_file.write((const char *)&fileheader, sizeof fileheader);
        break;

    case FILE_TYPE_FIFO:
        pack_file.write((const char *)&fileheader, sizeof fileheader);
        break;

    default:
        break;
    }
}

bool Packer::pack()
{
    dest_path_ += FILE_SUFFIX_PACK;

    // 创建文件头
    FileHeader pack_file_header = {0};
    strcpy(pack_file_header.name_, dest_path_.string().c_str());

    pack_file_header.metadata_.st_mode = S_IFREG;
    pack_file_header.metadata_.st_nlink = 1;

    // 根据文件头创建目标文件
    File pack_file(pack_file_header);
    if (!pack_file.open_file(std::ios::out | std::ios::binary | std::ios::trunc))
    {
        return false;
    }

    // 先写入备份信息
    pack_file.write_backup_info();

    // 切换工作路径
    std::filesystem::current_path(src_path_.parent_path());

    // 深度优先遍历目录树
    dfs_file(pack_file, std::filesystem::relative(src_path_, src_path_.parent_path()));

    pack_file.close();

    return true;
}

bool Packer::unpack(bool restore_metadata)
{
    // 判断是否为打包文件
    if (src_path_.extension() != FILE_SUFFIX_PACK)
    {
        return false;
    }

    // 先打开待解包文件
    File file_in(src_path_);
    if (!file_in.open_file(std::ios::in | std::ios::binary))
    {
        return false;
    }
    BackupInfo info = file_in.read_backup_info();

    // 切换到目标路径
    std::filesystem::create_directories(dest_path_);
    std::filesystem::current_path(dest_path_);

    // 创建目标文件的文件头
    char buf[BLOCK_BUFFER_SIZE] = {0};
    FileHeader fileheader = {0};
    while (file_in.peek() != EOF)
    {
        memset(&fileheader, 0, sizeof fileheader);
        fileheader = file_in.read_file_header();

        std::cout << fileheader.name_ << std::endl;

        // 创建目标文件
        File file_out(fileheader);

        // 只需要解包普通类型文件
        if (file_out.get_file_type() == FILE_TYPE_NORMAL && !file_out.is_hardlink())
        {
            if (!file_out.open_file(std::ios::out | std::ios::binary | std::ios::trunc))
            {
                return false;
            }

            size_t file_size = file_out.get_file_size();
            while (file_size >= BLOCK_BUFFER_SIZE)
            {
                file_in.read(buf, BLOCK_BUFFER_SIZE);
                file_out.write(buf, BLOCK_BUFFER_SIZE);
                file_size -= BLOCK_BUFFER_SIZE;
            }
            if (file_size)
            {
                file_in.read(buf, file_size);
                file_out.write(buf, file_size);
            }
            file_out.close();
        }

        // 恢复元数据
        if (restore_metadata)
        {
            file_out.restore_metadata();
        }
    }

    file_in.close();
    return true;
}