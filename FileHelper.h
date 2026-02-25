#ifndef FILEHELPER_H
#define FILEHELPER_H

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

class FileHelper
{
public:
    static std::string file_name()
    {
        // 1. Declare the folder path variable
        fs::path f_p = "./filea";

        // 2. Define naming components
        std::string f_n = "image"; 
        std::string f_x = ".ppm";

        // 3. Check if directory exists; if not, return the first name
        if (!fs::exists(f_p) || !fs::is_directory(f_p))
        {
            // Use / operator to join folder and filename safely
            return (f_p / (f_n + f_x)).string();
        }

        // 4. Count existing regular files in the directory
        int count = 0;
        for (const auto &entry : fs::directory_iterator(f_p))
        {
            if (entry.is_regular_file())
            {
                count++;
            }
        };

        // 5. Construct final filename: "./filea/image[count+1].ppm"
        fs::path f_f_n = f_p / (f_n + std::to_string(count + 1) + f_x);

        return f_f_n.string();
    }
};

#endif
