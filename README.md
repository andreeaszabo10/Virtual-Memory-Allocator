Copyright Szabo Cristina-Andreea 2022-2023
# Virtual Memory Allocator (VMA)

## Description
The Virtual Memory Allocator (VMA) is a simulation program designed to manage virtual memory blocks dynamically. It handles memory allocation, deallocation, and management of access permissions, simulating how memory operations occur at a low level in systems programming.

## Features
- **Arena Allocation**: Create a large memory arena to manage virtual memory blocks.
- **Block Allocation**: Allocate memory blocks within the arena specifying starting addresses and sizes.
- **Block Deallocation**: Free allocated blocks based on their starting address.
- **Read/Write Operations**: Perform read and write operations on allocated memory blocks with permission checks.
- **Permission Management**: Set access permissions (Read, Write, Execute) for each memory block.
- **Memory Mapping**: Display the current layout and status of memory blocks and their permissions.
- **Protection Control**: Modify the permissions for specific memory blocks.

3. **Commands**:
- `ALLOC_ARENA <size>`: Initialize the arena with the specified size.
- `ALLOC_BLOCK <address> <size>`: Allocate a block at a specific address.
- `WRITE <address> <size> <data>`: Write data to a block.
- `READ <address> <size>`: Read data from a block.
- `MPROTECT <address> <PERMISSIONS>`: Change the permissions of a block. Permissions can be combinations of `PROT_READ`, `PROT_WRITE`, and `PROT_EXEC`.
- `PMAP`: Display the memory mapping of the arena.
- `FREE_BLOCK <address>`: Free a block starting at the given address.
- `DEALLOC_ARENA`: Deallocate the entire arena and exit the program.

## Memory Permissions
- `PROT_READ`: Allows reading from the memory block.
- `PROT_WRITE`: Allows writing to the memory block.
- `PROT_EXEC`: Allows executing code from the memory block.
- Permissions can be combined using `|`, such as `PROT_READ | PROT_WRITE`.

## Error Handling
The program handles errors such as:
- Invalid addresses for memory operations.
- Access permission violations.
- Attempts to allocate memory over already allocated regions.
- Invalid commands.
