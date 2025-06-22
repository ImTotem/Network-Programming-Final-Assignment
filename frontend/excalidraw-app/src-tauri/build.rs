fn main() {
  let dst = cmake::Config::new("cpp")
      .build_target("collab_lib")
      .build();

  println!("cargo:rustc-link-search=native={}/build", dst.display());
  println!("cargo:rustc-link-lib=static=collab_lib");

  if cfg!(target_os = "macos") {
    println!("cargo:rustc-link-arg=-lc++");
  }

  tauri_build::build();
}
