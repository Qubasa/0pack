#!/usr/bin/env bash

# set -e
#set -x

pushd . || exit
cd ..
rm -f bin/0pack
./build.sh

popd || exit

echo ""
echo "======== TESTING 0PACK ========"
echo ""

echo "[*] Deleting old packed binary"
rm ./result/*

echo "[*] Packing Pie test"
../bin/0pack -i "$PWD/bins/pie_test.elf" -o "$PWD/result/pie_packed.elf" -p fork-payload.fasm -d
"$PWD/result/pie_packed.elf"

# echo "[*] Packing NTP-Client"
# ../bin/0pack -i "$PWD/bins/ntpclient" -o "$PWD/result/ntpclient_packed.elf" -p fork-payload.fasm -d
# "$PWD/result/ntpclient_packed.elf"





# echo "[*] Packing ntp client"
# ../bin/main.elf  "$PWD/bins/ntpclient" "$PWD/hello_world.fasm" "$PWD/result/ntpclient_packed.elf"
# ./result/ntpclient_packed.elf stratum2-4.NTP.TechFak.Uni-Bielefeld.DE de.pool.ntp.org time1.uni-paderborn.de
