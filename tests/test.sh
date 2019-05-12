#!/usr/bin/env bash

# set -e

pushd . || exit
cd ..
rm -f bin/0pack
./build.sh

popd || exit

echo ""
echo "======== TESTING 0PACK ========"
echo ""

echo "[*] Packing pie_test.elf"
../bin/0pack -i "$PWD/bins/pie_test.elf" -p hello_world.fasm -o "$PWD/result/pie_test_packed.elf"
# echo "[*] Executing pie_test.elf"
./result/pie_test_packed.elf

echo "[*] Deleting old packed binary"
rm -f ./result/pie_packed.elf

# echo "[*] Packing crackme.elf"
# ../bin/main.elf "$PWD/bins/pie_test.elf" "$PWD/hello_world.fasm" "$PWD/result/pie_packed.elf"
# ./result/pie_packed.elf

echo "[*] Packing NTP-Client"
../bin/0pack -i bins/ntpclient -o result/ntpclient_packed.elf -p hello_world.fasm -d
result/ntpclient_packed.elf

# echo "[*] Packing ntp client"
# ../bin/main.elf  "$PWD/bins/ntpclient" "$PWD/hello_world.fasm" "$PWD/result/ntpclient_packed.elf"
# ./result/ntpclient_packed.elf stratum2-4.NTP.TechFak.Uni-Bielefeld.DE de.pool.ntp.org time1.uni-paderborn.de
