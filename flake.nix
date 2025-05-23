{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-parts.url = "github:hercules-ci/flake-parts";
  };

  outputs = inputs @ {
    nixpkgs,
    flake-parts,
    ...
  }:
    flake-parts.lib.mkFlake {inherit inputs;} {
      systems = ["x86_64-linux"];

      perSystem = {
        pkgs,
        system,
        ...
      }: let
        llvm = pkgs.llvmPackages_19;
        stdenv = llvm.stdenv;
        libcxx = llvm.libcxx;
      in {
        devShells.default = pkgs.mkShell.override {inherit stdenv;} {
          packages = [
            pkgs.cmake
            pkgs.ninja
            llvm.libcxx
            llvm.libcxx.dev
            llvm.libcxxClang
            llvm.bintools
            llvm.compiler-rt
            llvm.libunwind
            pkgs.glibc
            pkgs.glibc.dev
          ];

          CXXFLAGS = "-nostdinc++ -isystem ${libcxx.dev}/include/c++/v1 -isystem ${pkgs.glibc.dev}/include";
          LDFLAGS = "-L${libcxx.out}/lib -lc++ -lc++abi -L${llvm.compiler-rt}/lib -lunwind -rtlib=compiler-rt -unwindlib=libunwind -Wl,--unresolved-symbols=ignore-in-object-files";
        };
      };
    };
}
