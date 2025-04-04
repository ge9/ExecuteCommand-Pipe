# ExecuteCommand-Pipe
A simple tool to run arbitrary commands with paths (of files or directories) passed from various context menus in Windows Explorer. Paths can be passed to the commands either as arguments or through stdin/stdout (pipe).
Implemented with COM (Component Object Model) technology to avoid limitations for path length or number of files.

# Usage

## (If you use Windows 11 and don't want to press Shift or click "Show More Options" everytime) Revert to old context menu

Run this command in cmd.exe or Win+R dialog and restart explorer.exe.

```
reg add "HKCU\Software\Classes\CLSID\{86ca1aa0-34aa-4e8b-a509-50c905bae2a2}\InprocServer32 /f /ve"
```

## Register as a class

Simply double-click to run `ExecuteCommandXXXX.exe` with no arguments, and it properly registers itself in the registry.
- Each released executable `ExecuteCommandXXXX.exe` has CLSID `{FFA07888-75BD-471A-B325-59274E73XXXX}`.
  - Since the CLSID must be determined in compile time and we have no method to pass arguments from outside the CLSID, we need a separate executable file for each command. 
- When run without admin right, it registers itself in `HKCU\Software\Classes\CLSID\{FFA07888-75BD-471A-B325-59274E73XXXX}`.
  - The full path of the executable is stored in the default value of `HKCU\Software\Classes\CLSID\{FFA07888-75BD-471A-B325-59274E73XXXX}\LocalServer32`.
- When run with admin right, it asks if it should use `HKLM` instead of `HKCU`.
  - But user-specific `HKCU` will be safer than system-wide `HKLM`.
  - If you use `HKLM`, you should not place the executable file inside `C:\Users\username`.
- You can also register it manually.
- Note that `HKCR` is the result of merging `HKCU\Software\Classes` and `HKLM\Software\Classes` together.
  - You can edit things in `HKCR`, but you should be aware which of `HKCU` or `HKLM` they come from.
- If you move the executable file, you should update the path in registry.
## Modify the argument
Once the CLSID is registered, you can append any argument to the executable path in `[HKCU or HKLM]\Software\Classes\CLSID\{FFA07888-75BD-471A-B325-59274E73XXXX}\LocalServer32`, according to your purpose (see examples below).

After the modification, rename the "LocalServer32" to any other name and then return it back (this seems the easiest way to reset some cache and apply the change).
### Available Options for ExecuteCommandXXXX.exe
- beginning with `d` or `-` ... for debugging; show the first and last files given and exit (execution through LocalServer32 automatically add "-Embedding" argument, so appending no argument also result in this debug mode).
- `a xxxx commandline` ... <u>A</u>rgument mode (newly added). Runs the given `commandline`, replacing "xxxx" with the list of quoted paths. `xxxx` can be any string that don't include space. `xxxx` can appear any times in `commandline`. Note that Windows has 32767 character command line length limit. if `xxxx` begins with the character `h`, the console window will be hidden.
- `p commandline` (or other characters) ... <u>P</u>ipe mode. Runs the given command, passing paths through pipe.  "\n" is appended to each path (including the last one).
- `h commandline` ... <u>H</u>idden mode. Similar to `p`, but the console window will be hidden.
### Examples
- For a single directory
  - Open Git Bash
    - `C:\path\to\ExecuteCommand4000.exe h cmd /c ""C:\Program Files\Git\usr\bin\cygpath" -f - | "C:\Program Files\Git\usr\bin\xargs" -d '\n' -I {} "C:\Program Files\Git\git-bash.exe" -c "cd \"{}\";exec bash""`
  - Open Git Bash in Windows' default terminal
    - `C:\path\to\ExecuteCommand4000.exe h cmd /c ""C:\Program Files\Git\usr\bin\cygpath" -f - | "C:\Program Files\Git\usr\bin\xargs.exe" -d '\n' -I {} cmd /c start "" "C:\Program Files\Git\usr\bin\env.exe" MSYSTEM=MINGW64 "C:\Program Files\Git\usr\bin\bash.exe" --login -i -c "cd \"{}\";exec bash""`
    - You need `//` in order for Git Bash's executables to pass `/` to Windows executables. You won't need it if you use Cygwin's xargs instead.
  - Open Cygwin bash
    - `C:\path\to\ExecuteCommand4000.exe h cmd /c ""C:\cygwin64\bin\cygpath" -f - | "C:\Program Files\Git\usr\bin\xargs" -d '\n' -I {} "C:\cygwin64\bin\mintty.exe" -e "C:\cygwin64\bin\bash.exe" --login -i -c "cd \"{}\";exec bash""`
    - Here you can use cygwin's xargs instead.
  - Open Cygwin Bash in Windows' default terminal
    - `C:\path\to\ExecuteCommand4000.exe h cmd /c ""C:\cygwin64\bin\cygpath" -f - | "C:\cygwin64\bin\xargs.exe" -d '\n' -I {} cmd /c start "" C:\cygwin64\bin\bash.exe --login -i -c "cd \"{}\";exec bash""`
  - Open VS Code
    - `C:\path\to\ExecuteCommand4000.exe h "C:\Program Files\Git\usr\bin\xargs" -d '\n' "/c/Program Files/Microsoft VS Code/Code.exe"`
