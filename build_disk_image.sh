#!/usr/bin/env bash
set -euo pipefail

typeset -r output=$1
typeset -r input_dir=$2
typeset -r staging_dir=$3
typeset -r mount_path=/tmp/bunos_disk

cp -ar $input_dir/* $staging_dir

[ ! -f "$output" ] || rm -f $output
fallocate -l 10MiB $output
mkfs.ext2 $output

sudo mkdir -p $mount_path
sudo mount $output $mount_path
sudo cp -ar $staging_dir/* $mount_path

sudo rm -rf $mount_path/lost\+found
sudo umount $mount_path
sudo rm -rf $mount_path
