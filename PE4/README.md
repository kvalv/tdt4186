To run an interactive repl:

```
./repl
```
This will generate `src/scanner.c`, and use GCC to compile `wish`.

The repl-script will automaticly run `wish testfile.wsh`, which will execute the commands written in testfile.wsh, line by line.

To use wish as a shell, simply enter `./wish`

Other scripts can be tried by editing `testfile.wsh`.

Tested and working on a VM through SSH at login.stud.ntnu.no