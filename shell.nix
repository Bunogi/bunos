{
  pkgs ? import <nixpkgs> {  },
  crossPkgs ? import <nixpkgs> {
    crossSystem = {
      config = "i686-elf";
    };
  }
}:

pkgs.mkShell.override { stdenv = crossPkgs.gcc10Stdenv; } {
  nativeBuildInputs = with pkgs; [ qemu cmake ninja gdb ];
}
