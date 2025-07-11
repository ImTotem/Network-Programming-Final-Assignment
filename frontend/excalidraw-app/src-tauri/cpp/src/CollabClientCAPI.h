#pragma once
#ifndef COLLAB_CLIENT_C_API_H
#define COLLAB_CLIENT_C_API_H

#ifdef __cplusplus
extern "C" {
#endif

// 콜백 타입 정의 - JSON 문자열 하나만 받음
typedef void (*collab_event_callback_t)(const char* json_payload, void* user_data);

void* collabclient_new();
void collabclient_delete(void* client);
bool collabclient_connect(void* client, const char* host, int port);
void collabclient_disconnect(void* client);
bool collabclient_emit(void* client, const char* json_payload);
// Rust FFI에서 콜백 등록
void collabclient_set_event_callback(void* client, collab_event_callback_t cb, void* user_data);

#ifdef __cplusplus
}
#endif
#endif