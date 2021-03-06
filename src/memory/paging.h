#ifndef _HUGOS_PAGING_H
#define _HUGOS_PAGING_H

#include <stdint.h>
#include <stdbool.h>
#include "memory.h"
#include "allocator.h"
#include <utils.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	bool writeable;
	bool user_accessible;
	bool write_through;
	bool cache_disabled;
	bool accessed;
	bool dirty;
	bool huge;
	bool global;
	bool no_execute;
} entry_flags_t;

int32_t map_page(uint64_t page_addr, entry_flags_t flags, allocator_vtable *allocator);
int32_t map_page_to(uint64_t page_addr, uint64_t frame_addr, entry_flags_t flags, allocator_vtable *allocator);

int32_t unmap_page(uint64_t page_addr, allocator_vtable *allocator);
void unmap_page_no_free(uint64_t page_addr);

int32_t translate_page(uint64_t page_addr, uint64_t *frame_addr);
int32_t translate_addr(uint64_t virtual_addr, uint64_t *physical_addr);

typedef struct {
	uint64_t _backup_addr;
} mapper_ctx_t;

mapper_ctx_t mapper_ctx_begin(uint64_t p4_frame_addr, allocator_vtable *allocator);
void mapper_ctx_end(mapper_ctx_t ctx);

uint64_t create_p4_table(allocator_vtable *allocator);

int32_t set_entry_flags_for_address(uint64_t addr, entry_flags_t flags);

void mark_for_no_map(uint64_t addr, allocator_vtable *allocator);
void unmark_for_no_map(uint64_t addr);

#ifdef __cplusplus
}
#endif

#endif
