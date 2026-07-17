# nitcbase

A compact educational Database Management System (DBMS) project used for teaching and experimentation.

This repository contains the student-facing implementation of the `nitcbase` DBMS: a modular C++ project that demonstrates core concepts such as block access, buffer management, B+ trees, caching, and a simple frontend interface.

Key goals
- Provide a lightweight, hands-on DBMS codebase suitable for assignments and labs.
- Demonstrate low-level storage and indexing mechanisms.
- Offer clear modules so students can implement, test, and extend components.

Repository layout (important folders)

- `Algebra/` — Relational algebra and query operators.
- `BlockAccess/` — Low-level block read/write interfaces.
- `BPlusTree/` — B+ tree index implementation.
- `Buffer/` — Block buffering and in-memory cache logic.
- `Cache/` — Relation and attribute cache tables.
- `Disk_Class/` — Disk abstraction layer.
- `Frontend/` and `FrontendInterface/` — User-facing CLI and command parsing.
- `Schema/` — Schema definitions and catalog handling.

Quick start

1. Ensure you have a C++ compiler and `make` installed (tested with g++ on Linux).
2. From this directory, build the project:

```bash
cd NITCbase/mynitcbase
make
```

3. Run the provided binary (if built):

```bash
./nitcbase
# or check build artifacts under `build/` for per-module binaries
```

Development tips
- Code is organized by component; when modifying or adding features, prefer small, focused commits.
- Use the `build/` directory artifacts while testing module-level changes.
- Many modules include headers (`*.h`) with inline documentation — start there to understand expected interfaces.

Testing & sample data
- The repository includes sample input files under `NITCbase/Files/Input_Files/` and batch execution files under `NITCbase/Files/Batch_Execution_Files/` for quick functional testing.
- To simulate disk-backed operations, `NITCbase/Disk/disk` can be used as the storage file — handle it carefully.

Contributing

If you want to contribute or extend this project:

1. Fork the repository.
2. Create a branch for your feature or fix.
3. Add tests or sample input demonstrating the change.
4. Open a PR with a concise description of the change and any required steps to verify it.

Authors & acknowledgements

This project is maintained for educational use. Credit the original course authors and maintainers when reusing substantial parts.

License

This repository does not include a license file. If you intend to publish or share this project broadly, consider adding a `LICENSE` file (MIT, Apache-2.0, or similar).

Need changes?

Tell me if you'd like a shorter README, a version tailored for instructors, or a README that includes build badges and CI instructions — I can update it.
