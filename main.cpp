#include <cstdio>
#include <string>
#include <vector>
#include <map>
#include <cmath>

using namespace std;


struct ELF_32_header {
    unsigned char e_ident[16];
    uint16_t type;//
    uint16_t machine;//
    uint32_t version;//
    uint32_t entry; //
    uint32_t phoff; // offset to program header
    uint32_t shoff; //offset to section header
    uint32_t flags; // 0
    uint16_t ehsize; // size of elf header
    uint16_t phentsize; // size of one program header
    uint16_t phnum; //count of program header
    uint16_t shentsize; //size of one section header
    uint16_t shnum; //count of header
    uint16_t shstrndx; //index of shstrheader
};

struct ELF_32_SECTION_HEADER {
    uint32_t sh_name; // index of name = offset of name in shstrtable
    uint32_t sh_type; // type of section
    uint32_t sh_flags; // flags
    uint32_t sh_addr; //
    uint32_t sh_offset; // offset of own section
    uint32_t sh_size; // size of own section
    uint32_t sh_link; // ??
    uint32_t sh_info; // ??
    uint32_t sh_addralign; // выравнивание данной секции
    uint32_t sh_entsize; // хранит размер одной записи, если секция хранит таблицу из записей фиксированного размера
};

struct ELF_32_SYMTAB {
    unsigned int st_name;
    unsigned int st_value;
    unsigned int st_size;
    unsigned char st_info;
    unsigned char st_other;
    unsigned short st_shndx;
};

ELF_32_header elf_header;
vector<ELF_32_SECTION_HEADER> elf_section_header;
vector<ELF_32_SYMTAB> symtab;
string TEXT = ".text";
string SYMTAB = ".symtab";
FILE *f;
FILE *g;
int offset_symtab;
int offset_text;
ELF_32_SECTION_HEADER symtab_header;
ELF_32_SECTION_HEADER text_header;
int count_of_symbols;
map<int, string> val_name;
map<int, string> addr_name;
int addr;
int L_index = 0;


int header_of_need_section(string *name) {
    int index = 0;
    int offset_ofstr = int(elf_section_header[elf_header.shstrndx].sh_offset);
    fseek(f, offset_ofstr, SEEK_SET);
    string cur;
    int current_index = 0;
    while (true) {
        char c;
        fread(&c, sizeof(char), 1, f);
        if (c)
            cur.push_back(c);
        else {
            if (*name == cur) {
                break;
            } else {
                cur.clear();
                current_index = index;
            }
        }
        index++;
    }
    current_index++;
    return current_index;
}

int offset_of_needsection(int index) {
    for (int i = 0; i < elf_header.shnum; i++) {
        if (elf_section_header[i].sh_name == index)
            return int(elf_section_header[i].sh_offset);
    }
    return 0;
}


string get_name(int offset) {
    string s = ".strtab";
    int offset_strtab = offset_of_needsection(header_of_need_section(&s));
    fseek(f, offset_strtab + offset, SEEK_SET);
    string returned;
    while (true) {
        char c;
        fread(&c, sizeof(char), 1, f);
        if (c)
            returned.push_back(c);
        else {
            break;
        }
    }

    return returned;
}

