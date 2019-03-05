//
//  file_list.cpp
//  mp1_2
//
//  Created by 謝子然 on 2018/9/26.
//  Copyright © 2018年 謝子然. All rights reserved.
//

#include "file_list.h"
#include <vector>
#include <string>
#include <string.h>
#include <dirent.h>
#include <algorithm>
using std::vector;
using std::string;

void get_file_list(const char * path, vector<string> & file_list) {
    struct dirent *dirent;
    DIR *dir;
    dir = opendir(path);
    if(dir != nullptr) {
        while((dirent = readdir(dir)) != NULL) {
            if(strcmp(dirent->d_name, ".") != 0 and strcmp(dirent->d_name, "..") != 0 and strcmp(dirent->d_name, ".loser_record") != 0 and strcmp(dirent->d_name, ".DS_Store") != 0)
                file_list.push_back(dirent->d_name);
        }
    }
    sort(file_list.begin(), file_list.end());
}

bool isExist(string filename, vector<string> & file_list) {
    if(file_list.size() == 0)
        return false;
    else {
        for(vector<string>::iterator it = file_list.begin(); it != file_list.end(); it++) {
            if(filename == *it)
                return true;
        }
        return false;
    }
    
}

