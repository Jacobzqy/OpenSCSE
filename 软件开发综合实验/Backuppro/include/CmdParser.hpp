#pragma once

#include "cxxopts.hpp"
#include "File.hpp"

#define MAX_HELP_WIDTH 160

class CmdParser
{
private:
    cxxopts::Options options_;
    cxxopts::ParseResult parse_result_;

public:
    bool flag_backup_;
    bool flag_restore_;
    bool flag_encrypt_;
    bool flag_metadata_;
    bool flag_help_;

    bool flag_compare_;
    bool flag_input_;
    bool flag_output_;
    bool flag_password_;
    bool flag_path_;
    bool flag_name_;
    bool flag_atime_;
    bool flag_mtime_;
    bool flag_ctime_;

    std::string arg_compare_;
    std::string arg_input_;
    std::string arg_output_;
    std::string arg_password_;
    std::string arg_path_;
    std::string arg_name_;
    time_t atime_start_, atime_end_;
    time_t mtime_start_, mtime_end_;
    time_t ctime_start_, ctime_end_;

private:
    std::string arg_atime_;
    std::string arg_mtime_;
    std::string arg_ctime_;

public:
    CmdParser();
    ~CmdParser();

    bool cmd_parse(int argc, char **argv);

    bool check_password();
    void print_error_info();

private:
    bool arg_check();
    bool check_args_backup();
    bool check_args_restore();
    bool check_args_compare();
    bool convert_time(std::string time, time_t &time_start, time_t &time_end);
    time_t string_to_time(std::string time);
    bool check_reg_exp();
};