/**
 * CILO Elf Loader
 * (c) 2008 Philippe Vachon <philippe@cowpig.ca>
 *
 * Licensed under the GNU General Public License v2. See COPYING
 * in the distribution source directory for more information.
 */
#include <elf.h>
#include <promlib.h>
#include <printf.h>
#include <ciloio.h>

/* platform-specific defines */
#include <platform.h>

/**
 * load a single ELF section into memory at address. Assumes ELF data is
 * contiguous in memory.
 * @param base location (in memory) of the ELF file
 * @param address address at which the ELF section will be loaded
 * @param file_offset offset (in bytes) in the ELF file where the section is
 * @param length Length of the section (in bytes)
 */
void load_elf32_section(struct file *fp, uint32_t address,
    uint32_t file_offset, uint32_t length)
{
    uint8_t *elf_loc = (uint8_t *)address;

#ifdef DEBUG
    printf("Init data: %08x length %08x\n", address, length);
#endif

    cilo_seek(fp, (uint32_t)file_offset, SEEK_SET);
    cilo_read(elf_loc, (uint32_t)length, 1, fp);

}


/**
 * Create an uninitialized data (.bss) region of memory.
 * @param address Start address of this region
 * @param lenght length of this region
 */
void load_elf32_uninitialized_memory(uint32_t address, uint32_t length)
{
    int i = 0;
    uint8_t *p = (uint8_t *)address;

#ifdef DEBUG
    printf("Uninit data: %08x, len %08x\n", address, length);
#endif

    for (i = 0; i < length; i++) {
        p[i] = 0;
    }
}

/**
 * Load an ELF file into memory from the given base. Loads at
 * offset + image_size so that a later memcpy routine can be used to copy
 * things into RAM and then kick off the boot process.
 * @param base The address of the ELF file in memory
 * @param loader_addr address of the loader binary in memory
 * @return
 */
void load_elf32_file(struct file *fp, char *cmd_line)
{
    struct elf32_header hdr;
    uint32_t mem_sz = 0;

    /* read in header entries */
    cilo_read(&hdr, sizeof(struct elf32_header), 1, fp);

    /* check the file magic */
    if (hdr.ident[0] != ELF_MAGIC_1 || hdr.ident[1] != ELF_MAGIC_2 ||
        hdr.ident[2] != ELF_MAGIC_3 || hdr.ident[3] != ELF_MAGIC_4)
    {
        printf("Bad ELF magic found. Found: %#2x %#2x %#2x %#2x.\n",
            hdr.ident[0], hdr.ident[1], hdr.ident[2], hdr.ident[3]);
        return;
    }
    /* check machine class: */
    if (hdr.ident[ELF_INDEX_CLASS] != ELF_CLASS_32)
    {
        printf("Invalid ELF machine class found. Found: %2x.\n",
            hdr.ident[ELF_INDEX_CLASS]);
        return;
    }

    /* check endianess: */
    if (hdr.ident[ELF_INDEX_DATA] != ELF_DATA_MSB) {
        printf("Non-big endian ELF file detected. Aborting load.\n");
        return;
    }

    if (hdr.ehsize != 52 /* bytes */) {
        printf("Warning: ELF header greater than 52 bytes found. Found: %u\n",
            hdr.ehsize);
    }

    if (hdr.phnum == 0) {
        printf("Found zero segments in ELF file. Aborting load.\n");
        return;
    }

    int i;
    struct elf32_phdr phdr;

    cilo_seek(fp, hdr.phoff, SEEK_SET);

    /* read the PT_LOAD segments into memory at paddr + mem_sz */
    cilo_seek(fp, hdr.phoff, SEEK_SET);
    for (i = 0; i < hdr.phnum; i++) {
        cilo_read(&phdr, sizeof(struct elf32_phdr), 1, fp);

        /* skip unloadable segments */
        if (phdr.type != ELF_PT_LOAD) continue;

        load_elf32_section(fp, phdr.paddr,
            phdr.offset, phdr.filesz);

        mem_sz += phdr.memsz;

        if (phdr.memsz - phdr.filesz > 0) {
            load_elf32_uninitialized_memory(phdr.paddr +
                phdr.filesz, phdr.memsz - phdr.filesz);
        }

        cilo_seek(fp, hdr.phoff + sizeof(struct elf32_phdr) * (i + 1), 
            SEEK_SET);
    }

    /* assume the entry point is the smallest address we're loading */
    printf("Loaded %d bytes.\n", mem_sz);

    printf("Kicking into Linux.\n");

#ifdef DEBUG
    printf("hdr.entry = 0x%08x\n", hdr.entry);
    printf("mem_sz = 0x%08x\n", mem_sz);
#endif

    ((void (*)(uint32_t mem_sz, char *cmd_line))(hdr.entry))
        (c_memsz(), cmd_line);
}

