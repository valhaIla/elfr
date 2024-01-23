#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <elf.h>

void display_symbol_name(Elf64_Sym *sym_table, char *str_table, int num_symbols) {
    printf("Symbol table:\n");
    printf("%-10s %-20s %-10s %-10s %-10s %-10s\n", "Index", "Name", "Value", "Size", "Binding", "Type");
    
    for (int i = 0; i < num_symbols; i++) {
        printf("%-10d %-20s %-10lx %-10ld %-10d %-10d\n", i, &str_table[sym_table[i].st_name],
               sym_table[i].st_value, sym_table[i].st_size, ELF64_ST_BIND(sym_table[i].st_info),
               ELF64_ST_TYPE(sym_table[i].st_info));
    }
}

void analyze_elf_file(const char *file_path) {
    int fd = open(file_path, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return;
    }
    
    off_t file_size = lseek(fd, 0, SEEK_END);
    if (file_size == -1) {
        perror("Error retrieving file size");
        close(fd);
        return;
    }
    
    void *file_data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (file_data == MAP_FAILED) {
        perror("Error mapping file into memory");
        close(fd);
        return;
    }
    
    Elf64_Ehdr *elf_header = (Elf64_Ehdr *)file_data;
    Elf64_Shdr *section_header_table = (Elf64_Shdr *)(file_data + elf_header->e_shoff);
    Elf64_Shdr *section_string_table = &section_header_table[elf_header->e_shstrndx];
    char *string_table = (char *)(file_data + section_string_table->sh_offset);
    
    Elf64_Shdr *symbol_table_section = NULL;
    int num_symbols = 0;
    
    for (int i = 0; i < elf_header->e_shnum; i++) {
        if (section_header_table[i].sh_type == SHT_SYMTAB) {
            symbol_table_section = &section_header_table[i];
            num_symbols = symbol_table_section->sh_size / sizeof(Elf64_Sym);
            break;
        }
    }
    
    if (symbol_table_section == NULL) {
        printf("No symbol table found in the ELF file.\n");
    } else {
        Elf64_Sym *symbol_table = (Elf64_Sym *)(file_data + symbol_table_section->sh_offset);
        display_symbol_name(symbol_table, string_table, num_symbols);
    }
    
    munmap(file_data, file_size);
    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <elf_file>\n", argv[0]);
        return 1;
    }
    
    analyze_elf_file(argv[1]);
    
    return 0;
}