string get_command(int n) {
    if ((n & 0b1111111) == 55)
        return "lui";
    if ((n & 0b1111111) == 23)
        return "auipc";
    if ((n & 0b1111111) == 111)
        return "jal";
    if (((n & 0b1111111) == 103) && (((n & 0b111000000000000) >> 12) == 0))
        return "jalr";
    if (((n & 0b1111111) == 99) && (((n & 0b111000000000000) >> 12) == 0))
        return "bne";
    if (((n & 0b1111111) == 99) && (((n & 0b111000000000000) >> 12) == 1))
        return "bne";
    if (((n & 0b1111111) == 99) && (((n & 0b111000000000000) >> 12) == 4))
        return "blt";
    if (((n & 0b1111111) == 99) && (((n & 0b111000000000000) >> 12) == 5))
        return "bge";
    if (((n & 0b1111111) == 99) && (((n & 0b111000000000000) >> 12) == 6))
        return "bltu";
    if (((n & 0b1111111) == 99) && (((n & 0b111000000000000) >> 12) == 7))
        return "bgeu";
    if (((n & 0b1111111) == 3) && (((n & 0b111000000000000) >> 12) == 0))
        return "lb";
    if (((n & 0b1111111) == 3) && (((n & 0b111000000000000) >> 12) == 1))
        return "lh";
    if (((n & 0b1111111) == 3) && (((n & 0b111000000000000) >> 12) == 2))
        return "lw";
    if (((n & 0b1111111) == 3) && (((n & 0b111000000000000) >> 12) == 4))
        return "lbu";
    if (((n & 0b1111111) == 3) && (((n & 0b111000000000000) >> 12) == 5))
        return "lhu";
    if (((n & 0b1111111) == 35) && (((n & 0b111000000000000) >> 12) == 0))
        return "sb";
    if (((n & 0b1111111) == 35) && (((n & 0b111000000000000) >> 12) == 1))
        return "sh";
    if (((n & 0b1111111) == 35) && (((n & 0b111000000000000) >> 12) == 2))
        return "sw";
    if (((n & 0b1111111) == 19) && (((n & 0b111000000000000) >> 12) == 0))
        return "addi";
    if (((n & 0b1111111) == 19) && (((n & 0b111000000000000) >> 12) == 2))
        return "slti";
    if (((n & 0b1111111) == 19) && (((n & 0b111000000000000) >> 12) == 3))
        return "sltiu";
    if (((n & 0b1111111) == 19) && (((n & 0b111000000000000) >> 12) == 4))
        return "xori";
    if (((n & 0b1111111) == 19) && (((n & 0b111000000000000) >> 12) == 6))
        return "ori";
    if (((n & 0b1111111) == 19) && (((n & 0b111000000000000) >> 12) == 7))
        return "andi";
    if (((n & 0b1111111) == 19) && (((n & 0b111000000000000) >> 12) == 1) &&
        (((n & 0b11111110000000000000000000000000) >> 25) == 0))
        return "slli";
    if (((n & 0b1111111) == 19) && (((n & 0b111000000000000) >> 12) == 5) &&
        (((n & 0b11111110000000000000000000000000) >> 25) == 0))
        return "srli";
    if (((n & 0b1111111) == 19) && (((n & 0b111000000000000) >> 12) == 5) &&
        (((n & 0b11111110000000000000000000000000) >> 25) == 32))
        return "srai";
    if (((n & 0b1111111) == 51) && (((n & 0b111000000000000) >> 12) == 0) &&
        (((n & 0b11111110000000000000000000000000) >> 25) == 0))
        return "add";
    if (((n & 0b1111111) == 51) && (((n & 0b111000000000000) >> 12) == 0) &&
        (((n & 0b11111110000000000000000000000000) >> 25) == 32))
        return "sub";
    if (((n & 0b1111111) == 51) && (((n & 0b111000000000000) >> 12) == 1) &&
        (((n & 0b11111110000000000000000000000000) >> 25) == 0))
        return "sll";
    if (((n & 0b1111111) == 51) && (((n & 0b111000000000000) >> 12) == 2) &&
        (((n & 0b11111110000000000000000000000000) >> 25) == 0))
        return "slt";
    if (((n & 0b1111111) == 51) && (((n & 0b111000000000000) >> 12) == 3) &&
        (((n & 0b11111110000000000000000000000000) >> 25) == 0))
        return "sltu";
    if (((n & 0b1111111) == 51) && (((n & 0b111000000000000) >> 12) == 4) &&
        (((n & 0b11111110000000000000000000000000) >> 25) == 0))
        return "xor";
    if (((n & 0b1111111) == 51) && (((n & 0b111000000000000) >> 12) == 5) &&
        (((n & 0b11111110000000000000000000000000) >> 25) == 0))
        return "srl";
    if (((n & 0b1111111) == 51) && (((n & 0b111000000000000) >> 12) == 5) &&
        (((n & 0b11111110000000000000000000000000) >> 25) == 32))
        return "sra";
    if (((n & 0b1111111) == 51) && (((n & 0b111000000000000) >> 12) == 6) &&
        (((n & 0b11111110000000000000000000000000) >> 25) == 0))
        return "or";
    if (((n & 0b1111111) == 51) && (((n & 0b111000000000000) >> 12) == 7) &&
        (((n & 0b11111110000000000000000000000000) >> 25) == 0))
        return "and";
    if ((n & 0b1111111) == 115 && (((n & 0b11111111111100000000000000000000) >> 20) == 0))
        return "ecall";
    if ((n & 0b1111111) == 115 && (((n & 0b11111111111100000000000000000000) >> 20) == 1))
        return "ebreak";
    if (((n & 0b1111111) == 51) && (((n & 0b111000000000000) >> 12) == 0) &&
        (((n & 0b11111110000000000000000000000000) >> 25) == 1))
        return "mul";
    if (((n & 0b1111111) == 51) && (((n & 0b111000000000000) >> 12) == 1) &&
        (((n & 0b11111110000000000000000000000000) >> 25) == 1))
        return "mulh";
    if (((n & 0b1111111) == 51) && (((n & 0b111000000000000) >> 12) == 2) &&
        (((n & 0b11111110000000000000000000000000) >> 25) == 1))
        return "mulhsu";
    if (((n & 0b1111111) == 51) && (((n & 0b111000000000000) >> 12) == 3) &&
        (((n & 0b11111110000000000000000000000000) >> 25) == 1))
        return "mulhu";
    if (((n & 0b1111111) == 51) && (((n & 0b111000000000000) >> 12) == 4) &&
        (((n & 0b11111110000000000000000000000000) >> 25) == 1))
        return "div";
    if (((n & 0b1111111) == 51) && (((n & 0b111000000000000) >> 12) == 5) &&
        (((n & 0b11111110000000000000000000000000) >> 25) == 1))
        return "divu";
    if (((n & 0b1111111) == 51) && (((n & 0b111000000000000) >> 12) == 6) &&
        (((n & 0b11111110000000000000000000000000) >> 25) == 1))
        return "rem";
    if (((n & 0b1111111) == 51) && (((n & 0b111000000000000) >> 12) == 7) &&
        (((n & 0b11111110000000000000000000000000) >> 25) == 1))
        return "remu";

    return "unknown_instruction";
}

