# 🗄️ NITCbase - A Relational Database Management System

<div align="center">

![C++](https://img.shields.io/badge/Language-C%2B%2B-blue?style=flat-square)
![Build](https://img.shields.io/badge/Build-Makefile-green?style=flat-square)
![License](https://img.shields.io/badge/License-Educational-orange?style=flat-square)

A **minimal relational database management system** from scratch, built in C++ as an educational project at NITC.

</div>

---

An educational, block-based **relational DBMS** implemented in C++.

This README is based on what the code _actually does_ (CLI grammar, disk layout, limits, and required workflow).

## Quick start

### Build

From the `mynitcbase/` directory:

```bash
make
```

Debug build:

```bash
make mode=debug
```

### Run

Important: the program uses relative paths like `../Disk/disk` and `../Files/...`, so run it **from** `mynitcbase/`.

Interactive mode:

```bash
./nitcbase
```

Run a provided batch script (reads from `../Files/Batch_Execution_Files/`):

```bash
./nitcbase run s8test.txt
```

You can also run batch scripts from inside the prompt:

```text
# RUN s8test.txt
```

## Dependencies

- `g++`, `make`
- `libreadline` (linked via `-lreadline`)

On Ubuntu/Debian:

```bash
sudo apt-get install -y build-essential libreadline-dev
```

There is also a top-level `Dockerfile` in the repository root for a preconfigured build environment.

## How the system boots

On startup the program:

1. Copies `../Disk/disk` → `../Disk/disk_run_copy`
2. Runs all reads/writes against `disk_run_copy`
3. On _graceful exit_, copies `disk_run_copy` → `disk`

If the program is killed, your last session’s changes may not be persisted back to `disk`.

## Command language (what’s implemented)

Commands are case-insensitive and the trailing `;` is optional.

### Help / utility

```sql
HELP;
ECHO hello_world;
RUN s11test.txt;
EXIT;
```

### DDL

```sql
CREATE TABLE Students(id NUM, name STR, cgpa NUM);
DROP TABLE Students;

OPEN TABLE Students;
CLOSE TABLE Students;

CREATE INDEX ON Students.id;
DROP INDEX ON Students.id;

ALTER TABLE RENAME Students TO Learners;
ALTER TABLE RENAME Learners COLUMN name TO full_name;
```

### DML

Workflow note: **most DML requires the source relation to be OPEN**.

Insert one record:

```sql
INSERT INTO Students VALUES (1, alice, 9.1);
```

Insert from a CSV file (read from `../Files/Input_Files/`):

```sql
INSERT INTO Students VALUES FROM s11students.csv;
```

Project (copy all rows / subset of attributes):

```sql
SELECT * FROM Students INTO Students_copy;
SELECT id, name FROM Students INTO Students_id_name;
```

Select with a single predicate (operators supported: `=`, `<`, `<=`, `>`, `>=`, `!=`):

```sql
SELECT * FROM Students INTO Top WHERE cgpa >= 9;
SELECT id, name FROM Students INTO TopNames WHERE cgpa >= 9;
```

Equi-join (only equality join is supported by the grammar):

```sql
SELECT * FROM A JOIN B INTO AB WHERE A.x = B.y;
SELECT A_x, B_y FROM A JOIN B INTO ABproj WHERE A.x = B.y;
```

### `FUNCTION ...`

There is a `FUNCTION ...` command hook in the parser, but the default implementation currently returns `SUCCESS` without doing work.

## Data types and input rules

- Column types: `NUM` and `STR`
  - `NUM` accepts integers and floating-point values.
- Tokenization is whitespace/comma based.
  - Practical implication: string values are **unquoted single tokens** (no spaces).
  - Values like `"Alice"` / `'Alice'` are not part of the accepted grammar.
- Empty CSV fields are rejected (“Null values not allowed”).

## Limits (from constants)

- Disk: 16 MiB total (`8192` blocks × `2048` bytes)
- Buffer pool: `32` blocks (LRU-style replacement)
- Max open relations in cache: `12`
- Max attributes per relation: `125`
- Attribute name storage size: `16` bytes (names longer than this are truncated with a warning)
- Internal temp relation name: `.temp` (used by some multi-step commands)

## Disk layout (high level)

The database is stored as a single binary “disk” file.

- Block Allocation Map: blocks `0–3`
- Relation catalog: block `4`
- Attribute catalog: starting block `5`

These catalogs are expected to exist in the initial `../Disk/disk` image.

## Running the provided tests

Batch files live in `../Files/Batch_Execution_Files/`.

Examples:

```bash
./nitcbase run s8test.txt
./nitcbase run s11test.txt
```

CSV inputs used by the tests live in `../Files/Input_Files/`.

## Project structure

```
mynitcbase/
  main.cpp
  Makefile
  define/              constants.h, id.h
  Disk_Class/          disk file copy + block I/O
  Buffer/              buffer pool + block buffers
  Cache/               open relation table + rel/attr caches
  Schema/              create/drop/open/close/rename, index management
  BlockAccess/         record-level operations
  BPlusTree/           indexing implementation
  Algebra/             insert/select/project/join execution
  Frontend/            thin wrapper calling Schema/Algebra
  FrontendInterface/   readline CLI + regex-based parser
  build/               generated objects (created by Makefile)
```

There is also an alternate/legacy interface in `../XFS_Interface/`.

## Common gotchas

- Run from `mynitcbase/` so the relative `../Disk` and `../Files` paths resolve.
- `OPEN TABLE ...` before doing `INSERT`/`SELECT` on that relation.
- Target relations in `SELECT ... INTO target` must not already exist.
- If you hard-kill the process, the session’s `disk_run_copy` may not get copied back to `disk`.
