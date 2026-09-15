# Mini Git

## What it does

Mini Git is a C++17 version-control application that saves file snapshots and maintains commit history. It provides four operations: `init` creates a repository, `add` stages file contents, `commit` records a snapshot, and `log` displays commits from newest to oldest.

The application tracks individual files in the repository's root folder. It supports text and binary files. Branches, merging, file restoration, and remote repositories are outside its scope.

## How it works

Repository data is stored in `.minigit`. File contents are saved as **blobs**. A **tree** maps filenames to blob IDs, and a **commit** records a tree ID, a message, a timestamp, and the previous commit ID.

Each object is stored under an ID calculated from its contents using the 64-bit FNV-1a hash. Identical objects reuse the same stored file. FNV-1a is not a cryptographic hash, and the storage format is not compatible with real Git.

The **index** holds the staged snapshot. Staging captures a file's contents at that moment; later edits do not affect the snapshot until the file is staged again. **HEAD** stores the newest commit ID. History is read by following the links from each commit to its parent.
