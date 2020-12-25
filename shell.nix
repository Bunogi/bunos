{
  pkgs ? import <nixpkgs> {  }
}:

pkgs.mkShell.override { stdenv = pkgs.gcc10Stdenv; } {
  nativeBuildInputs = with pkgs; [ qemu cmake ninja gdb mpfr gmp libmpc grub2_full xorriso curl ];
  hardeningDisable = [ "format" ];
}
