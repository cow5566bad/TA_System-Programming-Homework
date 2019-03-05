//
//
//  Created by 謝子然 on 2018/10/14.
//  Copyright © 2018 謝子然. All rights reserved.
//  hexdump skill : hexdump -e '8/1 "%02x " "\n"' .loser_record

#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string>
#include <string.h>
#include <sys/stat.h>
#include <vector>
#include "commit_struct.h"
#include "md5.h"
#include "md5path.h"
#include "file_list.h"
#include <sstream>
#include <stdint.h>

using namespace std;

void print_log_commit(commit_header * headerptr, commit_struct * commitptr, vector<string> & old_file_name, vector<string> & old_md5) {
    cout << "# commit " << headerptr->number_of_commit << endl;
    
    cout << "[new_file]" << endl;
    for(int i = 0; i < commitptr->add_list.size(); i++)
        cout << commitptr->add_list[i].second << endl;
    
    cout << "[modified]" << endl;
    for(int i = 0; i < commitptr->modify_list.size(); i++)
        cout << commitptr->modify_list[i].second << endl;
    
    cout << "[copied]" << endl;
    for(int i = 0; i < commitptr->copy_list.size(); i++)
        cout << commitptr->copy_list[i].first.second << " => " << commitptr->copy_list[i].second.second << endl;
    
    cout << "[deleted]" << endl;
    for(int i = 0; i < commitptr->del_list.size(); i++)
        cout << commitptr->del_list[i].second << endl;
    cout << "(MD5)" << endl;
    for(int i = 0; i < old_file_name.size(); i++) {
        cout << old_file_name[i] << " " << old_md5[i] << endl;
    }
}

