#include "CollabClientCAPI.h"
#include "CollabClient.h"
#include <cstring>
#include <vector>
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

bool collabclient_emit(void* client, const char* event, const char** args, int argc) {
    if (!client || !event) return false;
    std::vector<std::any> argvec;
    for (int i = 0; i < argc; ++i) {
        argvec.emplace_back(std::string(args[i]));
    }
    return static_cast<CollabClient*>(client)->emit(std::string(event), argvec);
}

void collabclient_set_event_callback(void* client, collab_event_callback_t cb, void* user_data) {
    if (!client) return;
    static_cast<CollabClient*>(client)->set_event_callback(cb, user_data);
}

} // extern "C" 