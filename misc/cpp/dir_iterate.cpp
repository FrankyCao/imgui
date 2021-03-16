#include "dir_iterate.h"
#if defined(__WIN32__) || defined(_WIN32)
#include <windows.h>
#include "dirent_portable.h"
#else
#include <dirent.h>
#endif
#include <iostream>
#include <string>
#include <algorithm>

using namespace std;

int DIR_Iterate(string directory, vector<string>& filesAbsolutePath, vector<string>& filesname, bool surfix)
{
    DIR* dir = opendir(directory.c_str());
    if ( dir == NULL )
    {
        cout<<directory<<" is not a directory or not exist!"<<endl;
        return -1;
    }
    struct dirent* d_ent = NULL;
    char dot[3] = ".";
    char dotdot[6] = "..";
    
    while ( (d_ent = readdir(dir)) != NULL )
    {
        if ( (strcmp(d_ent->d_name, dot) != 0)
            && (strcmp(d_ent->d_name, dotdot) != 0) )
        {
            if ( d_ent->d_type == DT_DIR )
            {
                string newDirectory = directory + string("/") + string(d_ent->d_name);
                if( directory[directory.length()-1] == '/')
                {
                    newDirectory = directory + string(d_ent->d_name);
                }
                
                if ( -1 == DIR_Iterate(newDirectory, filesAbsolutePath, filesname, surfix) )
                {
                    return -1;
                }
            }
            else
            {
                if (d_ent->d_name[0] == '.')
                continue;
                string absolutePath = directory + string("/") + string(d_ent->d_name);
                if( directory[directory.length()-1] == '/')
                {
                    absolutePath = directory + string(d_ent->d_name);
                }
                filesAbsolutePath.push_back(absolutePath);
                if (!surfix)
                {
                    char * pos = strchr(d_ent->d_name, '.');
                    *pos = '\0';
                }
                filesname.push_back(d_ent->d_name);
            }
        }
    }
    closedir(dir);
    std::sort(filesAbsolutePath.begin(), filesAbsolutePath.end());
    std::sort(filesname.begin(), filesname.end());
    return 0;
}