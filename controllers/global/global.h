#ifndef GLOBAL_H
#define GLOBAL_H

//Reference: https://blog.csdn.net/szm1234/article/details/120810079
int GetIniKeyString(const char* title, const char* key, const char* filename, char* buf);

int PutIniKeyString(const char* title, const char* key, const char* val, const char* filename);

#endif