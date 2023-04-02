# README

Name: `Covei Denis` \
Group: `312CA`

## Virtual Memory Allocator
This is the `Virtual Memory Allocator`. It simulates the operating system virtual memory. It is writen in C.


## What does it do?
A kernel buffer is virtually allocated. That means that the user can work with a supposedly contiguous buffer, which is in fact a list of sparsed memory blocks that the allocator handles in order to make them seem "contiguous".

Basically, the user works with a list of blocks, which are rows of consecutive sparsed miniblocks. So at a given moment, a block will start at an address and end at another. Its miniblocks will start at the same start address, will follow one another, and end at the same end address as the block's.

In the background, those blocks miniblocks are not consecutive, but represented as being so.

## Usage

In order to build and run the program, in the terminal, type the following command:

```console
$ make run
```

You can run the following commands:

1. Allocate a contiguous buffer which will simulate the virtual memory. It will be used as a kernel buffer.
```console
# command used
ALLOC_ARENA <size>
```

2. Free all the allocated memory.
```console
# command used
DEALLOC_ARENA
```

3. Allocate a miniblock of `size` bytes starting at a certain `address` in the arena.
```console
# command used
ALLOC_BLOCK <address> <size>
```

4. Display a number of `size` bytes at a given `address` in the arena. If there is not enough space allocated or the data at the address does not have enouch bytes written, the result will be truncated.
```console
# command used
READ <address> <size>
```

5. Write a number of `size` bytes at a given `address` in the arena. If there is not enouch space allocated to write in the arena, `data` will be truncated.
```console
# command used
WRITE <address> <size> <data>
```

6. Display data about the arena.
```console
# command used
PMAP
```
