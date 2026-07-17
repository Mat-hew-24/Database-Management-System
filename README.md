# NITCbase — A Lightweight Database Management System

**NITCbase** is a compact, educational Database Management System (DBMS) implemented in C++. It demonstrates core database concepts including block-level storage management, buffer pools, B+ tree indexing, query execution, and relational algebra operations. Designed as a teaching tool for understanding how databases work at the systems level.

---

## Project Overview

NITCbase provides a working implementation of:

- **Block-level Storage** — Direct disk I/O with 2KB blocks and custom block allocation.
- **Buffer Management** — LRU-based block cache with configurable capacity.
- **B+ Tree Indices** — Primary and secondary indexing on relations with efficient range queries.
- **Relation & Attribute Catalogs** — Metadata management for tables and their columns.
- **Query Execution** — Relational algebra operators (select, project, join) with query planning.
- **Record Management** — Slotted page format for efficient record storage and updates.
- **CLI Frontend** — Simple command-line interface for interactive database operations.

---

## Specifications

### Storage Layer

| Specification | Value |
|---|---|
| **Block Size** | 2,048 bytes |
| **Total Disk Size** | 16 MB |
| **Total Blocks** | 8,192 |
| **Block Allocation Map Blocks** | 4 |
| **Relation Catalog Block** | Block #4 |
| **Attribute Catalog Block** | Block #5 |

### Memory & Caching

| Specification | Value |
|---|---|
| **Buffer Capacity** | 32 blocks |
| **Max Open Relations** | 12 |
| **Attribute Size** | 16 bytes |
| **Header Size per Block** | 32 bytes |

### Data Types & Operators

- **Attribute Types:** `NUMBER` (integer/float), `STRING`
- **Conditional Operators:** `=`, `<=`, `<`, `>=`, `>`, `!=`
- **Block Types:** Record, Internal Index, Leaf Index, Unused, Block Allocation Map

---

## Folder Structure

```
Database-Management-System/
├── README.md                           # This file
├── Dockerfile                          # Docker setup for containerized execution
│
├── NITCbase/
│   ├── Disk/
│   │   ├── disk                        # Virtual disk file (16 MB, binary)
│   │   └── disk_run_copy               # Backup copy for safe testing
│   │
│   ├── Files/
│   │   ├── Input_Files/                # Sample CSV data for import
│   │   │   ├── quizmarks.csv
│   │   │   ├── s*.csv                  # Various student/event datasets
│   │   │   └── ...
│   │   ├── Output_Files/               # Catalogs & metadata (auto-generated)
│   │   │   ├── attribute_catalog
│   │   │   ├── block_allocation_map
│   │   │   └── relation_catalog
│   │   └── Batch_Execution_Files/      # Pre-scripted test queries
│   │
│   ├── mynitcbase/                     # Core DBMS implementation
│   │   ├── main.cpp                    # Entry point & stage implementations
│   │   ├── Makefile                    # Build configuration
│   │   ├── nitcbase                    # Compiled binary
│   │   │
│   │   ├── Algebra/
│   │   │   ├── Algebra.h               # Query operators (select, project, join)
│   │   │   └── Algebra.cpp
│   │   │
│   │   ├── BlockAccess/
│   │   │   ├── BlockAccess.h           # Low-level block I/O layer
│   │   │   └── BlockAccess.cpp
│   │   │
│   │   ├── BPlusTree/
│   │   │   ├── BPlusTree.h             # B+ tree index implementation
│   │   │   └── BPlusTree.cpp
│   │   │
│   │   ├── Buffer/
│   │   │   ├── BlockBuffer.h           # Single block cache entry
│   │   │   ├── BlockBuffer.cpp
│   │   │   ├── StaticBuffer.h          # Global buffer pool (LRU)
│   │   │   └── StaticBuffer.cpp
│   │   │
│   │   ├── Cache/
│   │   │   ├── OpenRelTable.h          # Open relation tracking
│   │   │   ├── OpenRelTable.cpp
│   │   │   ├── RelCacheTable.h         # Relation metadata cache
│   │   │   ├── RelCacheTable.cpp
│   │   │   ├── AttrCacheTable.h        # Attribute metadata cache
│   │   │   └── AttrCacheTable.cpp
│   │   │
│   │   ├── Disk_Class/
│   │   │   ├── Disk.h                  # Disk abstraction layer
│   │   │   └── Disk.cpp
│   │   │
│   │   ├── Frontend/
│   │   │   ├── Frontend.h              # Query executor & relation ops
│   │   │   └── Frontend.cpp
│   │   │
│   │   ├── FrontendInterface/
│   │   │   ├── FrontendInterface.h     # CLI command parser & dispatcher
│   │   │   ├── FrontendInterface.cpp
│   │   │   └── RegexHandler.h          # Regex-based query parsing
│   │   │
│   │   ├── Schema/
│   │   │   ├── Schema.h                # Catalog & schema management
│   │   │   └── Schema.cpp
│   │   │
│   │   ├── define/
│   │   │   ├── constants.h             # Global constants, enums, macros
│   │   │   └── id.h                    # Error & return codes
│   │   │
│   │   └── build/                      # Object files & intermediate builds
│   │
│   └── XFS_Interface/
│       ├── Makefile
│       ├── xfs-interface               # Alternative CLI build
│       └── *.cpp, *.h                  # Mirrored source files for XFS (filesystem-backed) tests
│
├── SQL_EXAM_REFS/
│   ├── sql-files/                      # SQL DDL & DML templates
│   ├── answers/                        # Reference SQL query solutions
│   │   ├── a1.sql, a2.sql, ...         # Answers to 25+ exam questions
│   │   └── ...
│   ├── data/                           # Reference datasets
│   │   ├── Course.csv
│   │   ├── Department.csv
│   │   ├── Faculty.csv
│   │   └── ...
│   └── pics/                           # Diagrams & reference materials
```