void print_log(ifstream & in, vector<unsigned int> & parse_size, unsigned int log_n) {
    in.seekg(0, ios::end);
    unsigned int record_total_size = in.tellg();
    
    unsigned int total_num_of_commit = parse_size.size();
    int offset = 0;
    for(int i = 0; i < log_n; i++) {
        if(i < total_num_of_commit) {
            offset = parse_size.back();
            in.seekg(-offset, ios::cur);
            commit_header * headerptr = new commit_header;
            commit_struct * commitptr = new commit_struct;
            
            unsigned int nCommit, nAdd, nModify, nCopy, nDel;
            in.read(reinterpret_cast<char *>(&nCommit), sizeof(nCommit));
            headerptr->number_of_commit = nCommit;
            in.seekg(4, ios::cur);
            in.read(reinterpret_cast<char *>(&nAdd), sizeof(nAdd));
            in.read(reinterpret_cast<char *>(&nModify), sizeof(nModify));
            in.read(reinterpret_cast<char *>(&nCopy), sizeof(nCopy));
            in.read(reinterpret_cast<char *>(&nDel), sizeof(nDel));
            in.seekg(4, ios::cur); // skip commit size block
            
            uint8_t tmp_size;
            for(int i = 0; i < nAdd; i++) {
                in.read(reinterpret_cast<char *>(&tmp_size), sizeof(tmp_size));
                char * filename = new char [static_cast<unsigned int>(tmp_size) + 1];
                in.read(filename, sizeof(char) * static_cast<unsigned int>(tmp_size));
                filename[static_cast<unsigned int>(tmp_size)] = '\0';
                string s(filename);
                commitptr->add_list.push_back(make_pair(s.length(), s));
                delete [] filename;
            }
            
            for(int i = 0; i < nModify; i++) {
                in.read(reinterpret_cast<char *>(&tmp_size), sizeof(tmp_size));
                char * filename = new char [static_cast<unsigned int>(tmp_size) + 1];
                in.read(filename, sizeof(char) * static_cast<unsigned int>(tmp_size));
                filename[static_cast<unsigned int>(tmp_size)] = '\0';
                string s(filename);
                commitptr->modify_list.push_back(make_pair(s.length(), s));
                delete [] filename;
            }
            
            for(int i = 0; i < nCopy; i++) {
                in.read(reinterpret_cast<char *>(&tmp_size), sizeof(tmp_size));
                char * filename1 = new char [static_cast<unsigned int>(tmp_size) + 1];
                in.read(filename1, sizeof(char) * static_cast<unsigned int>(tmp_size));
                filename1[static_cast<unsigned int>(tmp_size)] = '\0';
                string from(filename1);
                delete [] filename1;
                
                in.read(reinterpret_cast<char *>(&tmp_size), sizeof(tmp_size));
                char * filename2 = new char [static_cast<unsigned int>(tmp_size) + 1];
                in.read(filename2, sizeof(char) * static_cast<unsigned int>(tmp_size));
                filename2[static_cast<unsigned int>(tmp_size)] = '\0';
                string to(filename2);
                delete [] filename2;
                commitptr->copy_list.push_back(make_pair(make_pair(from.length(), from), make_pair(to.length(), to)));
                
            }
            
            for(int i = 0; i < nDel; i++) {
                in.read(reinterpret_cast<char *>(&tmp_size), sizeof(tmp_size));
                char * filename = new char [static_cast<unsigned int>(tmp_size) + 1];
                in.read(filename, sizeof(char) * static_cast<unsigned int>(tmp_size));
                filename[static_cast<unsigned int>(tmp_size)] = '\0';
                string s(filename);
                commitptr->del_list.push_back(make_pair(s.length(), s));
                delete [] filename;
            }
            
            /* get md5 */
            vector<string> old_file_name, old_md5;
            
            while(in.read(reinterpret_cast<char *>(&tmp_size), sizeof(tmp_size))) {
                int newsize = static_cast<int>(tmp_size);
                char * name = new char [newsize + 1];//
                name[newsize] = '\0';
                string md5_str;
                uint16_t md5_short;
                in.read(name, newsize);
                
                stringstream buffer;
                for(int i = 0; i < 8; i++) {
                    stringstream ss;
                    in.read(reinterpret_cast<char *>(&md5_short), sizeof(md5_short));
                    char * from = new char [4];
                    char * to = new char [5];
                    sprintf(from, "%04x", md5_short);
                    to[0] = from[2];
                    to[1] = from[3];
                    to[2] = from[0];
                    to[3] = from[1];
                    to[4] = '\0';
                    string s(to);
                    buffer << s;
                    delete [] from;
                    delete [] to;
                }
                buffer >> md5_str;
                buffer.clear();
                
                string name_str(name);
                
                old_file_name.push_back(name_str);
                old_md5.push_back(md5_str);
                delete [] name;
                if(in.tellg() == record_total_size) break;
            }
            
            record_total_size -= parse_size.back();
            in.seekg(-offset, ios::cur);
            parse_size.pop_back();
            print_log_commit(headerptr, commitptr, old_file_name, old_md5);
            if( i < (log_n < total_num_of_commit ? log_n - 1 : total_num_of_commit - 1))
                cout << endl;
            delete headerptr;
            delete commitptr;
        }
    }
}

void print_commit(commit_header * headerptr, commit_struct * commitptr) {
    cout << "[new_file]" << endl;
    for(int i = 0; i < commitptr->add_list.size(); i++)
        cout << commitptr->add_list[i].second << endl;
    
    cout << "[modified]" << endl;
    for(int i = 0; i < commitptr->modify_list.size(); i++)
        cout << commitptr->modify_list[i].second << endl;
    
    cout << "[copied]" << endl;
    for(int i = 0; i < commitptr->copy_list.size(); i++)
        cout << commitptr->copy_list[i].first.second << " => " << commitptr->copy_list[i].second.second << endl;
    
    cout << "[deleted]" << endl;
    for(int i = 0; i < commitptr->del_list.size(); i++)
        cout << commitptr->del_list[i].second << endl;
}

