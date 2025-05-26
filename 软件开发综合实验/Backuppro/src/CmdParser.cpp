#include "CmdParser.hpp"

CmdParser::CmdParser() : options_("backuppro", "a backup master for Linux")
{
    options_.set_width(MAX_HELP_WIDTH).add_options()("b, backup", "备份文件", cxxopts::value(flag_backup_))("r, restore", "恢复文件", cxxopts::value(flag_restore_))("c, compare", "比较指定备份文件与现有文件的差异", cxxopts::value(arg_compare_))("i, input", "输入文件路径", cxxopts::value(arg_input_))("o, output", "输出文件路径", cxxopts::value(arg_output_))("p, password", "指定密码", cxxopts::value(arg_password_))("h, help", "查看帮助文档", cxxopts::value(flag_help_));

    options_.set_width(MAX_HELP_WIDTH).add_options("Backup")("e, encrypt", "备份时加密文件", cxxopts::value(flag_encrypt_))("path", "过滤路径：正则表达式", cxxopts::value(arg_path_))("name", "过滤文件名：正则表达式", cxxopts::value(arg_name_))("atime", "文件的访问时间区间，例\"2023-10-11 08:00:00 2023-10-12 20:00:00\"", cxxopts::value(arg_atime_))("mtime", "文件的修改时间区间，格式同atime", cxxopts::value(arg_mtime_))("ctime", "文件的改变时间区间，格式同atime", cxxopts::value(arg_ctime_));

    options_.set_width(MAX_HELP_WIDTH).add_options("Restore")("a, metadata", "恢复文件的元数据", cxxopts::value(flag_metadata_));
}

CmdParser::~CmdParser()
{
}

bool CmdParser::cmd_parse(int argc, char **argv)
{
    try
    {
        parse_result_ = options_.parse(argc, argv);
    }
    catch (cxxopts::OptionException e)
    {
        std::cout << e.what() << std::endl;
        return false;
    }

    flag_input_ = parse_result_.count("input");
    flag_output_ = parse_result_.count("output");
    flag_password_ = parse_result_.count("password");
    flag_path_ = parse_result_.count("path");
    flag_name_ = parse_result_.count("name");
    flag_atime_ = parse_result_.count("atime");
    flag_mtime_ = parse_result_.count("mtime");
    flag_ctime_ = parse_result_.count("ctime");
    flag_compare_ = parse_result_.count("compare");

    if (flag_help_)
    {
        std::cout << options_.help() << std::endl;
        return true;
    }

    if (!arg_check())
    {
        return false;
    }

    return true;
}

bool CmdParser::arg_check()
{
    if (flag_backup_ + flag_restore_ + flag_compare_ != 1)
    {
        std::cout << "[error]invalid parameter" << std::endl;
        return false;
    }
    if (flag_backup_ && !check_args_backup())
    {
        return false;
    }
    if (flag_restore_ && !check_args_restore())
    {
        return false;
    }
    if (flag_compare_ && !check_args_compare())
    {
        return false;
    }
    if (flag_atime_ && !convert_time(arg_atime_, atime_start_, atime_end_))
    {
        return false;
    }
    if (flag_mtime_ && !convert_time(arg_mtime_, mtime_start_, mtime_end_))
    {
        return false;
    }
    if (flag_ctime_ && !convert_time(arg_ctime_, ctime_start_, ctime_end_))
    {
        return false;
    }
    if (flag_backup_ && flag_password_ && !check_password())
    {
        return false;
    }
    if (!check_reg_exp())
    {
        return false;
    }
    return true;
}

bool CmdParser::check_args_backup()
{
    if (!flag_input_ || !flag_output_)
    {
        std::cout << "[error]no input or output" << std::endl;
        return false;
    }
    if (!flag_encrypt_ && flag_password_)
    {
        std::cout << "[error]invalid parameter(using -e to encrypt)" << std::endl;
        return false;
    }
    return true;
}

bool CmdParser::check_args_restore()
{
    if (!flag_input_ || !flag_output_)
    {
        std::cout << "[error]no input or output" << std::endl;
        return false;
    }
    if (flag_compare_ || flag_encrypt_ || flag_path_ || flag_name_ || flag_atime_ || flag_ctime_ || flag_mtime_)
    {
        std::cout << "[error]invalid parameter" << std::endl;
        return false;
    }
    return true;
}

bool CmdParser::check_args_compare()
{
    if (flag_encrypt_ || flag_path_ || flag_name_ || flag_atime_ || flag_ctime_ || flag_mtime_ || flag_metadata_ || flag_input_ || flag_output_ || flag_password_)
    {
        std::cout << "[error]invalid parameter" << std::endl;
        return false;
    }
    return true;
}

bool CmdParser::convert_time(std::string time, time_t &time_start, time_t &time_end)
{
    std::string time_format("(\\d{1,4}-(?:1[0-2]|0?[1-9])-(?:0?[1-9]|[1-2]\\d|30|31)) ((?:[0-1]\\d|2[0-3]):[0-5]\\d:[0-5]\\d)");
    std::regex reg("^" + time_format + " " + time_format + "$");
    if (!std::regex_match(time, reg))
    {
        std::cout << "error:invalid time: " << time << std::endl;
        return false;
    }
    reg.assign("(\\d{1,4}-(?:1[0-2]|0?[1-9])-(?:0?[1-9]|[1-2]\\d|30|31)) ((?:[0-1]\\d|2[0-3]):[0-5]\\d:[0-5]\\d)");
    std::sregex_iterator it(time.begin(), time.end(), reg);
    time_start = string_to_time(it->str());
    it++;
    time_end = string_to_time(it->str());
    return true;
}

time_t CmdParser::string_to_time(std::string time)
{
    char *ch = (char *)time.data();
    tm tm_;
    int year, month, day, hour, minute, second;
    sscanf(ch, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);
    tm_.tm_year = year - 1900;
    tm_.tm_mon = month - 1;
    tm_.tm_mday = day;
    tm_.tm_hour = hour;
    tm_.tm_min = minute;
    tm_.tm_sec = second;
    tm_.tm_isdst = 0;
    time_t t_ = mktime(&tm_);
    return t_;
}

bool CmdParser::check_reg_exp()
{
    try
    {
        if (flag_path_)
        {
            std::regex reg(arg_path_);
        }
        if (flag_name_)
        {
            std::regex reg(arg_name_);
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "error: invalid regular expression: ";
        std::cout << e.what() << std::endl;
        return false;
    }
    return true;
}

bool CmdParser::check_password()
{
    if (arg_password_.length() < 8)
    {
        std::cout << "[error]Password must contain at least 8 characters" << std::endl;
        return false;
    }
    return true;
}

void CmdParser::print_error_info()
{
    std::cout << "Try 'backuppro --help' for more information" << std::endl;
}