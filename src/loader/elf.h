#ifndef elf_HEADER
#define elf_HEADER

#include "elf.h"

#include "loader.h"


struct elf {
    loader_object loader;
    uint8_t * data;
    size_t size;
};


struct elf * elf_create          (const void * data, size_t size);
struct elf * elf_create_filename (const void * data, size_t size);
void         elf_delete (struct elf * elf);
struct elf * elf_copy   (const struct elf * elf);

uint64_t        elf_entry  (const struct elf * elf);
struct memmap * elf_memmap (const struct elf * elf);

#endif