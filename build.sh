#!/bin/sh
cd $(dirname 0)
mkdir build
for i in $(seq 0 15);
do
    hex_ind=$(printf "%02X\n" $i)
    printf "#pragma once\n#define MYUUID \"ffa07888-75bd-471a-b325-59274e7340$hex_ind\"" > myuuid.h
    "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\amd64\MSBuild.exe" ExecuteCommand.sln /p:Configuration=Release /p:Platform=x64
    cp ./x64/Release/ExecuteCommand.exe ./build/ExecuteCommand40$hex_ind.exe
done
