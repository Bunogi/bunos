{
  pkgs ? import <nixpkgs> {  }
}:

pkgs.mkShell.override { stdenv = pkgs.gcc10Stdenv; } {
  # Not all of these are needed to build the project but are nice to have.
  # Maybe i should add some of these to my system permanently instead...
  nativeBuildInputs = with pkgs; [ qemu cmake ninja gdb mpfr gmp libmpc grub2_full xorriso curl bochs valgrind];
  hardeningDisable = [ "format" ];
}
