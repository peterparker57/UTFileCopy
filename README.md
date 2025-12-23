# UTFileCopy DLL

A lightweight 32-bit Windows DLL for performing silent file operations in Clarion applications using the `#RUNDLL` template command.

## Overview

UTFileCopy provides silent file and directory operations without opening visible command windows, solving a common problem when using Clarion's `#RUN` command during template processing and compilation.

### Why This Exists

Clarion's built-in `#RUN` template command executes external commands, but it opens visible command windows that briefly flash on screen during compilation. This can be disruptive during development and looks unprofessional in automated build environments. UTFileCopy solves this by providing silent file operations through a DLL that can be called via `#RUNDLL`, eliminating any visible windows.

## Key Features

- **Silent Operations** - No command windows or UI dialogs
- **Single File Copy** - Copy individual files with optional destination
- **Recursive Directory Copy** - Copy entire directory trees with wildcard support
- **Multiple Directory Creation** - Create nested folder structures in one call
- **Clarion Compatible** - Uses `__stdcall` (PASCAL) calling convention
- **No Dependencies** - Uses only Windows API (no external runtime requirements)
- **32-bit Architecture** - Compatible with Clarion's 32-bit requirements

## Installation

Copy `UTFileCopy.dll` to one of the following locations:

1. Your application's directory
2. Your Clarion template directory (e.g., `C:\Clarion11\accessory\bin\`)
3. Any directory in your system PATH

For development use, placing it in `%CWROOT%\accessory\bin\` makes it available to all Clarion templates.

## Parameter Format

All operations use pipe-delimited (`|`) parameters passed as a single string:

```
operation|parameter1|parameter2|parameter3|...
```

## Operations

### copy - Single File Copy

Copy a single file from source to destination.

**Syntax:**
```
copy|source|destination
copy|source|                  (copies to current directory)
```

**Parameters:**
- `source` - Full or relative path to source file
- `destination` - Full or relative path to destination (optional, defaults to current directory)

**Examples:**
```clarion
#SET(%CopyCommand,'copy|C:\source\file.dll|C:\dest\file.dll')
#RUNDLL('UTFileCopy.DLL', 'SilentCopy', %CopyCommand),PASCAL

#SET(%CopyCommand,'copy|myfile.txt|')
#RUNDLL('UTFileCopy.DLL', 'SilentCopy', %CopyCommand),PASCAL
```

### xcopy - Recursive Directory Copy

Copy files and directories recursively with wildcard support.

**Syntax:**
```
xcopy|source|destination
```

**Parameters:**
- `source` - Source path with optional wildcards (e.g., `C:\source\*.txt` or `C:\source\*`)
- `destination` - Destination directory path

**Examples:**
```clarion
! Copy all files from source directory
#SET(%CopyCommand,'xcopy|C:\source\*|C:\dest')
#RUNDLL('UTFileCopy.DLL', 'SilentCopy', %CopyCommand),PASCAL

! Copy only text files
#SET(%CopyCommand,'xcopy|C:\source\*.txt|C:\dest')
#RUNDLL('UTFileCopy.DLL', 'SilentCopy', %CopyCommand),PASCAL

! Copy entire directory tree
#SET(%CopyCommand,'xcopy|C:\source|C:\dest')
#RUNDLL('UTFileCopy.DLL', 'SilentCopy', %CopyCommand),PASCAL
```

### mkdir - Create Multiple Directories

Create one or more directories, including nested paths, in a single call.

**Syntax:**
```
mkdir|folder1|folder2|folder3|...
```

**Parameters:**
- `folder1, folder2, ...` - One or more directory paths to create

The operation automatically creates parent directories as needed (similar to `mkdir -p` on Unix).

**Examples:**
```clarion
! Create single directory
#SET(%MkdirParams,'mkdir|output')
#RUNDLL('UTFileCopy.DLL', 'SilentCopy', %MkdirParams),PASCAL

! Create nested directory structure
#SET(%MkdirParams,'mkdir|output\bin\x86')
#RUNDLL('UTFileCopy.DLL', 'SilentCopy', %MkdirParams),PASCAL

! Create multiple directories
#SET(%MkdirParams,'mkdir|wwwroot|wwwroot\css|wwwroot\js|wwwroot\images')
#RUNDLL('UTFileCopy.DLL', 'SilentCopy', %MkdirParams),PASCAL
```

## Clarion Usage Examples

### Example 1: Copy Runtime DLLs

Copy necessary DLL files from Clarion's bin directory to the current project output:

```clarion
#DECLARE(%CopyCommand)
#DECLARE(%DLLsToCopy)

! Define DLLs to copy
#SET(%DLLsToCopy,'msvcr120.dll|msvcp120.dll|WebView2Loader.dll')

! Copy each DLL
#FOR(%DLLsToCopy)
  #SET(%CopyCommand,'copy|' & %CWROOT & 'accessory\bin\' & %DLLToCopy & '|')
  #RUNDLL('UTFileCopy.DLL', 'SilentCopy', %CopyCommand),PASCAL
#ENDFOR
```

### Example 2: Setup WebView2 Directory Structure

Create the complete folder structure and copy web resources for a WebView2 control:

```clarion
#DECLARE(%CopyCommand)
#DECLARE(%MkdirParams)

! Create folder structure
#SET(%MkdirParams,'mkdir|wwwroot|wwwroot\css|wwwroot\js|wwwroot\controls')
#RUNDLL('UTFileCopy.DLL', 'SilentCopy', %MkdirParams),PASCAL

! Copy CSS files
#SET(%CopyCommand,'xcopy|' & %CWROOT & 'accessory\resources\wwwroot\css\*|wwwroot\css')
#RUNDLL('UTFileCopy.DLL', 'SilentCopy', %CopyCommand),PASCAL

! Copy JavaScript files
#SET(%CopyCommand,'xcopy|' & %CWROOT & 'accessory\resources\wwwroot\js\*|wwwroot\js')
#RUNDLL('UTFileCopy.DLL', 'SilentCopy', %CopyCommand),PASCAL
```

### Example 3: Copy Control-Specific Resources

Dynamically create folders and copy resources for each COM control:

```clarion
#DECLARE(%CopyCommand)
#DECLARE(%MkdirParams)
#DECLARE(%ControlName)
#DECLARE(%COMHeaders)

! Process each control
#FOR(%COMHeaders)
  ! Get control name
  #SET(%ControlName, %COMHeader & 'Control')

  ! Create control-specific folder
  #SET(%MkdirParams,'mkdir|wwwroot\controls\' & %ControlName)
  #RUNDLL('UTFileCopy.DLL', 'SilentCopy', %MkdirParams),PASCAL

  ! Copy control resources
  #SET(%CopyCommand,'xcopy|' & %CWROOT & 'accessory\resources\wwwroot\controls\' & %ControlName & '\*|wwwroot\controls\' & %ControlName)
  #RUNDLL('UTFileCopy.DLL', 'SilentCopy', %CopyCommand),PASCAL
#ENDFOR
```

### Example 4: Build Output Organization

Organize build outputs into separate directories:

```clarion
#DECLARE(%CopyCommand)
#DECLARE(%MkdirParams)

! Create output structure
#SET(%MkdirParams,'mkdir|dist|dist\bin|dist\resources|dist\docs')
#RUNDLL('UTFileCopy.DLL', 'SilentCopy', %MkdirParams),PASCAL

! Copy executable
#SET(%CopyCommand,'copy|bin\MyApp.exe|dist\bin\MyApp.exe')
#RUNDLL('UTFileCopy.DLL', 'SilentCopy', %CopyCommand),PASCAL

! Copy all DLLs
#SET(%CopyCommand,'xcopy|bin\*.dll|dist\bin')
#RUNDLL('UTFileCopy.DLL', 'SilentCopy', %CopyCommand),PASCAL

! Copy resource files
#SET(%CopyCommand,'xcopy|resources\*|dist\resources')
#RUNDLL('UTFileCopy.DLL', 'SilentCopy', %CopyCommand),PASCAL
```

## API Reference

### SilentCopy Function

```c
__declspec(dllexport) void __stdcall SilentCopy(char* pszParams);
```

**Calling Convention:** `__stdcall` (PASCAL compatible)

**Parameters:**
- `pszParams` - Pipe-delimited string containing operation and parameters

**Return Value:** None (void function)

**Error Handling:** The function performs operations silently and does not return error codes. Failed operations are silently ignored to prevent interrupting template processing.

## Technical Details

### Architecture
- **Platform:** Windows 32-bit
- **Calling Convention:** `__stdcall` (required for Clarion PASCAL compatibility)
- **Character Encoding:** ANSI input, Unicode (UTF-16) internally
- **API Dependencies:** Windows API only (kernel32, shell32, shlwapi)

### Windows APIs Used
- `CopyFileW` - Single file copy operation
- `SHFileOperationW` - Recursive directory/wildcard copy
- `CreateDirectoryW` - Directory creation
- `MultiByteToWideChar` - ANSI to Unicode conversion
- `GetFullPathNameW` - Path resolution

### Flags and Behavior
The `xcopy` operation uses the following `SHFileOperation` flags:
- `FOF_SILENT` - No progress dialog
- `FOF_NOCONFIRMATION` - No confirmation prompts
- `FOF_NOERRORUI` - No error dialogs
- `FOF_NOCONFIRMMKDIR` - Auto-create destination directories

### Thread Safety
The DLL calls `DisableThreadLibraryCalls` on initialization for better performance in single-threaded Clarion template processing.

## Building from Source

### Requirements
- Visual Studio 2019 or 2022 (Community, Professional, or Enterprise)
- Windows SDK
- MSBuild (included with Visual Studio)

### Build Instructions

1. Clone or download the repository
2. Run the build script:
   ```batch
   build.bat
   ```

The build script automatically:
- Locates MSBuild from Visual Studio 2022 or 2019
- Builds the 32-bit Release configuration
- Outputs `UTFileCopy.dll` to `bin\Release\`

### Manual Build

You can also build using Visual Studio:

1. Open `UTFileCopy.sln` in Visual Studio
2. Select **Release** configuration
3. Select **Win32** platform
4. Build Solution (F7)

### Project Structure

```
UTCommand/
├── src/
│   ├── UTFileCopy.c      # Main implementation
│   └── UTFileCopy.h      # Header file
├── UTFileCopy.def        # DLL export definitions
├── UTFileCopy.vcxproj    # Visual Studio project
├── UTFileCopy.sln        # Visual Studio solution
├── build.bat             # Build script
├── bin/
│   └── Release/
│       └── UTFileCopy.dll
└── README.md
```

## Use Cases

### Template Development
Ideal for Clarion template developers who need to:
- Copy runtime dependencies during code generation
- Setup project directory structures
- Deploy web resources for WebView2 controls
- Organize build outputs

### Build Automation
Perfect for automated build scripts that require:
- Silent file operations
- No user interaction
- Reliable directory creation
- Batch file operations

### Application Deployment
Useful for deployment scenarios:
- Copying runtime dependencies
- Creating installation directories
- Organizing application resources
- Setting up configuration structures

## Limitations

- Windows-only (uses Windows API)
- 32-bit architecture (required for Clarion compatibility)
- No error reporting to caller (silent by design)
- ANSI string input (non-Unicode paths may have limitations)
- No progress feedback for large operations

## License

MIT License

Copyright (c) 2025

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

## Support

For questions, issues, or feature requests, please open an issue on the project repository.

## Version History

### 1.0.0
- Initial release
- Single file copy operation
- Recursive directory copy with wildcards
- Multiple directory creation
- Full Clarion PASCAL compatibility
