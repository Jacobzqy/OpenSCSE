#include "CmdParser.hpp"
#include "Task.hpp"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "backuppro: missing operand" << std::endl;
        std::cout << "Try 'backuppro --help' for more information" << std::endl;
    }
    CmdParser cmd_parser;
    if (!cmd_parser.cmd_parse(argc, argv))
    {
        std::cout << "Try 'backuppro --help' for more information" << std::endl;
        return -1;
    }

    if (cmd_parser.flag_help_)
    {
        return 0;
    }
    if (cmd_parser.flag_backup_)
    {
        Task task(cmd_parser.arg_input_, cmd_parser.arg_output_);

        unsigned char mode = 0;
        if (cmd_parser.flag_encrypt_)
        {
            mode |= BACKUP_MODE_ENCRYPT;
            if (!cmd_parser.flag_password_)
            {
                std::string pwd1, pwd2;
                std::cout << "Input password: ";
                std::cin >> pwd1;
                std::cout << "Input password again: ";
                std::cin >> pwd2;
                if (pwd1 != pwd2)
                {
                    std::cout << "[error]entered passwords differ" << std::endl;
                    return -1;
                }

                cmd_parser.flag_password_ = true;
                cmd_parser.arg_password_ = pwd2;

                if (!cmd_parser.check_password())
                {
                    return -1;
                }
            }
        }

        task.set_mode(mode);

        Filter filter;
        if (cmd_parser.flag_path_)
        {
            filter.set_path_filter(cmd_parser.arg_path_);
        }
        if (cmd_parser.flag_name_)
        {
            filter.set_name_filter(cmd_parser.arg_name_);
        }
        if (cmd_parser.flag_atime_)
        {
            filter.set_access_time(cmd_parser.atime_start_, cmd_parser.atime_end_);
        }
        if (cmd_parser.flag_mtime_)
        {
            filter.set_modify_time(cmd_parser.mtime_start_, cmd_parser.mtime_end_);
        }
        if (cmd_parser.flag_ctime_)
        {
            filter.set_change_time(cmd_parser.ctime_start_, cmd_parser.ctime_end_);
        }

        task.set_filter(filter);

        if (!task.backup(cmd_parser.arg_password_))
        {
            return -1;
        }
        else
        {
            std::cout << "Done" << std::endl;
        }
    }

    if (cmd_parser.flag_restore_)
    {
        Task task(cmd_parser.arg_input_, cmd_parser.arg_output_);
        if (!task.get_backup_info())
        {
            return -1;
        }
        if ((task.get_backup_mode() & BACKUP_MODE_ENCRYPT) && !cmd_parser.flag_password_)
        {
            std::cout << "Input password: ";
            std::cin >> cmd_parser.arg_password_;
        }
        task.restore_metadata(cmd_parser.flag_metadata_);
        if (!task.restore(cmd_parser.arg_password_))
        {
            return -1;
        }
        else
        {
            std::cout << "Done" << std::endl;
        }
    }

    if (cmd_parser.flag_compare_)
    {
        Task task(cmd_parser.arg_compare_, cmd_parser.arg_compare_);
        if (!task.compare())
        {
            return -1;
        }
        else
        {
            std::cout << "Done" << std::endl;
        }
    }

    return 0;
}