# NITCbase: Relational Database Management System

NITCbase is an educational Relational Database Management System (RDBMS) designed to teach the internal workings of a database. It features a modular, layered architecture that abstracts low-level disk storage into a high-level relational model.

## Project Structure

- **NITCbase/mynitcbase/**: The primary implementation of the DBMS.
  - **Disk_Class/**: Manages the virtual disk file (`disk`).
  - **Buffer/**: Implements a buffer cache for disk blocks.
  - **Cache/**: Maintains in-memory caches for relation and attribute metadata.
  - **BlockAccess/**: Handles block-level data manipulation and searching.
  - **BPlusTree/**: Implements B+ Tree indexing for efficient retrieval.
  - **Schema/**: Manages Data Definition Language (DDL) operations like creating/deleting relations.
  - **Algebra/**: Implements Data Manipulation Language (DML) operations (Select, Project, Join, Insert).
  - **Frontend/**: High-level API for database operations.
  - **FrontendInterface/**: The Command Line Interface (CLI) layer using `readline`.
  - **define/**: Contains global constants, error codes, and shared data structures.
- **NITCbase/Disk/**: Contains the virtual disk file (`disk`) used by the system.
- **NITCbase/Files/**: Contains input CSVs, batch execution files, and reference output files.
- **NITCbase/XFS_Interface/**: A standalone tool for managing the virtual disk (e.g., formatting, importing data from the host OS).
- **SQL_EXAM_REFS/**: Reference SQL materials, sample data, and query answers.

## Architecture & Layers

NITCbase follows a strict layered architecture:
1.  **Frontend Interface**: Processes user commands and displays results.
2.  **Algebra/Schema Layer**: Translates user commands into relational operations.
3.  **Block Access Layer**: Performs record-level operations using linear or indexed searches.
4.  **B+ Tree Layer**: Manages indices for optimized data access.
5.  **Cache Layer**: Provides fast access to frequently used metadata.
6.  **Buffer Layer**: Minimizes disk I/O by caching blocks in memory.
7.  **Disk Layer**: Interfaces directly with the virtual disk file.

## Building and Running

### Main DBMS (`mynitcbase`)
Build commands are executed within the `NITCbase/mynitcbase` directory:

- **Build**: `make`
- **Debug Build**: `make mode=debug`
- **Run**: `./nitcbase`
- **Clean**: `make clean`

### XFS Interface Tool
Build commands are executed within the `NITCbase/XFS_Interface` directory:

- **Build**: `make`
- **Run**: `./xfs-interface`
- **Clean**: `make clean`

### Docker
A `Dockerfile` is provided at the root for a consistent development environment:

- **Build Image**: `docker build -t nitcbase .`
- **Run Container**: `docker run -it -v $(pwd):/home/nitcbase/NITCbase nitcbase`

## Development Conventions

- **Language**: C++
- **Layering**: Always adhere to the layered architecture. Higher layers should only call lower layers.
- **Error Handling**: Use the error codes defined in `NITCbase/mynitcbase/define/constants.h`. Functions should return `SUCCESS` or a specific error code.
- **Disk Persistence**: Ensure the buffer is flushed (either manually or via the `StaticBuffer` destructor) to persist changes to the `disk` file.
- **External Dependencies**: The project uses `readline` for its CLI.
- **Constants**: Refer to `define/constants.h` for system limits (e.g., `BLOCK_SIZE`, `MAX_OPEN`, `ATTR_SIZE`).

## Common CLI Commands

Once running `./nitcbase`, you can use commands such as:
- `HELP`: Display available commands.
- `OPEN <RELNAME>`: Open a relation.
- `CLOSE <RELNAME>`: Close a relation.
- `SELECT ...`: Query data from relations.
- `INSERT INTO <RELNAME> VALUES (...)`: Add records.
- `CREATE TABLE <RELNAME> (...)`: Define a new relation.
- `RUN <FILENAME>`: Execute a batch of commands from a file in `Batch_Execution_Files`.
- `EXIT`: Terminate the session.
