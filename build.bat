@echo off
cd %~dp0
mkdir build

for %%j in (0) do (
for %%i in (0 1 2 3 4 5 6 7 8 9 A B C D E F) do (
    echo #pragma once > myuuid.h
    echo #define MYUUID "ffa07888-75bd-471a-b325-59274e7340%%j%%i" >> myuuid.h
    "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\amd64\MSBuild.exe" ExecuteCommand.sln /p:Configuration=Release /p:Platform=x64
    copy .\x64\Release\ExecuteCommand.exe .\build\ExecuteCommand40%%j%%i.exe
)
)