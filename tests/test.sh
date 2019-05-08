#!/usr/bin/env bash

# set -e

pushd . || exit
cd ..
rm -f bin/main.elf
./build.sh

popd || exit

echo ""
echo "======== TESTING 0PACK ========"
echo ""

# echo "[*] Packing pie_test.elf"
# ../bin/main.elf "$PWD/bins/pie_test.elf" "$PWD/result/pie_test_packed.elf"
# echo "[*] Executing pie_test.elf"
# ./result/pie_test_packed.elf

echo "[*] Deleting old packed binary"
rm -f ./result/pie_packed.elf

echo "[*] Packing crackme.elf"
../bin/main.elf "$PWD/bins/pie_test.elf" "$PWD/hello_world.fasm" "$PWD/result/pie_packed.elf"
./result/pie_packed.elf

# echo "[*] Packing metasploit.elf"
# ../bin/main.elf "$PWD/bins/metasploit.elf" "$PWD/result/metasploit_packed.elf"

