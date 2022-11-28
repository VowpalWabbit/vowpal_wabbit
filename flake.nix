{
  # Currently, this flake only contains tools helpful for development.
  # Over time, we may add a package definition and other things.
  description = "Development utils for Vowpal Wabbit.";

  inputs.flake-utils.url = "github:numtide/flake-utils";

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let pkgs = nixpkgs.legacyPackages.${system}; in
      let
        core-dependencies = [
          pkgs.spdlog
          pkgs.fmt
          pkgs.zlib
          pkgs.rapidjson
          pkgs.eigen
          pkgs.gtest
          pkgs.boost
        ];
      in
      let
        python-clang-tidy-package = pkgs.stdenv.mkDerivation {
          name = "python-clang-tidy";
          src = pkgs.fetchurl {
            url = "https://github.com/llvm/llvm-project/releases/download/llvmorg-14.0.6/clang-tools-extra-14.0.6.src.tar.xz";
            sha256 = "sha256-fPO4/1bGXE0erjxWiD/EpsvD/586FTCnTWbkXScnGGY=";
          };
          sourceRoot = "clang-tools-extra-14.0.6.src";
          phases = [ "unpackPhase" "installPhase" "fixupPhase" ];
          propagatedBuildInputs = [ pkgs.python3 pkgs.clang-tools_14 ];
          installPhase = ''
            mkdir -p $out/bin
            cp clang-tidy/tool/run-clang-tidy.py $out/bin
            cp clang-tidy/tool/run-clang-tidy.py $out/bin/run-clang-tidy
            cp clang-tidy/tool/clang-tidy-diff.py $out/bin
            cp clang-tidy/tool/clang-tidy-diff.py $out/bin/clang-tidy-diff
          '';
        };
      in
      let
        python-clang-format-package = pkgs.stdenv.mkDerivation {
          name = "python-clang-format";
          src = pkgs.fetchurl {
            url = "https://github.com/llvm/llvm-project/releases/download/llvmorg-14.0.0/clang-14.0.0.src.tar.xz";
            sha256 = "sha256-9df/uG7Vf5fXxHHVQsTlaF20t1+4F8TD8Ce/pJ5WG5s=";
          };
          sourceRoot = "clang-14.0.0.src";
          phases = [ "unpackPhase" "installPhase" "fixupPhase" ];
          propagatedBuildInputs = [ pkgs.python3 pkgs.clang-tools_14 ];
          installPhase = ''
            mkdir -p $out/bin
            cp tools/clang-format/clang-format-diff.py $out/bin
            cp tools/clang-format/clang-format-diff.py $out/bin/clang-format-diff
          '';
        };
      in
      let
        compile-commands = pkgs.stdenv.mkDerivation {
          src = self;
          buildInputs = [ ] ++ core-dependencies;
          nativeBuildInputs = [ pkgs.cmake ];
          name = "compile-commands";
          buildCommand = ''
            set -e
            mkdir -p $out
            cmake -S "${self}" -B "$out" \
              -DCMAKE_EXPORT_COMPILE_COMMANDS=On \
              -DRAPIDJSON_SYS_DEP=On \
              -DFMT_SYS_DEP=On \
              -DSPDLOG_SYS_DEP=On \
              -DVW_BOOST_MATH_SYS_DEP=On \
              -DVW_ZLIB_SYS_DEP=On \
              -DVW_GTEST_SYS_DEP=On \
              -DVW_EIGEN_SYS_DEP=On
          '';

        };
      in
      let
        clang-tidy-all-script = pkgs.writeShellScriptBin "vw-clang-tidy-all" ''
          ${python-clang-tidy-package}/bin/run-clang-tidy -p ${compile-commands} -quiet -header-filter=vw/*
        '';
      in
      let
        clang-tidy-diff-script = pkgs.writeShellScriptBin "vw-clang-tidy-diff" ''
          ${python-clang-tidy-package}/bin/clang-tidy-diff -p 0 -path ${compile-commands} -quiet -use-color "$@"
        '';
      in
      {
        formatter = pkgs.nixpkgs-fmt;
        devShell = pkgs.mkShell {
          packages = [
            python-clang-tidy-package
            python-clang-format-package
            clang-tidy-all-script
            clang-tidy-diff-script
          ] ++ core-dependencies;
        };
      }
    );
}
