#include "multiboot.hpp"
#include <utils.hpp>

MultibootData::tag_union MultibootData::find_tag(multiboot::tag_types type) {
	auto current_tag = this->mb_data_start;
	while (current_tag < this->mb_data_end) {
		if (current_tag->type == static_cast<uint32_t>(type)) {
			return tag_union { .generic = current_tag };
		}
		current_tag = add_bytes(current_tag, current_tag->size);
		current_tag = align_up(current_tag, 8);
	}
	return tag_union { .generic = nullptr };
}

void MultibootData::print_tags() {
	auto current_tag = this->mb_data_start;
	while (current_tag < this->mb_data_end) {
		kprintf("Tag type: %u, size: 0x%x\n", current_tag->type, current_tag->size);
		current_tag = add_bytes(current_tag, current_tag->size);
		current_tag = align_up(current_tag, 8);
	}
}