# Notes for CSE403 project, RocksDB+TSX
Adam Kafka and Ryan Santos

## Next steps
- Interposition on pthread library
    - At first to get an idea of where locks are being used (output a backtrace every time)
    - Use this information to determine how to procede... continue interposition, or fork code.
    - Can also use interpositioning library to test and verify our TSX with a simple program that uses locks
    - https://stackoverflow.com/questions/3707358/get-all-the-thread-id-created-with-pthread-created-within-an-process/3709027#3709027
    - Open up log file, and wrap pthread\_create such that we log every call with where it came from

- Run examples in rockDB and learn from lock usage. Maybe lock success/failures as well...