string reg(int n) {
    if (n == 0)
        return "zero";
    if (n == 1)
        return "ra";
    if (n == 2)
        return "sp";
    if (n == 3)
        return "gp";
    if (n == 4)
        return "tp";
    if (n == 5)
        return "t0";
    if (n == 6)
        return "t1";
    if (n == 7)
        return "t2";
    if (n == 8)
        return "fp";
    if (n == 9)
        return "s1";
    if (n == 10)
        return "a0";
    if (n == 11)
        return "a1";
    if (n == 12)
        return "a2";
    if (n == 13)
        return "a3";
    if (n == 14)
        return "a4";
    if (n == 15)
        return "a5";
    if (n == 16)
        return "a6";
    if (n == 17)
        return "a7";
    if (n == 18)
        return "s2";
    if (n == 19)
        return "s3";
    if (n == 20)
        return "s4";
    if (n == 21)
        return "s5";
    if (n == 22)
        return "s6";
    if (n == 23)
        return "s7";
    if (n == 24)
        return "s8";
    if (n == 25)
        return "s9";
    if (n == 26)
        return "s10";
    if (n == 27)
        return "s11";
    if (n == 28)
        return "t3";
    if (n == 29)
        return "t4";
    if (n == 30)
        return "t5";
    if (n == 31)
        return "t6";
    return "invalid";
}

