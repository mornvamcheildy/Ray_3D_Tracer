#ifndef FILEHELPER_H
#define FILEHELPER_H

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

class FileHelper
{
public:
    /* Path to the folder to count from variable declaration and assignement
    std::string f_p = "./fileb";*/

    static std::string file_name()
    {

        std::string f_n = "./fileb/image"; // file name start
        std::string f_x = ".ppm";          // file extension

        if (!fs::exists("./fileb") || !fs::is_directory("./fileb"))
        {
            return f_n + f_x;
        }

        int count = 0;
        for (const auto &entry : fs::directory_iterator("./fileb"))
        {
            if (entry.is_regular_file())
            {
                count++;
            }
        };

        // Finale file name nomination f_f_n
        std::string f_f_n = f_n + std::to_string(count + 1) + f_x;

        return f_f_n;
    }
};

#endif
