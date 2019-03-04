//
//  commit_struct.h
//  test_binary_io
//
//  Created by 謝子然 on 2018/10/14.
//  Copyright © 2018 謝子然. All rights reserved.
//

#ifndef commit_struct_h
#define commit_struct_h
#include <vector>
#include <string>
#include <utility>
#include <stdint.h>
using std::vector;
using std::string;
using std::pair;

typedef struct commit_header {
    uint32_t  number_of_commit;
    /* max number of file = 1000 */
    uint32_t number_of_file;
    uint32_t number_of_add;
    uint32_t number_of_modify;
    uint32_t number_of_copy;
    uint32_t number_of_del;
    /* max commit size = 784036 bytes */
    uint32_t commit_size;
    commit_header() {
        number_of_commit = number_of_file = number_of_add = number_of_modify = number_of_copy = number_of_del = 0;
    }
} commit_header;

typedef struct commit_struct {
    vector<pair<uint8_t, string> > add_list;
    vector<pair<uint8_t, string> > modify_list;
    vector<pair<pair<uint8_t, string>, pair<uint8_t, string> > > copy_list;
    vector<pair<uint8_t, string> > del_list;
    vector<pair<uint8_t, string> > file_list;
    vector<string> md5_list;
} commit_struct;

uint32_t caculate_commit_size(commit_header * headptr, commit_struct * commitptr) {
    /* header size at least 7 uint32_t => 28 bytes */
    uint32_t sum_size = 28;
    
    for(int i = 0; i < commitptr->add_list.size(); i++)
        sum_size += commitptr->add_list[i].first + 1;
    for(int i = 0; i < commitptr->copy_list.size(); i++)
        sum_size += commitptr->copy_list[i].first.first + commitptr->copy_list[i].second.first + 2;
    for(int i = 0; i < commitptr->modify_list.size(); i++)
        sum_size += commitptr->modify_list[i].first + 1;
    for(int i = 0; i < commitptr->del_list.size(); i++)
        sum_size += commitptr->del_list[i].first + 1;
    /* filename +md5 */
    for(int i = 0; i < commitptr->file_list.size(); i++)
        sum_size += commitptr->file_list[i].first + 1 + 16; //md5 16 bytes
    return sum_size;
}

#endif /* commit_struct_h */
