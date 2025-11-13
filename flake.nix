{
  description = "comp 222 project 2";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }: 
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };

        proj1 = pkgs.stdenv.mkDerivation {
          pname = "proj2";
          version = "0.0.1";
          src = ./.;

          nativeBuildInputs = [
            pkgs.pkg-config
            pkgs.meson
            pkgs.ninja
          ];

          buildInputs = [
          ];


        };
      in {
        packages.default = proj1;

        devShell = pkgs.mkShell {
          nativeBuildInputs = [
            pkgs.pkg-config
            pkgs.meson
            pkgs.ninja
            pkgs.clang-tools
          ];
          buildInputs = [
          ];
        };
      });
}