- For multiple files or directories
  - pass files as a string argument (with xargs)
    - `C:\path\to\ExecuteCommand4000.exe h "C:\Program Files\Git\usr\bin\xargs.exe" -d '\n' -- "C:\path\to\script.bat"`
      - Currently not successfull if both input files' and batch file's path contain spaces, mainly because recent cygwin made it difficult (maybe impossible) to pass arbitrary string with double quotation `"` (cf. https://cygwin.com/pipermail/cygwin/2020-June/245226.html).  This also applies to the next example.
  - pass files as a string argument, opening interactive window
    - `C:\path\to\ExecuteCommand4000.exe a xxx "C:\path\to\script.bat" xxx`
      - `script.bat` may contain interactive commands (like `pause`)
      - If you use pipe, you need some other command to keep alive the desired program after EOF input.
        - `C:\path\to\ExecuteCommand4000.exe h "C:\cygwin64\bin\xargs.exe" -d '\n' -- cmd /c start "" cmd /c "C:\path\to\script.bat"`
  - pass to mpv's stdin
    - `C:\path\to\ExecuteCommand4000.exe p C:\path\to\mpv\mpv.exe --player-operation-mode=pseudo-gui --playlist=-`
  - write paths to file
    - `C:\path\to\ExecuteCommand4000.exe h busybox sh -c "cat > $HOME/out.txt"`
  - run a GUI application for each file (don't run this for too many files!) (use cygwin's xargs in recent Windows 11; refer to #3)
    - `C:\path\to\ExecuteCommand4000.exe h "C:\cygwin64\bin\xargs.exe" -d '\n' -n1 -P0 "/cygdrive/c/Program Files/Windows NT/Accessories/wordpad.exe"`
## Invoke the class by `DelegateExecute`
If properly registered as a class, it can be invoked by writing the CLSID (include `{}`) to the `DelegateExecute` value of `command` keys for respective context menus.
At least the following registry keys work. Tested in Windows 11.
- `HKCR\SystemFileAssociations\.xxx\shell` or `HKCR\SystemFileAssociations\XXXXXXX\shell` (right-click)
- `HKCR\*\shell` (right-click, for all files)
- `HKCR\Directory\shell` (right-click, for directories)
- `HKCR\Directory\Background\shell` (right-clicking blank area in directories)
- `HKCR\XXXXXXX\shell`, specified by `HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.xxx\UserChoice` (right-click or as default apps (double-click or return key))
  - Since the values of `UserChoice` cannot be directly changed (even by administrators), you should use a dummy file to associate .xxx with and then change value in `HKCR\XXXXXXX\shell`. 
### Example .reg files

- for .txt
  ```
  Windows Registry Editor Version 5.00

  [HKEY_CURRENT_USER\Software\Classes\SystemFileAssociations\.txt\shell\mycommand]
  @="mycommand_name"
  "Icon"="C:\\WINDOWS\\notepad.exe"

  [HKEY_CURRENT_USER\Software\Classes\SystemFileAssociations\.txt\shell\mycommand\command]
  "DelegateExecute"="{FFA07888-75BD-471A-B325-59274E734000}"

  ```
- for directory background
  ```
  Windows Registry Editor Version 5.00

  [HKEY_CURRENT_USER\Software\Classes\Directory\Background\shell\VSCode]
  @="MyVSCode"
  "Icon"="C:\\Program Files\\Microsoft VS Code\\Code.exe"

  [HKEY_CURRENT_USER\Software\Classes\Directory\Background\shell\VSCode\command]
  "DelegateExecute"="{FFA07888-75BD-471A-B325-59274E734000}"

  ```
# Building
Use Visual Studio or MSBuild.exe. `build.sh` generates multiple exe files with different UUIDs.
# License
- MIT (inherited from [the original Microsoft sample](https://github.com/microsoft/Windows-classic-samples/tree/main/Samples/Win7Samples/winui/shell/appshellintegration/ExecuteCommandVerb))
- public domain for my revision
