#ifndef FILEHELPER_H
#define FILEHELPER_H

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

class FileHelper
{
public:
    // Returns the number of regular files in a directory
    static int count_files(const std::string &folder_path)
    {
        if (!fs::exists(folder_path) || !fs::is_directory(folder_path))
        {
            return 0; // Return 0 if folder doesn't exist
        }

        int count = 0;
        for (const auto &entry : fs::directory_iterator(folder_path))
        {
            if (entry.is_regular_file())
            {
                count++;
            }
        }
        return count;
    }
};

#endif
