//
// Created by 박성빈 on 25. 6. 5.
//

#ifndef DOTENV_H
#define DOTENV_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>


/**
* @param path .env라는 파일이 있는 디렉터리 또는 env 파일 자체의 경로일 수 있음.
* @param overwrite 기존 변수 덮어쓰기 여부.
* @return 성공 시 0, 파일을 열 수 없으면 -1
*/
int env_load(const char* path, bool overwrite);


#ifdef __cplusplus
}
#endif

#endif //DOTENV_H
