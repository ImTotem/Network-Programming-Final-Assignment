use std::os::raw::{c_char, c_int, c_void};

pub type CollabEventCallback = extern "C" fn(event: *const c_char, args: *const *const c_char, argc: c_int, user_data: *mut c_void);

#[link(name = "collab_lib")] // CMake 빌드 시 생성되는 라이브러리 이름
extern "C" {
    pub fn collabclient_new() -> *mut c_void;
    pub fn collabclient_delete(client: *mut c_void);
    pub fn collabclient_connect(client: *mut c_void, host: *const c_char, port: c_int) -> bool;
    pub fn collabclient_disconnect(client: *mut c_void);
    pub fn collabclient_emit(client: *mut c_void, event: *const c_char, args: *const *const c_char, argc: c_int) -> bool;
    pub fn collabclient_set_event_callback(client: *mut c_void, cb: CollabEventCallback, user_data: *mut c_void);
} 