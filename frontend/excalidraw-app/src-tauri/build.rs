fn main() {
  let dst = cmake::Config::new("cpp")
      .build_target("collab_lib")
      .build();

  println!("cargo:rustc-link-search=native={}/build", dst.display());
  println!("cargo:rustc-link-lib=static=collab_lib");

  // Cargo가 무시할 수 없는, 링커에게 직접 전달하는 인자입니다.
  // `-l`은 "라이브러리를 링크하라"는 의미의 링커 플래그입니다.
  println!("cargo:rustc-link-arg=-lc++");

  tauri_build::build();
}