/**
 * load a single ELF section into memory at address. Assumes ELF data is
 * contiguous in memory.
 * @param base location (in memory) of the ELF file
 * @param address address at which the ELF section will be loaded
 * @param file_offset offset (in bytes) in the ELF file where the section is
 * @param length Length of the section (in bytes)
 */
void load_elf64_section(struct file *fp, uint64_t address, 
    uint64_t file_offset, uint64_t length)
{
    uint8_t *elf_loc = (uint8_t *)address;

#ifdef DEBUG
    printf("Init data: %016x length %016x\n", address, length);
#endif

    cilo_seek(fp, (uint32_t)file_offset, SEEK_SET);
    cilo_read(elf_loc, (uint32_t)length, 1, fp); 

}

/**
 * Create an uninitialized data (.bss) region of memory.
 * @param address Start address of this region
 * @param lenght length of this region
 */
void load_elf64_uninitialized_memory(uint64_t address, uint64_t length)
{
    int i = 0;
    uint8_t *p = (uint8_t *)address;

#ifdef DEBUG
    printf("Uninit data: %016x, len %016x\n", address, length); 
#endif

    for (i = 0; i < length; i++) {
        p[i] = 0;
    }
}

void load_elf64_file(struct file *fp, char *cmd_line)
{
    struct elf64_hdr hdr;
    uint32_t mem_sz = 0;

    cilo_seek(fp, 0, SEEK_SET);

    /* read in header entries */
    cilo_read(&hdr, sizeof(struct elf64_hdr), 1, fp);

    /* check the file magic */
    if (hdr.e_ident[0] != ELF_MAGIC_1 || hdr.e_ident[1] != ELF_MAGIC_2 ||
        hdr.e_ident[2] != ELF_MAGIC_3 || hdr.e_ident[3] != ELF_MAGIC_4)
    {
        printf("Bad ELF magic found. Found: %#2x %#2x %#2x %#2x.\n",
            hdr.e_ident[0], hdr.e_ident[1], hdr.e_ident[2], hdr.e_ident[3]);
        return;
    }
    /* check machine class: */
    if (hdr.e_ident[ELF_INDEX_CLASS] != ELF_CLASS_64)
    {
        printf("Invalid ELF machine class found. Found: %2x.\n",
            hdr.e_ident[ELF_INDEX_CLASS]);
        return;
    }

    /* check endianess: */
    if (hdr.e_ident[ELF_INDEX_DATA] != ELF_DATA_MSB) {
        printf("Non-big endian ELF file detected. Aborting load.\n");
        return;
    }

    if (hdr.e_ehsize != 52 /* bytes */) {
        printf("Warning: ELF header greater than 52 bytes found. Found: %u\n",
            hdr.e_ehsize);
    }

    if (hdr.e_phnum == 0) {
        printf("Found zero segments in ELF file. Aborting load.\n");
        return;
    }

    cilo_seek(fp, hdr.e_phoff, SEEK_SET);

    struct elf64_phdr phdr;
    int i;
    for (i = 0; i < hdr.e_phnum; i++) {
        cilo_read(&phdr, sizeof(struct elf64_phdr), 1, fp);

        if (phdr.p_type != ELF_PT_LOAD) continue;
        load_elf64_section(fp, phdr.p_paddr, phdr.p_offset, phdr.p_filesz);
        mem_sz += phdr.p_memsz;

        if (phdr.p_memsz - phdr.p_filesz > 0)
        {
            load_elf64_uninitialized_memory(phdr.p_paddr + phdr.p_filesz, 
                phdr.p_memsz - phdr.p_filesz);
        }
    }



}
