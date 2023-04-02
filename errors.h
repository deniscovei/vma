#pragma once
#include <stdio.h>
#include <errno.h>

#define MALLOC_FAILURE "malloc() failed"
#define UNINITIALISED_LIST "Uninitialised list, element cannot be inserted\n"
#define INVALID_START_ADDRESS "The allocated address is outside the size of arena\n"
#define INVALID_END_ADDRESS "The end address is past the size of the arena\n"
#define INVALID_ALLOC_BLOCK "This zone was already allocated.\n"
#define INVALID_ADDRESS_FREE "Invalid address for free.\n"
#define INVALID_ADDRESS_MPROTECT "Invalid address for mprotect.\n"
#define INVALID_ADDRESS_WRITE "Invalid address for write.\n"
#define INVALID_PERMISSIONS_WRITE "Invalid permissions for write.\n"
#define INVALID_ADDRESS_READ "Invalid address for read.\n"
#define INVALID_PERMISSIONS_READ "Invalid permissions for read.\n"
#define WARNING_WRITE_SIZE "size was bigger than the block size. Writing %ld characters.\n"
#define WARNING_READ_SIZE "size was bigger than the block size. Reading %ld characters.\n"
#define INVALID_COMMAND "Invalid command. Please try again.\n"

#define CHECK_MEMORY(mem_block, err_msg) \
	do { \
		if (!(mem_block)) { \
			fprintf(stderr, err_msg); \
			fprintf(stderr, " (%s:%d)\n", __FILE__, __LINE__); \
			exit(errno); \
		} \
	} while (0)

#define DISPLAY_ERROR(err_msg) printf(err_msg)

#define DISPLAY_WARNING(wrn_msg, opt) printf("Warning: " wrn_msg, opt)
