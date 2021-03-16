# For more info about how to use see:
# https://github.com/VowpalWabbit/vowpal_wabbit/wiki/Maintainer-Info#nix

with import <nixpkgs> { };
{
  mystdenv ? pkgs.stdenv,
  myboost ? pkgs.boost174
 }:
mystdenv.mkDerivation {
  name = "vw-build-nix-shell";
  buildInputs = [
    cmake
    myboost
    zlib
    ninja
    flatbuffers
  ];
}
