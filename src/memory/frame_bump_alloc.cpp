#include "frame_bump_alloc.hpp"
#include <utils.h>
#include <stdio.h>

const allocator_vtable frame_bump_alloc_state_vtable = {
	.allocate = reinterpret_cast<uint64_t(*)(struct _allocator_vtable*)>(frame_bump_alloc_state::bump_alloc_allocate),
	.deallocate = nullptr
};

uint64_t frame_bump_alloc_state::allocate() {
	uint64_t attempt = ALIGN_UP(this->next_alloc, 0x1000);

	while (1) {
		fprintf(stdserial, "Attempt alloc at %p\n", attempt);
		if (attempt >= this->kernel_start && attempt < this->kernel_end) {
			fprintf(stdserial, "bump alloc kernel jump\n");
			attempt = ALIGN_UP(this->kernel_end, 0x1000);
			continue;
		}

		if (attempt >= this->multiboot_start && attempt < this->multiboot_end) {
			fprintf(stdserial, "bump alloc multiboot jump\n");
			attempt = ALIGN_UP(this->multiboot_end, 0x1000);
			continue;
		}

		for (multiboot::memory_map_entry entry : *this->mem_map) {
			if (entry.base_addr <= attempt && attempt < entry.base_addr + entry.length) {
				if (entry.type == multiboot::memory_type::AVAILABLE) {
					this->next_alloc = attempt + 0x1000;
					return attempt;
				} else {
					attempt = ALIGN_UP(entry.base_addr + entry.length, 0x1000);
					break;
				}
			}
		}
		attempt = ALIGN_UP(attempt + 0x1000, 0x1000);
	}
}

uint64_t frame_bump_alloc_state::bump_alloc_allocate(frame_bump_alloc_state *allocator) {
	return allocator->allocate();
}
