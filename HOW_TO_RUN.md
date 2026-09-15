# Build and Run

## Requirements

C++17 compiler with filesystem support. Commands below target Linux with GCC 9 or newer.

## Build

```bash
g++ -std=c++17 -Wall -Wextra -pedantic main.cpp -o minigit
```

## Commands

```text
minigit init
minigit add FILE
minigit commit "MESSAGE"
minigit log
```

Commands operate on `.minigit` in the current directory. `add` accepts a regular filename in that directory, without a parent path. Filenames containing spaces must be quoted. Commit messages are passed directly, without a `-m` option.

## Example session

```bash
mkdir demo
cd demo
../minigit init
printf 'Version one\n' > notes.txt
../minigit add notes.txt
../minigit commit "Initial snapshot"
printf 'Version two\n' > notes.txt
../minigit add notes.txt
../minigit commit "Update notes"
../minigit log
```

The log displays `Update notes` before `Initial snapshot`. IDs and timestamps vary. The index remains populated after a commit. Commands must run one at a time; writes are not transactional.
