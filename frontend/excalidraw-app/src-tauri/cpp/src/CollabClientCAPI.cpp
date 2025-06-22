#include "CollabClientCAPI.h"
#include "CollabClient.h"
#include <string>

extern "C" {

void* collabclient_new() {
    return new CollabClient();
}

void collabclient_delete(void* client) {
    delete static_cast<CollabClient*>(client);
}

bool collabclient_connect(void* client, const char* host, int port) {
    if (!client) return false;
    return static_cast<CollabClient*>(client)->connect(std::string(host), port);
}

void collabclient_disconnect(void* client) {
    if (!client) return;
    static_cast<CollabClient*>(client)->disconnect();
}

bool collabclient_emit(void* client, const char* json_payload) {
    if (!client) return false;
    return static_cast<CollabClient*>(client)->emit(json_payload);
}

void collabclient_set_event_callback(void* client, collab_event_callback_t cb, void* user_data) {
    if (!client) return;
    static_cast<CollabClient*>(client)->set_event_callback(cb, user_data);
}

} // extern "C" 