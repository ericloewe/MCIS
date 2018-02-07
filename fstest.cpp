/*#include <iostream>
#include <fstream>
#include "boost/filesystem.hpp" //Included as part of the C++17 standard

#define testFolder "./testinputs"

namespace fs = boost::filesystem;

int main()
{
    fs::path filepath();


    if (fs::is_directory(testFolder))
    {
        for (fs::directory_iterator itr{testFolder}; itr != fs::directory_iterator(); ++itr)
        {
            std::cout << itr -> path << std::endl;
        }
    }

}*/

#include <iostream>
#include <fstream>
#include <string>

#define testFolder "./testinputs/"
#define testLen 8
#define testStart 1

int main()
{
    std::string path;
    std::ifstream infile;

    for (int i = 1; i <= testLen; i++)
    {
        path = testFolder;
        path += "test";
        path += std::to_string(i);
        std::cout << "Opening file "<< path << "  ... ";

        infile.open(path);
        if (!infile.good())
        {
            std::cout << "Failed!" << std::endl;
        }
        else
        {
            std::cout << "Succeeded!" << std::endl;
        }
        infile.close();
    }
}



