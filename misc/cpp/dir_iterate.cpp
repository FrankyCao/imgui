#include "dir_iterate.h"
#if defined(__WIN32__) || defined(_WIN32)
#include <windows.h>
#include "dirent_portable.h"
#else
#include <dirent.h>
#endif
#include <string.h>
#include <iostream>
#include <string>
#include <algorithm>

static std::vector<std::string> split(std::string str, std::string pattern)
{
    std::string::size_type pos;
    std::vector<std::string> result;
    str += pattern;//扩展字符串以方便操作
    int size=str.size();

    for(int i = 0; i < size; i++)
    {
        pos = str.find(pattern, i);
        if(pos < size)
        {
            std::string s = str.substr(i, pos - i);
            result.push_back(s);
            i = pos + pattern.size() - 1;
        }
    }
    return result;
}

int DIR_Iterate(std::string directory, std::vector<std::string>& filesAbsolutePath, std::vector<std::string>& filesname, bool surfix, std::string filters)
{
    DIR* dir = opendir(directory.c_str());
    if ( dir == NULL )
    {
        //std::cout << directory << " is not a directory or not exist!" << std::endl;
        return -1;
    }
    
    struct dirent* d_ent = NULL;
    char dot[3] = ".";
    char dotdot[6] = "..";
    std::vector<std::string> filter_array;
    if (!filters.empty())
    {
        filter_array = split(filters, "|");
    }
    while ( (d_ent = readdir(dir)) != NULL )
    {
        if ( (strcmp(d_ent->d_name, dot) != 0)
            && (strcmp(d_ent->d_name, dotdot) != 0) )
        {
            if ( d_ent->d_type == DT_DIR )
            {
                std::string newDirectory = directory + std::string("/") + std::string(d_ent->d_name);
                if( directory[directory.length()-1] == '/')
                {
                    newDirectory = directory + std::string(d_ent->d_name);
                }
                
                if ( -1 == DIR_Iterate(newDirectory, filesAbsolutePath, filesname, surfix, filters) )
                {
                    return -1;
                }
            }
            else
            {
                if (d_ent->d_name[0] == '.')
                continue;
                std::string absolutePath = directory + std::string("/") + std::string(d_ent->d_name);
                if( directory[directory.length()-1] == '/')
                {
                    absolutePath = directory + std::string(d_ent->d_name);
                }
                char * pos = strchr(d_ent->d_name, '.');
                std::string surfix_name = std::string(pos);
                if (!surfix)
                {
                    *pos = '\0';
                }
                if (filter_array.size() == 0)
                {
                    filesAbsolutePath.push_back(absolutePath);
                    filesname.push_back(d_ent->d_name);
                }
                else
                {
                    for (auto filter : filter_array)
                    {
                        if (surfix_name.compare(filter) == 0)
                        {
                            filesAbsolutePath.push_back(absolutePath);
                            filesname.push_back(d_ent->d_name);
                            break;
                        }
                    }
                }
            }
        }
    }
    closedir(dir);
    std::sort(filesAbsolutePath.begin(), filesAbsolutePath.end());
    std::sort(filesname.begin(), filesname.end());
    return 0;
}