---

## Key Capabilities

### 1. Relational Data Management
- Create, insert, update, and delete relations dynamically.
- Support for fixed-width attributes (STRING, NUMBER).
- Primary key enforcement via B+ tree indices.

### 2. Efficient Indexing
- B+ tree construction and maintenance.
- Range queries and exact match lookups.
- Automatic index updates on data modification.

### 3. Query Processing
- Select / Project / Join operations.
- Predicate pushdown and simple query optimization.
- Batch query execution from script files.

### 4. Persistent Storage
- Block-based virtual disk with custom allocation map.
- Crash-safe design (disk image persisted between runs).
- Slotted page record layout for variable-length fields.

### 5. Resource Management
- Multi-level caching: buffer pool → relation cache → attribute cache.
- Lazy initialization of relation metadata.
- Limit of 12 concurrently open relations to prevent thrashing.

---

## Building & Running

### Prerequisites
- **GCC** or **Clang** C++ compiler (C++11 or later)
- **GNU Make**
- **Linux/Unix-like OS** (tested on Ubuntu 20.04+)

### Build

```bash
cd NITCbase/mynitcbase
make
```

This produces:
- `nitcbase` — Main compiled binary
- `build/` — Directory containing per-module object files

### Run

```bash
./nitcbase
```

This launches an interactive CLI where you can enter SQL-like commands to create tables, insert data, and run queries.

### Docker

```bash
docker build -t nitcbase .
docker run -it nitcbase
```

---

## Usage Example

```sql
-- Create a relation (table)
CREATE TABLE students (rollno INT PRIMARY KEY, name STRING, gpa NUMBER);

-- Insert records
INSERT INTO students VALUES (1, "Alice", 3.8);
INSERT INTO students VALUES (2, "Bob", 3.6);

-- Query with selection
SELECT * FROM students WHERE gpa > 3.7;

-- Project specific columns
SELECT name, gpa FROM students;
```

---

## Testing & Samples

Sample datasets are provided under `NITCbase/Files/Input_Files/`:
- `quizmarks.csv`, `s8products.csv`, `s11students.csv`, etc.

Batch execution files in `NITCbase/Files/Batch_Execution_Files/` allow scripted test runs.

Expected output is written to `NITCbase/Files/Output_Files/` with catalogs and results.

---

## Architecture Highlights

### Layered Design
1. **Disk Layer** — Virtual disk with block-level I/O.
2. **Buffer Layer** — In-memory LRU cache for hot blocks.
3. **Index Layer** — B+ tree indices for fast lookups.
4. **Cache Layer** — Relation & attribute metadata caching.
5. **Algebra Layer** — Relational operators (select, project, join).
6. **Frontend Layer** — CLI parser and query dispatcher.

### Error Handling
- Comprehensive error codes in `define/id.h`.
- Graceful degradation for resource limits (buffer full, too many open relations).

---

## Contributing & Extending

To add features or modify the system:

1. **Understand the layer** — Identify which module(s) your feature touches.
2. **Review existing code** — Check header files for expected interfaces.
3. **Implement & test** — Use the sample data or add your own test cases.
4. **Update catalogs** — Ensure relation/attribute metadata stays consistent.
5. **Document changes** — Include inline comments explaining non-obvious logic.

---

## Known Limitations

- **No transaction support** — Single-threaded, no ACID guarantees.
- **Limited SQL** — Basic DDL/DML only; no aggregates, subqueries, or CTEs.
- **Single-threaded** — No concurrent query execution.
- **Fixed schema** — Cannot alter table structure after creation.
- **No compression** — Stores records uncompressed in blocks.

---

## References

- **B+ Tree Algorithm** — Refer to `NITCbase/mynitcbase/BPlusTree/`.
- **SQL Exam References** — Sample solutions in `SQL_EXAM_REFS/answers/`.
- **Disk Structure** — See `NITCbase/Disk/disk` (binary, use hex editor to inspect).

---

## License

This repository does not include a license file. If you intend to publish or redistribute this project, consider adding a `LICENSE` file (e.g., MIT, Apache-2.0).

---

## Questions or Issues?

For questions about specific modules, see the header files (`*.h`) in each directory—they contain detailed documentation of interfaces and expected behavior.

For known issues or bugs, refer to the code comments and the batch execution files to understand expected behavior.

---

**Happy exploring!** 🚀
