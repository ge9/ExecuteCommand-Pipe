# ExecuteCommand-Pipe
A simple tool to run arbitrary commands from context menu in explorer.exe (though it may be run with other method, such as DropTarget).

COM (Component Object Model) technology is used, so it is very versatile (virtually no limitations for path length or number of files).

It executes given command and sends selected file names to the stdin of it (in UTF-8).

# Usage
1. Place ExecuteCommandXXXX.exe anywhere (inside user folder is ok) and run it
2. Open registry editor and make sure its full path is stored in the registry key `HKCU\Software\Classes\CLSID\{FFA07888-75BD-471A-B325-59274E73XXXX}\LocalServer32`.
3. Add any argument to the value as you like (see examples below)
4. Rename "LocalServer32" to any other name and then return it back (this seems the easiest way to apply the change)
5. Add context menu that calls `{FFA07888-75BD-471A-B325-59274E73XXXX}` with `DelegateExecute` method (under `HKCU\Software\Classes\SystemFileAssociations\` will be good; see below) (see other websites if you are not sure)
6. Now you can run the command from context menu!
- Since the UUID must be determined in compile time and we have no method to pass arguments from outside the CLSID, we need a separate executable file for each command. 
# Options
The first character is
- 'd' or '-' ... for debugging; show the first and last files given and exit (execution through LocalServer32 automatically add "-Embedding" argument, so adding no argument in the registry value also result in the debug mode).
- 'p' (or any char) ... it runs given command (the first 2 chars are ignored).
- 'h' ... similar to 'p' but shows no console window.
# Examples
- pass arguments to mpv
  - `C:\Users\yourname\ExecuteCommand4000.exe p C:\path\to\mpv\mpv.exe --playlist=-`
- write to file
  - `C:\Users\yourname\ExecuteCommand4000.exe h busybox sh -c "cat > $HOME/out.txt"`
- run notepad for each file (don't run this for too many files!)
  - `C:\Users\yourname\ExecuteCommand4000.exe h "C:\Program Files\Git\bin\bash.exe" -c "sed 's/\\\\/\\\\\\\\/g;s/ /\\\\ /g' | xargs -n1 -P0 notepad"`
  - (Git bash seems better in this case; busybox's UTF-8 support seems a bit incomplete)
- sample registry entry for .txt
```
Windows Registry Editor Version 5.00

[HKEY_CURRENT_USER\Software\Classes\SystemFileAssociations\.txt\shell\mycommand]
@="mycommand_name"
"Icon"="C:\\WINDOWS\\notepad.exe"

[HKEY_CURRENT_USER\Software\Classes\SystemFileAssociations\.txt\shell\mycommand\command]
"DelegateExecute"="{FFA07888-75BD-471A-B325-59274E734000}"

```
# Building
Use Visual Studio or MSBuild.exe. `build.sh` generates multiple exe files with different UUIDs.
# License
- MIT (derived from [the original Microsoft sample](https://github.com/microsoft/Windows-classic-samples/tree/main/Samples/Win7Samples/winui/shell/appshellintegration/ExecuteCommandVerb))
- public domain for my revision
