#include "Aes.hpp"

Aes::Aes(std::string file_path, std::string password)
{
    file_path_.assign(file_path);
    MD5((unsigned char *)(password.c_str()), password.length(), key_);
}

Aes::~Aes()
{
}

bool Aes::encrypt()
{
    AES_KEY aes_key;
    AES_set_encrypt_key(key_, 128, &aes_key);

    File file_in(file_path_);
    if (!file_in.open_file(std::ios::in | std::ios::binary))
    {
        return false;
    }

    BackupInfo info = file_in.read_backup_info();

    char buf_in[AES_BLOCK_SIZE], buf_out[AES_BLOCK_SIZE];
    unsigned char ivec[AES_BLOCK_SIZE] = {0};

    std::filesystem::path ept_path(file_path_);
    ept_path += FILE_SUFFIX_ENCRYPT;
    File file_out(ept_path);
    if (!file_out.open_file(std::ios::out | std::ios::binary | std::ios::trunc))
    {
        return false;
    }
    file_out.write_backup_info(info);

    AES_cbc_encrypt(key_, (unsigned char *)buf_out, AES_BLOCK_SIZE, &aes_key, ivec, AES_ENCRYPT);
    file_out.write((const char *)buf_out, sizeof buf_out);

    while (file_in.read(buf_in, AES_BLOCK_SIZE))
    {
        AES_cbc_encrypt((unsigned char *)buf_in, (unsigned char *)buf_out, AES_BLOCK_SIZE, &aes_key, ivec, AES_ENCRYPT);
        file_out.write((const char *)buf_out, sizeof buf_out);
    }

    char delta = AES_BLOCK_SIZE - file_in.gcount();
    memset(buf_in + file_in.gcount(), delta, delta);
    AES_cbc_encrypt((unsigned char *)buf_in, (unsigned char *)buf_out, AES_BLOCK_SIZE, &aes_key, ivec, AES_ENCRYPT);
    file_out.write((const char *)buf_out, sizeof buf_out);

    file_in.close();
    file_out.close();
    return true;
}

int Aes::decrypt()
{
    if (file_path_.extension() != FILE_SUFFIX_ENCRYPT)
    {
        return -2;
    }

    AES_KEY aes_key;
    AES_set_decrypt_key(key_, 128, &aes_key);

    File file_in(file_path_);
    if (!file_in.open_file(std::ios::in | std::ios::binary))
    {
        return -2;
    }
    BackupInfo info = file_in.read_backup_info();

    char buf_in[AES_BLOCK_SIZE], buf_out[AES_BLOCK_SIZE];
    unsigned char ivec[AES_BLOCK_SIZE] = {0};

    file_in.read(buf_in, AES_BLOCK_SIZE);
    AES_cbc_encrypt((unsigned char *)buf_in, (unsigned char *)buf_out, AES_BLOCK_SIZE, &aes_key, ivec, AES_DECRYPT);
    if (memcmp(key_, buf_out, sizeof key_))
    {
        return -1;
    }

    std::filesystem::path dpt_path(file_path_);
    dpt_path.replace_extension("");
    File file_out(dpt_path);
    if (!file_out.open_file(std::ios::out | std::ios::binary | std::ios::trunc))
    {
        return -2;
    }
    file_out.write_backup_info(info);

    while (file_in.read(buf_in, AES_BLOCK_SIZE))
    {
        AES_cbc_encrypt((unsigned char *)buf_in, (unsigned char *)buf_out, AES_BLOCK_SIZE, &aes_key, ivec, AES_DECRYPT);
        if (file_in.peek() == EOF)
        {
            file_out.write(buf_out, AES_BLOCK_SIZE - buf_out[AES_BLOCK_SIZE - 1]);
            break;
        }
        file_out.write(buf_out, AES_BLOCK_SIZE);
    }

    file_in.close();
    file_out.close();
    return 0;
}