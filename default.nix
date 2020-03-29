with import <nixpkgs> {};
stdenv.mkDerivation {
  name = "cli-fm";
  src = ./.;
  nativeBuildInputs = [ pkgs.cmake ];
}
