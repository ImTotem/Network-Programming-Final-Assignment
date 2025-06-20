#pragma once
#ifndef COLLAB_CLIENT_C_API_H
#define COLLAB_CLIENT_C_API_H

#ifdef __cplusplus
extern "C" {
#endif

// 콜백 타입 정의
// event는 항상 "payload"로 고정, args는 반드시 {"event":..., "data":...} JSON string 하나만 전달됨
// 콜백 구현 시 args[0]만 사용해야 함
typedef void (*collab_event_callback_t)(const char* event, const char** args, int argc, void* user_data);

void* collabclient_new();
void collabclient_delete(void* client);
bool collabclient_connect(void* client, const char* host, int port);
void collabclient_disconnect(void* client);
// event는 항상 "payload"로 고정, args는 반드시 {"event":..., "data":...} JSON string 하나만 전달됨
bool collabclient_emit(void* client, const char* event, const char** args, int argc);
// Rust FFI에서 콜백 등록
void collabclient_set_event_callback(void* client, collab_event_callback_t cb, void* user_data);

#ifdef __cplusplus
}
#endif
#endif