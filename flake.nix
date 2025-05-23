{
  description = "C++ project with spdlog";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in {
        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            clang
            cmake
            spdlog spdlog.dev
            fmt fmt.dev
            mimalloc mimalloc.dev
          ];
          shellHook = ''
            export CC=clang
            export CXX=clang++
          '';
        };
      });
}