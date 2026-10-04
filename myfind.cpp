#include <iostream>
#include <unistd.h>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <sys/wait.h>
#include <sstream>

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
void searchNonRecursive(std::string &searchPath, std::string &filename, bool caseInsensitive)
{
    for (const fs::directory_entry &entry : fs::directory_iterator(searchPath))
    {
        if (entry.is_regular_file() && namesMatch(entry.path().filename().string(), filename, caseInsensitive))
        {
            std::ostringstream oss;
            oss << getpid() << ": " << filename << ": " << fs::absolute(entry.path().string()) << "\n";
            std::string line = oss.str();

            write(STDOUT_FILENO, line.data(), line.size());
        }
    }
}

// sucht auch in den Unterordnern
void searchRecursive(std::string &searchPath, std::string &filename, bool caseInsensitive)
{

    for (const fs::directory_entry &entry : fs::recursive_directory_iterator(searchPath))
    {
        if (entry.is_regular_file() && namesMatch(entry.path().filename().string(), filename, caseInsensitive))
        {
            std::ostringstream oss;
            oss << getpid() << ": " << filename << ": " << fs::absolute(entry.path().string()) << "\n";
            std::string line = oss.str();

            write(STDOUT_FILENO, line.data(), line.size());
        }
    }
}

// To ensure a correct output of the search results and prevent race conditions
// the output strings will first be assembled and then be written into console with one atomic write execution.
// see protocol for more info

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

    std::vector<pid_t> child_pids;

    for (std::string &filename : options.filenames)
    {
        std::cout << "filename = " << filename << std::endl;

        pid_t pid = fork();

        if (pid < 0)
        {
            std::cout << "This shouldnt have happened, fork failed";
            return 1;
        }
        else if (pid == 0)
        {

            if (options.recursive)
            {
                searchRecursive(options.searchPath, filename, options.caseInsensitive);
            }
            else
            {
                searchNonRecursive(options.searchPath, filename, options.caseInsensitive);
            }
            exit(0);
        }
        else
        {
            child_pids.push_back(pid);
        }
    }
    for (pid_t cpid : child_pids)
    {
        int status;
        if (waitpid(cpid, &status, 0) == -1)
        {
            std::cout << "wait failed";
        }
    }

    return 0;
}