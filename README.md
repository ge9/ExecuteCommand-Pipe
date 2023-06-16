# ExecuteCommand-Pipe
A simple tool to run arbitrary commands with paths (of files or directories) passed from various context menus in Windows Explorer. Paths are encoded in UTF-8, each suffixed by "\n", and then passed to the given command, through standard input/output.
Implemented with COM (Component Object Model) technology to avoid limitations for path length or number of files.

# Usage

## Register as a class

Simply double-click to run `ExecuteCommandXXXX.exe` with no arguments, and it properly registers itself in the registry.
- Each released executable `ExecuteCommandXXXX.exe` has CLSID `{FFA07888-75BD-471A-B325-59274E73XXXX}`.
  - Since the CLSID must be determined in compile time and we have no method to pass arguments from outside the CLSID, we need a separate executable file for each command. 
- When run without admin right, it registers itself in `HKCU\Software\Classes\CLSID\{FFA07888-75BD-471A-B325-59274E73XXXX}`.
  - The full path of the executable is stored in `HKCU\Software\Classes\CLSID\{FFA07888-75BD-471A-B325-59274E73XXXX}\LocalServer32`.
- When run with admin right, it asks if it should use `HKLM` instead of `HKCU`.
  - But user-specific `HKCU` will be safer than system-wide `HKLM`.
  - If you use `HKLM`, you should not place the executable file inside `C:\Users\username`.
- You can also register it manually.
- Note that `HKCR` is the result of merging `HKCU\Software\Classes` and `HKLM\Software\Classes` together.
  - You can edit things in `HKCR`, but you should be aware which of `HKCU` or `HKLM` they come from.
- If you move the executable file, you should update the path in registry.
## Modify the argument
Once the CLSID is registered, you can append any argument to the default value of `[HKCU or HKLM]\Software\Classes\CLSID\{FFA07888-75BD-471A-B325-59274E73XXXX}`, according to your purpose (see examples below).

After the modification, rename "LocalServer32" to any other name and then return it back (this seems the easiest way to reset the cache and apply the change).
### Available Options for ExecuteCommandXXXX.exe
The first character is
- 'd' or '-' ... for debugging; show the first and last files given and exit (execution through LocalServer32 automatically add "-Embedding" argument, so appending no argument also result in this debug mode).
- 'p' (or other characters) ... it runs the given command (the first 2 characters are ignored).
- 'h' ... similar to 'p', but hide the console window.
### Examples
- For a single directory
  - Open Git Bash
    - `C:\path\to\ExecuteCommand4000.exe h cmd /c ""C:\Program Files\Git\usr\bin\cygpath" -f - | "C:\Program Files\Git\usr\bin\xargs" -d '\n' -I {} "C:\Program Files\Git\git-bash.exe" -c "cd \"{}\";exec bash""`
  - Open Git Bash in Windows' default terminal
    - `C:\path\to\ExecuteCommand4000.exe h cmd /c ""C:\Program Files\Git\usr\bin\cygpath" -f - | "C:\cygwin64\bin\xargs.exe" -d '\n' -I {} cmd /c start "" "C:\Program Files\Git\usr\bin\env.exe" MSYSTEM=MINGW64 "C:\Program Files\Git\usr\bin\bash.exe" --login -i -c "cd \"{}\";exec bash""`
    - You need cygwin's xargs here because git bash's executables can't pass arguments to cmd.exe. However, you can workaround this by some other method to run something in Windows' default terminal, for example by https://github.com/ge9/win-console-delegator with `cmd /c start "" `.
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
  - pass files as a string argument, opening in interactive window
    - `C:\path\to\ExecuteCommand4000.exe h "C:\cygwin64\bin\xargs.exe" -d '\n' -- cmd /c start "" cmd /c "C:\path\to\script.bat"`
      - Git Bash's xargs doesn't work here again.
      - In this case, stdin won't be closed forcefully, so interactive commands (e.g. `pause`) in script.bat will be meaningful.
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
- MIT (derived from [the original Microsoft sample](https://github.com/microsoft/Windows-classic-samples/tree/main/Samples/Win7Samples/winui/shell/appshellintegration/ExecuteCommandVerb))
- public domain for my revision
