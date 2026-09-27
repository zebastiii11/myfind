#include <iostream>
#include <unistd.h>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;

// Speichert alle Optionen und Argumente nach dem Einlesen
struct Options
{
    bool recursive = false;
    bool caseInsensitive = false;
    std::string searchPath;
    std::vector<std::string> filenames;
};

// String in Kleinbuchstaben
std::string toLower(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c)
                   { return std::tolower(c); });

    return text;
}

// vergleicht zwei Dateinamen
bool namesMatch(const std::string &actualName, const std::string &searchedName, bool caseInsensitive)
{
    if (caseInsensitive)
    {
        return toLower(actualName) == toLower(searchedName);
    }

    return actualName == searchedName;
}

// sucht nur im direkten Ordner, nicht in den Unterordnern
void searchNonRecursive(const Options &options)
{
    for (const std::string &filename : options.filenames)
    {
        for (const fs::directory_entry &entry : fs::directory_iterator(options.searchPath))
        {
            if (entry.is_regular_file() && namesMatch(entry.path().filename().string(), filename, options.caseInsensitive))
            {
                std::cout << getpid() << ": " << filename << ": " << fs::absolute(entry.path()).string() << std::endl;
            }
        }
    }
}

// sucht auch in den Unterordnern
void searchRecursive(const Options &options)
{
    for (const std::string &filename : options.filenames)
    {
        for (const fs::directory_entry &entry : fs::recursive_directory_iterator(options.searchPath))
        {
            if (entry.is_regular_file() && namesMatch(entry.path().filename().string(), filename, options.caseInsensitive))
            {
                std::cout << getpid() << ": " << filename << ": " << fs::absolute(entry.path()).string() << std::endl;
            }
        }
    }
}

int main(int argc, char *argv[])
{
    Options options;

    int option;
    while ((option = getopt(argc, argv, "Ri")) != -1)
    {
        switch (option)
        {
        case 'R':
            options.recursive = true;
            break;
        case 'i':
            options.caseInsensitive = true;
            break;
        default:
            std::cerr << "Unknown option" << std::endl;
            return 1;
        }
    }

    if (optind >= argc)
    {
        std::cerr << "Missing searchpath" << std::endl;
        return 1;
    }

    options.searchPath = argv[optind];
    optind++;

    if (optind >= argc)
    {
        std::cerr << "Missing filename" << std::endl;
        return 1;
    }

    while (optind < argc)
    {
        options.filenames.push_back(argv[optind]);
        optind++;
    }

    std::cout << "recursive = " << options.recursive << std::endl;
    std::cout << "caseInsensitive = " << options.caseInsensitive << std::endl;
    std::cout << "searchPath = " << options.searchPath << std::endl;

    for (const std::string &filename : options.filenames)
    {
        std::cout << "filename = " << filename << std::endl;
    }

    if (options.recursive)
    {
        searchRecursive(options);
    }
    else
    {
        searchNonRecursive(options);
    }

    return 0;
}