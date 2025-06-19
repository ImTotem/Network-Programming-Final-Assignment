use std::sync::Mutex;
use once_cell::sync::{Lazy, OnceCell};
use std::os::raw::{c_char, c_int, c_void};
use std::ffi::{CStr};
use crate::ffi;
use tauri::{AppHandle, Emitter};

#[derive(Copy, Clone)]
pub struct ClientPtr(pub *mut c_void);

unsafe impl Sync for ClientPtr {}
unsafe impl Send for ClientPtr {}

static CLIENT: Lazy<Mutex<Option<ClientPtr>>> = Lazy::new(|| Mutex::new(None));
static APP_HANDLE: OnceCell<AppHandle> = OnceCell::new();

pub fn set_app_handle(handle: AppHandle) {
    let _ = APP_HANDLE.set(handle);
}

pub fn get_app_handle() -> &'static AppHandle {
    APP_HANDLE.get().expect("AppHandle not set")
}

pub fn get_client() -> *mut c_void {
    let mut guard = CLIENT.lock().unwrap();
    if guard.is_none() {
        unsafe {
            *guard = Some(ClientPtr(ffi::collabclient_new()));
        }
    }
    guard.unwrap().0
}

// Rust 콜백 → Tauri 이벤트 emit
pub extern "C" fn rust_collab_event_callback(event: *const c_char, args: *const *const c_char, argc: c_int, _user_data: *mut c_void) {
    let event_name = unsafe { CStr::from_ptr(event).to_string_lossy().to_string() };
    let mut arg_vec = Vec::new();
    for i in 0..argc {
        let arg_ptr = unsafe { *args.add(i as usize) };
        let arg = unsafe { CStr::from_ptr(arg_ptr).to_string_lossy().to_string() };
        arg_vec.push(arg);
    }
    // Tauri 이벤트 emit (전역 app 핸들러 사용)
    tauri::async_runtime::spawn(async move {
        get_app_handle().emit("collab_event", (event_name, arg_vec)).ok();
    });
}

pub fn set_event_callback() {
    let client = get_client();
    unsafe {
        ffi::collabclient_set_event_callback(client, rust_collab_event_callback, std::ptr::null_mut());
    }
}