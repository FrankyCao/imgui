#ifndef __DIR_ITERATE__
#define __DIR_ITERATE__
#include <string>
#include <vector>

int DIR_Iterate(std::string directory, std::vector<std::string>& filesAbsolutePath, std::vector<std::string>& filesname, bool surfix = false, std::string filters = "");

#endif /* __DIR_ITERATE__ */
