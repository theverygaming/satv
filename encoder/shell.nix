with import <nixpkgs> { };
let
  gccForLibs = stdenv.cc.cc;
in
stdenv.mkDerivation {
  name = "satv";
  buildInputs = [
    gnumake
  ];
}
