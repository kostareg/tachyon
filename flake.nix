{
  description = "Development environment with clang and libc++";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
  }:
    flake-utils.lib.eachDefaultSystem (system: let
      pkgs = nixpkgs.legacyPackages.${system};
    in {
      devShells.default = pkgs.mkShell {
        hardeningDisable = ["all"];

        buildInputs = with pkgs; [
          llvmPackages.clangUseLLVM
          libcxx
          llvm
          lld
          cmake
          ninja
          pkg-config
          llvmPackages.libunwind
          gcovr
          lcov
          jq
          gperf
          linuxKernel.packages.linux_6_6.perf
        ];

        shellHook = ''
          export CXX=clang++
          export CC=clang
          export CXXFLAGS="-stdlib=libc++"
          export LDFLAGS="-stdlib=libc++"
          export LD=lld
        '';
      };
    });
}
