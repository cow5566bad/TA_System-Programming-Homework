//
//  file_list.h
//  mp1_2
//
//  Created by 謝子然 on 2018/9/26.
//  Copyright © 2018年 謝子然. All rights reserved.
//

#ifndef file_list_h
#define file_list_h
#include <string>
#include <vector>
using std::vector;
using std::string;

void get_file_list(const char * path, vector<string> & file_list);
bool isExist(string filename, vector<string> & file_list);

#endif /* file_list_h */