void write_commit(commit_header * headerptr, commit_struct * commitptr, char * folder_append) {
    ofstream out(folder_append, ofstream::out | ofstream::app | ofstream::binary);
    /* write header */
    out.write(reinterpret_cast<const char *>(&headerptr->number_of_commit), sizeof(headerptr->number_of_commit));
    out.write(reinterpret_cast<const char *>(&headerptr->number_of_file), sizeof(headerptr->number_of_file));
    out.write(reinterpret_cast<const char *>(&headerptr->number_of_add), sizeof(headerptr->number_of_add));
    out.write(reinterpret_cast<const char *>(&headerptr->number_of_modify), sizeof(headerptr->number_of_modify));
    out.write(reinterpret_cast<const char *>(&headerptr->number_of_copy), sizeof(headerptr->number_of_copy));
    out.write(reinterpret_cast<const char *>(&headerptr->number_of_del), sizeof(headerptr->number_of_del));
    out.write(reinterpret_cast<const char *>(&headerptr->commit_size), sizeof(headerptr->commit_size));
    
    /* add */
    for(int i = 0; i < commitptr->add_list.size(); i++) {
        out.write(reinterpret_cast<const char *>(&commitptr->add_list[i].first), sizeof(commitptr->add_list[i].first));
        out.write(commitptr->add_list[i].second.c_str(), commitptr->add_list[i].second.length());
    }
    
    /* modify */
    for(int i = 0; i < commitptr->modify_list.size(); i++) {
        out.write(reinterpret_cast<const char *>(&commitptr->modify_list[i].first), sizeof(commitptr->modify_list[i].first));
        out.write(commitptr->modify_list[i].second.c_str(), commitptr->modify_list[i].second.length());
    }
    
    /* copy */
    for(int i = 0; i < commitptr->copy_list.size(); i++) {
        out.write(reinterpret_cast<const char *>(&commitptr->copy_list[i].first.first), sizeof(commitptr->copy_list[i].first.first));
        out.write(commitptr->copy_list[i].first.second.c_str(), commitptr->copy_list[i].first.second.length());
        out.write(reinterpret_cast<const char *>(&commitptr->copy_list[i].second.first), sizeof(commitptr->copy_list[i].second.first));
        out.write(commitptr->copy_list[i].second.second.c_str(), commitptr->copy_list[i].second.second.length());
    }
    
    /* del */
    for(int i = 0; i < commitptr->del_list.size(); i++) {
        out.write(reinterpret_cast<const char *>(&commitptr->del_list[i].first), sizeof(commitptr->del_list[i].first));
        out.write(commitptr->del_list[i].second.c_str(), commitptr->del_list[i].second.length());
    }
    
    /* md5 */
    for(int i = 0; i < commitptr->file_list.size(); i++) {
        out.write(reinterpret_cast<const char *>(&commitptr->file_list[i].first), sizeof(commitptr->file_list[i].first));
        out.write(commitptr->file_list[i].second.c_str(), commitptr->file_list[i].second.length());
        /* out.write(commitptr->md5_list[i].c_str(), commitptr->md5_list[i].size()); it's wrong, because it will write md5 string in ASCII format. e.g. 1abc => write 31 61 62 63*/
        
        for(int j = 0; j < 32; j += 4) {
            char * from = new char[4];
            uint16_t to;
            from[0] = commitptr->md5_list[i][j+2];
            from[1] = commitptr->md5_list[i][j+3];
            from[2] = commitptr->md5_list[i][j];
            from[3] = commitptr->md5_list[i][j+1];
            stringstream ss;
            ss << from;
            ss >> hex >> to;
            out.write(reinterpret_cast<const char *>(&to), sizeof(to));
            delete [] from;
        }
    }
 
}