string bind(int n) {
    n = (n) >> 4;
    if (n == 0)
        return "LOCAL";
    if (n == 1)
        return "GLOBAL";
    if (n == 2)
        return "WEAK";
    if (n == 10)
        return "LOOS";
    if (n == 12)
        return "HIOS";
    if (n == 13)
        return "LOPROC";
    if (n == 15)
        return "HIPROC";
    return "invalid";
}

string index(int n) {
    if (n == 0)
        return "UNDEF";
    if (n == 65280)
        return "LORESERVE";
    if (n == 65281)
        return "AFTER";
    if (n == 65311)
        return "HIPROC";
    if (n == 65312)
        return "LOOS";
    if (n == 65343)
        return "HIOS";
    if (n == 65521)
        return "ABS";
    if (n == 65522)
        return "COMMON";
    if (n == 65535)
        return "XINDEX";

    return to_string(n);
}

string vis(int n) {
    if (n == 0)
        return "DEFAULT";
    if (n == 1)
        return "INTERNAL";
    if (n == 2)
        return "HIDDEN";
    if (n == 3)
        return "PROTECTED";
    if (n == 4)
        return "EXPORTED";
    if (n == 5)
        return "SINGLETON";
    if (n == 6)
        return "ELIMINATE";
    return "invalid";
}

string type(int n) {
    n = (n) & 0xf;
    if (n == 0)
        return "NOTYPE";
    if (n == 1)
        return "OBJECT";
    if (n == 2)
        return "FUNC";
    if (n == 3)
        return "SECTION";
    if (n == 4)
        return "FILE";
    if (n == 5)
        return "COMMON";
    if (n == 6)
        return "TLS";
    if (n == 10)
        return "LOOS";
    if (n == 12)
        return "HIOS";
    if (n == 13)
        return "LOPROC";
    if (n == 15)
        return "HIPROC";
    return "invalid";
}

int add_to_two(int n, int size) {
    if (n < pow(2, size - 1))
        return n;
    return int(n - pow(2, size));

}

