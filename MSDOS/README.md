## Compiling

### Requisites

- NodeJS on host OS
- Borland C++ 3.0 in MS-DOS
- Borland Assembler 5.0 in MS-DOS

### 1. Building makefiles

Run node to boot the builder project from the root of the MSDOS folder.

```bash
node ./builder
```

This will create EXEC, LISTLIBS, LIBOBJS, MAKEFILE, RUNTIME, RZRCA.PRJ and SOURCES.MAC

### 2. Configure Dosbox

The dosbox machine should mount where your TC and TASM folders are, likely as C:, and TC\BIN and TASM\BIN should be appended to the PATH. See bottom of `launch.conf` for a windows-based example.

### 3. Compile

Mount the MSDOS folder to D:, navigate to it in Dosbox, and run `make`.

### 4. Launch

Launch BIN\GAME
