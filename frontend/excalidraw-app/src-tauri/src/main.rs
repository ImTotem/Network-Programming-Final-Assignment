// Prevents additional console window on Windows in release, DO NOT REMOVE!!
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

mod ffi;
mod collab_client;


#[tauri::command]
fn collab_connect(host: String, port: u16) -> bool {
    let client = collab_client::get_client();
    let c_host = std::ffi::CString::new(host).unwrap();
    unsafe { ffi::collabclient_connect(client, c_host.as_ptr(), port as i32) }
}

#[tauri::command]
fn collab_emit(event: String, args: Vec<String>) -> bool {
    let client = collab_client::get_client();
    let c_event = std::ffi::CString::new(event).unwrap();
    let c_args: Vec<std::ffi::CString> = args.iter().map(|s| std::ffi::CString::new(s.as_str()).unwrap()).collect();
    let c_ptrs: Vec<*const i8> = c_args.iter().map(|s| s.as_ptr()).collect();
    unsafe { ffi::collabclient_emit(client, c_event.as_ptr(), c_ptrs.as_ptr(), c_ptrs.len() as i32) }
}

#[tauri::command]
fn collab_disconnect() {
    let client = collab_client::get_client();
    unsafe { ffi::collabclient_disconnect(client) }
}

fn main() {
    tauri::Builder::default()
        .setup(|app| {
            collab_client::set_app_handle(app.handle().clone());
            collab_client::set_event_callback();
            Ok(())
        })
        .invoke_handler(tauri::generate_handler![
            collab_connect,
            collab_emit,
            collab_disconnect
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