int main(int argc, char *argv[]) {

    f = fopen(argv[1], "r");
    g = fopen(argv[2], "w");
    if(!f){
        printf("input file is not available");
        return 0;
    }
    if(!g){
        printf("output file is not available");
        return 0;
    }

    fread(&elf_header, sizeof(ELF_32_header), 1, f);
    if(elf_header.version!=1){
        printf("invalid file version");
        return 0;
    }

    //читаем заголовки
    fseek(f, int(elf_header.shoff), SEEK_SET);

    for (int i = 0; i < elf_header.shnum; i++) {
        ELF_32_SECTION_HEADER cur{};
        fread(&cur, sizeof(ELF_32_SECTION_HEADER), 1, f);
        elf_section_header.push_back(cur);
    }

    for (int i = 0; i < elf_header.shnum; i++) {
        if (elf_section_header[i].sh_name == header_of_need_section(&SYMTAB)) {
            symtab_header = elf_section_header[i];
            break;
        }
    }
    for (int i = 0; i < elf_header.shnum; i++) {
        if (elf_section_header[i].sh_name == header_of_need_section(&TEXT)) {
            text_header = elf_section_header[i];
            break;
        }
    }
    addr = int(text_header.sh_addr);
    count_of_symbols = int(symtab_header.sh_size) / int(sizeof(ELF_32_SYMTAB));
    offset_symtab = offset_of_needsection(header_of_need_section(&SYMTAB));

    fseek(f, offset_symtab, SEEK_SET);

    for (int i = 0; i < count_of_symbols; i++) {
        ELF_32_SYMTAB cur{};
        fseek(f, offset_symtab + 16 * i, SEEK_SET);
        fread(&cur, sizeof(ELF_32_SYMTAB), 1, f);
        symtab.push_back(cur);
        string name = get_name(int(symtab[i].st_name));
        string type_ = type(cur.st_info);
        int value = int(symtab[i].st_value);
        if (type_ == "OBJECT" || type_ == "FUNC") {
            val_name[value] = name;
        }
    }

    offset_text = offset_of_needsection(header_of_need_section(&TEXT));
    fseek(f, offset_text, SEEK_SET);

    for (int i = 0; i < text_header.sh_size / 4; i++) {
        fseek(f, offset_text + i * 4, SEEK_SET);
        int cur = 0;
        fread(&cur, 4, 1, f);
        string command = get_command(cur);
        int imm;
        if (command == "jal") {
            uint32_t immp1, immp2, immp3, immp4;
            immp1 = ((cur & (uint32_t) 0b10000000000000000000000000000000) >> 31) << 1;
            immp2 = ((cur & (uint32_t) 0b00000000000011111111000000000000) >> 12) << 1;
            immp3 = ((cur & (uint32_t) 0b00000000000100000000000000000000) >> 20) << 1;
            immp4 = ((cur & (uint32_t) 0b01111111111000000000000000000000) >> 21) << 1;
            imm = int(immp4 + immp3 * 1024 + immp2 * 2048 + immp1 * pow(2, 19));

            imm = add_to_two(int(imm), 20);
            imm += addr;
            imm = add_to_two(int(imm), 20);

            if (val_name.find(int(imm)) != val_name.end())
                addr_name[int(imm)] = val_name[int(imm)];
            else {
                string s = "L";
                s += (to_string(L_index));
                addr_name[int(imm)] = s;
                L_index++;
            }

        } else if (command == "beq" || command == "bne"
                   || command == "blt" || command == "bgeu"
                   || command == "bge" || command == "bltu") { //1 111111 01010 01111 001 0100 1


            uint32_t immp1, immp2, immp3, immp4;
            immp1 = ((cur & (uint32_t) 0b10000000000000000000000000000000) >> 31) << 1;
            immp2 = ((cur & (uint32_t) 0b00000000000000000000000010000000) >> 7) << 1;
            immp3 = ((cur & (uint32_t) 0b01111110000000000000000000000000) >> 25) << 1;
            immp4 = ((cur & (uint32_t) 0b00000000000000000000111100000000) >> 8) << 1;
            imm = immp1 * pow(2, 11) + immp2 * pow(2, 10) + immp3 * 16 + immp4;
            imm = add_to_two(int(imm), 12);
            imm += addr;
            imm = add_to_two(int(imm), 12);
            if (val_name.find(int(imm)) != val_name.end())
                addr_name[int(imm)] = val_name[int(imm)];
            else {
                string s = "L";
                s += (to_string(L_index));
                addr_name[int(imm)] = s;
                L_index++;
            }

        }

        addr += 4;

    }

    addr = int(text_header.sh_addr);

    for (int i = 0; i < text_header.sh_size / 4; i++) {
        if (addr_name.find(int(addr)) != addr_name.end()) {
            const char *adname = addr_name[addr].c_str();
            fprintf(g, "%08x <%s>:\n", addr, adname);
        }

        fseek(f, offset_text + i * 4, SEEK_SET);
        int cur = 0;
        fread(&cur, 4, 1, f);
        string command = get_command(cur);
        const char *com = command.c_str();
        fprintf(g, "%05x:\t", addr);

        if (command == "lui" || command == "auipc") {
            int rg = (cur & 0b111110000000) >> 7;
            string srd = reg(rg);
            if(srd=="invalid"){
                printf("invalid reg");
                return 0;
            }
            const char *rd = srd.c_str();
            int imm = add_to_two(int((cur & 0b11111111111111111111000000000000) >> 12), 20);
            fprintf(g, "%08x\t%7s\t%s,0x%x\n", cur, com, rd, imm);
        } else if (command == "jal") {
            int rg = (cur & 0b111110000000) >> 7;
            string srg = reg(rg);
            if(srg=="invalid"){
                printf("invalid reg");
                return 0;
            }

            const char *rd = srg.c_str();

            uint32_t immp1, immp2, immp3, immp4, imm;
            immp1 = ((cur & (uint32_t) 0b10000000000000000000000000000000) >> 31) << 1;
            immp2 = ((cur & (uint32_t) 0b00000000000011111111000000000000) >> 12) << 1;
            immp3 = ((cur & (uint32_t) 0b00000000000100000000000000000000) >> 20) << 1;
            immp4 = ((cur & (uint32_t) 0b01111111111000000000000000000000) >> 21) << 1;
            imm = int(immp4 + immp3 * 1024 + immp2 * 2048 + immp1 * pow(2, 19));

            imm = add_to_two(int(imm), 20);
            imm += addr;
            imm = add_to_two(int(imm), 20);

            const char *lable = addr_name[int(imm)].c_str();

            fprintf(g, "%08x\t%7s\t%s, 0x%x <%s>\n", cur, com, rd, imm, lable);

        } else if (command == "jalr" || command == "lb" || command == "lh"
                   || command == "lbu" || command == "lw"
                   || command == "lhu") {
            int rgd = (cur & 0b111110000000) >> 7;
            int rgs1 = (cur & 0b11111000000000000000) >> 15;
            string imm = to_string(add_to_two(int((cur & 0b11111111111100000000000000000000) >> 20), 12));
            string srgs1 = reg(rgs1);
            string srgs2 = reg(rgd);
            if(srgs1=="invalid" || srgs2=="invalid"){
                printf("invalid reg");
                return 0;
            }

            const char *rs1 = srgs1.c_str();
            const char *rd = srgs2.c_str();
            const char *imm1 = imm.c_str();

            fprintf(g, "%08x\t%7s\t%s, %s(%s)\n", cur, com, rd, imm1, rs1);
        } else if (command == "beq" || command == "bne"
                   || command == "blt" || command == "bgeu"
                   || command == "bge" || command == "bltu") { //1 111111 01010 01111 001 0100 1

            int rgs2 = (cur & 0b1111100000000000000000000) >> 20;
            int rgs1 = (cur & 0b11111000000000000000) >> 15;
            string srs1 = reg(rgs1);
            const char *rs1 = srs1.c_str();
            string srs2 = reg(rgs2);
            const char *rs2 = srs2.c_str();
            if(srs1=="invalid" || srs2=="invalid"){
                printf("invalid reg");
                return 0;
            }


            uint32_t immp1, immp2, immp3, immp4, imm;
            immp1 = ((cur & (uint32_t) 0b10000000000000000000000000000000) >> 31) << 1;
            immp2 = ((cur & (uint32_t) 0b00000000000000000000000010000000) >> 7) << 1;
            immp3 = ((cur & (uint32_t) 0b01111110000000000000000000000000) >> 25) << 1;
            immp4 = ((cur & (uint32_t) 0b00000000000000000000111100000000) >> 8) << 1;
            imm = immp1 * pow(2, 11) + immp2 * pow(2, 10) + immp3 * 16 + immp4;
            imm = add_to_two(int(imm), 12);
            imm += addr;
            imm = add_to_two(int(imm), 12);

            const char *lable = addr_name[int(imm)].c_str();

            fprintf(g, "%08x\t%7s\t%s, %s, 0x%x <%s>\n", cur, com, rs1, rs2, imm, lable);

        } else if (command == "sh" || command == "sb" || command == "sw") {
            int rgs2 = (cur & 0b1111100000000000000000000) >> 20;
            int rgs1 = (cur & 0b11111000000000000000) >> 15;
            string srs1 = reg(rgs1);
            string srs2 = reg(rgs2);
            if(srs1=="invalid" || srs2=="invalid"){
                printf("invalid reg");
                return 0;
            }

            const char *rs1 = srs1.c_str();
            const char *rs2 = srs2.c_str();
            uint32_t immp1, immp2, imm;
            immp1 = ((cur & (uint32_t) 0b11111110000000000000000000000000) >> 25);
            immp2 = ((cur & (uint32_t) 0b00000000000000000000111110000000) >> 7);

            imm = immp1 * 32 + immp2;

            fprintf(g, "%08x\t%7s\t%s, %d(%s)\n", cur, com, rs2, imm, rs1);


        } else if (command == "slti" || command == "addi"
                   || command == "xori" || command == "sltiu"
                   || command == "ori" || command == "andi" || command == "slli" || command == "srli" ||
                   command == "srai") {
            int rgd = (cur & 0b111110000000) >> 7;
            int rgs1 = (cur & 0b11111000000000000000) >> 15;
            string imm = to_string(add_to_two(int((cur & 0b11111111111100000000000000000000) >> 20), 12));
            string srs = reg(rgs1);
            string srd = reg(rgd);
            if(srs=="invalid" || srd=="invalid"){
                printf("invalid reg");
                return 0;
            }

            const char *rs1 = srs.c_str();
            const char *rd = srd.c_str();
            const char *imm1 = imm.c_str();
            fprintf(g, "%08x\t%7s\t%s, %s, %s\n", cur, com, rd, rs1, imm1);
        } else if (command == "add" || command == "sub" || command == "sll" ||
                   command == "slt" || command == "sltu" || command == "xor" ||
                   command == "srl" || command == "sra" || command == "or" || command == "and" ||
                   command == "mul" || command == "mulh" || command == "mulhsu" ||
                   command == "divu" || command == "div" || command == "mulhu" ||
                   command == "rem" || command == "remu") {

            int rgs2 = (cur & 0b1111100000000000000000000) >> 20;
            int rgd = (cur & 0b111110000000) >> 7;
            int rgs1 = (cur & 0b11111000000000000000) >> 15;
            string srs1 = reg(rgs1);
            string srd = reg(rgd);
            string srs2 = reg(rgs2);
            if(srs1=="invalid" || srd=="invalid" || srs2=="invalid"){
                printf("invalid reg");
                return 0;
            }

            const char *rs1 = srs1.c_str();
            const char *rd = srd.c_str();
            const char *rs2 = srs2.c_str();
            fprintf(g, "%08x\t%7s\t%s, %s, %s\n", cur, com, rd, rs1, rs2);

        } else if (command == "ecall" || command == "ebreak") {
            fprintf(g, "%08x\t%7s\n", cur, com);
        } else {
            fprintf(g, "%08x\tunknown_instruction\n", cur);
        }
        addr += 4;
    }
    fprintf(g, "\nSymbol Value              Size Type \tBind     Vis       Index Name\n");
    for (int i = 0; i < count_of_symbols; i++) {
        ELF_32_SYMTAB cur{};
        fseek(f, offset_symtab + sizeof(ELF_32_SYMTAB) * i, SEEK_SET);
        fread(&cur, sizeof(ELF_32_SYMTAB), 1, f);
        string sname = get_name(int(symtab[i].st_name));
        const char *name = sname.c_str();
        string stype = type(cur.st_info);
        const char *type_ = stype.c_str();
        string sbind = bind(cur.st_info);
        const char *bind_ = sbind.c_str();
        string svis = vis(cur.st_other);
        const char *vis_ = svis.c_str();
        string sindex = index(cur.st_shndx);
        const char *index_ = sindex.c_str();

        int value = int(symtab[i].st_value);
        int size = int(symtab[i].st_size);
        if(stype == "invalid" || sbind == "invalid" || svis=="invalid" || sindex == "invalid"){
            printf("invalid symtab");
            return 0;
        }
        fprintf(g, "[%4i] 0x%-15X %5i %-8s %-8s %-8s %6s %s\n", i, value, size, type_, bind_, vis_, index_, name);
    }

    fclose(f);
    fclose(g);
}
