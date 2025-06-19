fn main() {
  // C++ 라이브러리 빌드 (cpp/CMakeLists.txt 기준)
  let dst = cmake::Config::new("cpp")
      .build_target("collab_lib")
      .build();

  println!("cargo:rustc-link-search=native={}/build", dst.display());
  println!("cargo:rustc-link-lib=dylib=collab_lib");
  tauri_build::build()
}