int main(int argc, const char * argv[]) {
    
    char path[2048];
    if(strcmp(argv[1], "log") == 0)
        strcpy(path, argv[3]);
    else
        strcpy(path, argv[2]);
    char folder_append[strlen(path) + strlen("/.loser_record") + 1];
    sprintf(folder_append, "%s/.loser_record", path);
    
    ifstream in(folder_append, ifstream::in | ifstream::binary);
    if(!in) {
        if(strcmp(argv[1], "commit") == 0) {
            fstream create(folder_append, fstream::in | fstream::out | fstream::trunc);
            create.close();
            in.open(folder_append, ifstream::in | ifstream::binary);
        }
    }
    
    commit_header * headptr = new commit_header;
    commit_struct * commitptr = new commit_struct;
    
    vector<unsigned int> parse_size; //for log command
    
    vector<string> file_name;
    get_file_list(path, file_name);
    
    vector<string> new_md5;
    for(int i = 0; i < file_name.size(); i++) {
        char path_append[2048];
        sprintf(path_append, "%s/%s", path, file_name[i].c_str());
        new_md5.push_back(get_md5(path_append));
    }
    
    /* specify .loser_record is empty or not */
    struct stat results;
    stat(folder_append, &results);
    
    if(strcmp(argv[1], "commit") == 0 || strcmp(argv[1], "status") == 0 || strcmp(argv[1], "log") == 0) {
        if(results.st_size == 0 || !in) {
            /* ".loser_record size = 0(i.e. file just create)" or "status without .loser_record existing" */
            
            /* commit struct */
            for(int i = 0; i < file_name.size(); i++)
                commitptr->add_list.push_back(make_pair(file_name[i].length(), file_name[i]));
                                              
            commitptr->md5_list = new_md5;
            for(int i = 0; i < file_name.size(); i++) {
                commitptr->file_list.push_back(make_pair(file_name[i].length(), file_name[i]));
            }
            
            /* commit header */
            headptr->number_of_commit = 1;
            headptr->number_of_file = (unsigned int)file_name.size();
            headptr->number_of_add = (unsigned int)commitptr->add_list.size();
            headptr->number_of_modify = (unsigned int)commitptr->modify_list.size();
            headptr->number_of_copy = (unsigned int)commitptr->copy_list.size();
            headptr->number_of_del = (unsigned int)commitptr->del_list.size();
            headptr->commit_size = caculate_commit_size(headptr, commitptr);
        }
        else {
            unsigned int nCommit = 0;
            unsigned int record_total_size = 0;
            int offset = 0;
            in.seekg(0, ios::end);
            record_total_size = in.tellg();
            in.seekg(0, ios::beg);
            
            while(true) {
                nCommit += 1;
                in.seekg(24, ios::cur); // move to end of "number of del"
                
                unsigned int size;
                if(in.read(reinterpret_cast<char *>(&size), sizeof(size))) {
                    parse_size.push_back(size);
                    in.seekg(size - 28, ios::cur);
                }

                if(in.tellg() == record_total_size)
                    break;
            }
            
            headptr->number_of_commit = nCommit + 1;
            offset = -parse_size.back();
            in.seekg(offset, ios::end);
            unsigned int nAdd, nModify, nCopy, nDel;
            in.seekg(8, ios::cur);
            in.read(reinterpret_cast<char *>(&nAdd), sizeof(nAdd));
            in.read(reinterpret_cast<char *>(&nModify), sizeof(nModify));
            in.read(reinterpret_cast<char *>(&nCopy), sizeof(nCopy));
            in.read(reinterpret_cast<char *>(&nDel), sizeof(nDel));
            in.seekg(4, ios::cur); // skip commit size block
            
            uint8_t tmp_size;
            for(int i = 0; i < nAdd; i++) {
                in.read(reinterpret_cast<char *>(&tmp_size), sizeof(tmp_size));
                in.seekg(tmp_size, ios::cur);
            }
            
            for(int i = 0; i < nModify; i++) {
                in.read(reinterpret_cast<char *>(&tmp_size), sizeof(tmp_size));
                in.seekg(tmp_size, ios::cur);
            }
            
            for(int i = 0; i < nCopy; i++) {
                in.read(reinterpret_cast<char *>(&tmp_size), sizeof(tmp_size));
                in.seekg(tmp_size, ios::cur);
                in.read(reinterpret_cast<char *>(&tmp_size), sizeof(tmp_size));
                in.seekg(tmp_size, ios::cur);
            }
            
            for(int i = 0; i < nDel; i++) {
                in.read(reinterpret_cast<char *>(&tmp_size), sizeof(tmp_size));
                in.seekg(tmp_size, ios::cur);
            }
            
            vector<string> old_file_name, old_md5;
            
            while(in.read(reinterpret_cast<char *>(&tmp_size), sizeof(tmp_size))) {
                unsigned int newsize = static_cast<unsigned int>(tmp_size);
                char * name = new char [newsize + 1];
                name[newsize] = '\0';
                in.read(name, newsize);
                string name_str(name);
                
                string md5_str;
                uint16_t md5_short;
                stringstream buffer;
                for(int i = 0; i < 8; i++) {
                    stringstream ss;
                    in.read(reinterpret_cast<char *>(&md5_short), sizeof(md5_short));
                    char * from = new char [4];
                    char * to = new char [5];
                    sprintf(from, "%04x", md5_short);
                    to[0] = from[2];
                    to[1] = from[3];
                    to[2] = from[0];
                    to[3] = from[1];
                    to[4] = '\0';
                    string s(to);
                    buffer << s;
                    delete [] from;
                    delete [] to;
                }
                buffer >> md5_str;
                buffer.clear();
                
                old_file_name.push_back(name_str);
                old_md5.push_back(md5_str);
                delete [] name;
                if(in.tellg() == record_total_size) break;
            }
            
            for(int i = 0; i < file_name.size(); i++) {
                bool exist_in_old_file_list = false;
                for(int j = 0; j < old_file_name.size(); j++) {
                    if(file_name[i] == old_file_name[j]) {
                        exist_in_old_file_list = true;
                        if(new_md5[i] != old_md5[j])
                            commitptr->modify_list.push_back(make_pair(file_name[i].length(), file_name[i]));
                        break;
                    }
                }
                if(!exist_in_old_file_list) {
                    bool exist_in_old_md5 = false;
                    for(int k = 0; k < old_md5.size(); k++) {
                        if(new_md5[i] == old_md5[k]) {
                            commitptr->copy_list.push_back(make_pair(make_pair(old_file_name[k].length(), old_file_name[k]), make_pair(file_name[i].length(), file_name[i])));
                            exist_in_old_md5 = true;
                            break;
                        }
                    }
                    if(!exist_in_old_md5) {
                        commitptr->add_list.push_back(make_pair(file_name[i].length(), file_name[i]));
                    }
                }
            }
            
            for(int i = 0; i < old_file_name.size(); i++) {
                bool exist_in_new_file_list = false;
                for(int j = 0; j < file_name.size(); j++) {
                    if(old_file_name[i] == file_name[j]) {
                        exist_in_new_file_list = true;
                        break;
                    }
                    
                }
                if(!exist_in_new_file_list)
                    commitptr->del_list.push_back(make_pair(old_file_name[i].length(), old_file_name[i]));
            }
            
            commitptr->md5_list = new_md5;
            for(int i = 0; i < file_name.size(); i++) {
                commitptr->file_list.push_back(make_pair(file_name[i].length(), file_name[i]));
            }
            /* commit header */
            headptr->number_of_file = (unsigned int)file_name.size();
            headptr->number_of_add = (unsigned int)commitptr->add_list.size();
            headptr->number_of_modify = (unsigned int)commitptr->modify_list.size();
            headptr->number_of_copy = (unsigned int)commitptr->copy_list.size();
            headptr->number_of_del = (unsigned int)commitptr->del_list.size();
            headptr->commit_size = caculate_commit_size(headptr, commitptr);
        }//end else (not a new record)
        
        /* last step of commit : write to .loser_record */
        if(strcmp(argv[1], "commit") == 0)
            write_commit(headptr, commitptr, folder_append);
        else if(strcmp(argv[1], "log") == 0) {
            stringstream ss;
            unsigned int n_log;
            ss << argv[2];
            ss >> n_log;
            print_log(in, parse_size, n_log);
        }
        else
            print_commit(headptr, commitptr);
            
    }//end if is commit or status or log
    
    return 0;
}
