/*
 * x86_64-cb/bootboot.c
 *
 * Copyright (C) 2017 - 2021 bzt (bztsrc@gitlab)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This file is part of the BOOTBOOT Protocol package.
 * @brief Boot loader for the x86_64 coreboot payload
 *
 * Memory map
 *      0h -  1000h reserved by coreboot
 *   1000h -  2000h SMP trampoline
 *   2000h -  3000h bootboot structure
 *   3000h -  4000h environment
 *   4000h - 29000h paging tables
 *  29000h - 69000h core stacks, 1k for each core
 *  69000h - A0000h free memory
 *  A0000h - F0000h reserved by coreboot
 *  F0000h - FFFF0h special IMD records, see arch_write_tables() in coreboot/src/arch/x86/tables.c
 * 100000h -300000h libpayload, this code, and heap
 * 300000h -      ? initrd
 */

#define BBDEBUG 1

//#define BBNOIDEFALLBACK 1

#include <libpayload-config.h>
#include <libpayload.h>
#if IS_ENABLED(CONFIG_LP_CBFS)
# include <cbfs.h>
#endif
#if IS_ENABLED(CONFIG_LP_STORAGE)
# include <storage/storage.h>
#endif
#if IS_ENABLED(CONFIG_LP_USB)
# include <usb/usb.h>
# include <usb/usbmsc.h>
#endif
#include "tinf.h"
#include "../dist/bootboot.h"

#define PAGESIZE 4096

#if BBDEBUG
#define DBG(fmt, ...) do{printf(fmt,__VA_ARGS__); }while(0);
#else
#define DBG(fmt, ...)
#endif

#define INITRD_BASE ((CONFIG_LP_BASE_ADDRESS + 1024*1024 + CONFIG_LP_HEAP_SIZE + PAGESIZE-1) & ~(PAGESIZE-1))

extern void ap_trampoline();
#if __WORDSIZE == 64
extern void bsp64_init(uint64_t apicid);
#else
extern void bsp_init();
#endif

#define send_ipi(a,m,v) do { \
    while(*((volatile uint32_t*)((uintptr_t)lapic_addr + 0x300)) & (1 << 12)) __asm__ __volatile__ ("pause" : : : "memory"); \
    *((volatile uint32_t*)((uintptr_t)lapic_addr + 0x310)) = (*((volatile uint32_t*)((uintptr_t)lapic_addr + 0x310)) & \
        0x00ffffff) | (a << 24); \
    *((volatile uint32_t*)((uintptr_t)lapic_addr + 0x300)) = (*((volatile uint32_t*)((uintptr_t)lapic_addr + 0x300)) & m) | v;  \
} while(0)

/*** ELF64 defines and structs ***/
#define ELFMAG      "\177ELF"
#define SELFMAG     4
#define EI_CLASS    4       /* File class byte index */
#define ELFCLASS64  2       /* 64-bit objects */
#define EI_DATA     5       /* Data encoding byte index */
#define ELFDATA2LSB 1       /* 2's complement, little endian */
#define PT_LOAD     1       /* Loadable program segment */
#define EM_X86_64   62      /* AMD x86-64 architecture */

typedef struct
{
  unsigned char e_ident[16];/* Magic number and other info */
  uint16_t    e_type;         /* Object file type */
  uint16_t    e_machine;      /* Architecture */
  uint32_t    e_version;      /* Object file version */
  uint64_t    e_entry;        /* Entry point virtual address */
  uint64_t    e_phoff;        /* Program header table file offset */
  uint64_t    e_shoff;        /* Section header table file offset */
  uint32_t    e_flags;        /* Processor-specific flags */
  uint16_t    e_ehsize;       /* ELF header size in bytes */
  uint16_t    e_phentsize;    /* Program header table entry size */
  uint16_t    e_phnum;        /* Program header table entry count */
  uint16_t    e_shentsize;    /* Section header table entry size */
  uint16_t    e_shnum;        /* Section header table entry count */
  uint16_t    e_shstrndx;     /* Section header string table index */
} Elf64_Ehdr;

typedef struct
{
  uint32_t    p_type;         /* Segment type */
  uint32_t    p_flags;        /* Segment flags */
  uint64_t    p_offset;       /* Segment file offset */
  uint64_t    p_vaddr;        /* Segment virtual address */
  uint64_t    p_paddr;        /* Segment physical address */
  uint64_t    p_filesz;       /* Segment size in file */
  uint64_t    p_memsz;        /* Segment size in memory */
  uint64_t    p_align;        /* Segment alignment */
} Elf64_Phdr;

typedef struct
{
  uint32_t    sh_name;        /* Section name (string tbl index) */
  uint32_t    sh_type;        /* Section type */
  uint64_t    sh_flags;       /* Section flags */
  uint64_t    sh_addr;        /* Section virtual addr at execution */
  uint64_t    sh_offset;      /* Section file offset */
  uint64_t    sh_size;        /* Section size in bytes */
  uint32_t    sh_link;        /* Link to another section */
  uint32_t    sh_info;        /* Additional section information */
  uint64_t    sh_addralign;   /* Section alignment */
  uint64_t    sh_entsize;     /* Entry size if section holds table */
} Elf64_Shdr;

typedef struct
{
  uint32_t    st_name;        /* Symbol name (string tbl index) */
  uint8_t     st_info;        /* Symbol type and binding */
  uint8_t     st_other;       /* Symbol visibility */
  uint16_t    st_shndx;       /* Section index */
  uint64_t    st_value;       /* Symbol value */
  uint64_t    st_size;        /* Symbol size */
} Elf64_Sym;

/*** PE32+ defines and structs ***/
#define MZ_MAGIC                    0x5a4d      /* "MZ" */
#define PE_MAGIC                    0x00004550  /* "PE\0\0" */
#define IMAGE_FILE_MACHINE_AMD64    0x8664      /* AMD x86_64 architecture */
#define PE_OPT_MAGIC_PE32PLUS       0x020b      /* PE32+ format */
typedef struct
{
  uint16_t magic;         /* MZ magic */
  uint16_t reserved[29];  /* reserved */
  uint32_t peaddr;        /* address of pe header */
} mz_hdr;

typedef struct {
  uint32_t magic;         /* PE magic */
  uint16_t machine;       /* machine type */
  uint16_t sections;      /* number of sections */
  uint32_t timestamp;     /* time_t */
  uint32_t sym_table;     /* symbol table offset */
  uint32_t numsym;        /* number of symbols */
  uint16_t opt_hdr_size;  /* size of optional header */
  uint16_t flags;         /* flags */
  uint16_t file_type;     /* file type, PE32PLUS magic */
  uint8_t  ld_major;      /* linker major version */
  uint8_t  ld_minor;      /* linker minor version */
  uint32_t text_size;     /* size of text section(s) */
  uint32_t data_size;     /* size of data section(s) */
  uint32_t bss_size;      /* size of bss section(s) */
  int32_t entry_point;    /* file offset of entry point */
  int32_t code_base;      /* relative code addr in ram */
} pe_hdr;

typedef struct {
  uint32_t iszero;        /* if this is not zero, then iszero+nameoffs gives UTF-8 string */
  uint32_t nameoffs;
  int32_t value;          /* value of the symbol */
  uint16_t section;       /* section it belongs to */
  uint16_t type;          /* symbol type */
  uint8_t storclass;      /* storage class */
  uint8_t auxsyms;        /* number of pe_sym records following */
} pe_sym;

/*** other defines and structs ***/
typedef struct {
    uint32_t type[4];
    uint8_t  uuid[16];
    uint64_t start;
    uint64_t end;
    uint64_t flags;
    uint8_t  name[72];
} efipart_t;

typedef struct {
    char        jmp[3];
    char        oem[8];
    uint16_t    bps;
    uint8_t     spc;
    uint16_t    rsc;
    uint8_t     nf;
    uint8_t     nr0;
    uint8_t     nr1;
    uint16_t    ts16;
    uint8_t     media;
    uint16_t    spf16;
    uint16_t    spt;
    uint16_t    nh;
    uint32_t    hs;
    uint32_t    ts32;
    uint32_t    spf32;
    uint32_t    flg;
    uint32_t    rc;
    char        vol[6];
    char        fst[8];
    char        dmy[20];
    char        fst2[8];
} __attribute__((packed)) bpb_t;

typedef struct {
    char        name[8];
    char        ext[3];
    char        attr[9];
    uint16_t    ch;
    uint32_t    attr2;
    uint16_t    cl;
    uint32_t    size;
} __attribute__((packed)) fatdir_t;

/**
 * return type for fs drivers
 */
typedef struct {
    uint8_t *ptr;
    uint32_t size;
} file_t;

/*** common variables ***/
extern struct sysinfo_t lib_sysinfo;
file_t initrd;      // initrd file descriptor
file_t core;        // kernel file descriptor
BOOTBOOT *bootboot = (BOOTBOOT*)0x2000;
char *environment = (char*)0x3000;
uint64_t *paging = (uint64_t*)0x4000;

int reqwidth = 1024, reqheight = 768;
char *kernelname="sys/core";
unsigned char *kne, nosmp=0;

// alternative environment name
char *cfgname="sys/config";

uint64_t mm_addr = BOOTBOOT_MMIO;   // virtual addresses
uint64_t fb_addr = BOOTBOOT_FB;
uint64_t bb_addr = BOOTBOOT_INFO;
uint64_t env_addr= BOOTBOOT_ENV;
uint64_t core_addr=BOOTBOOT_CORE;

uint64_t entrypoint=0, lapic_addr=0, initstack = 1024;
uint16_t lapic_ids[1024];

/**
 * SHA-256
 */
typedef struct {
   uint8_t d[64];
   uint32_t l;
   uint32_t b[2];
   uint32_t s[8];
} SHA256_CTX;
#define SHA_ADD(a,b,c) if(a>0xffffffff-(c))b++;a+=c;
#define SHA_ROTL(a,b) (((a)<<(b))|((a)>>(32-(b))))
#define SHA_ROTR(a,b) (((a)>>(b))|((a)<<(32-(b))))
#define SHA_CH(x,y,z) (((x)&(y))^(~(x)&(z)))
#define SHA_MAJ(x,y,z) (((x)&(y))^((x)&(z))^((y)&(z)))
#define SHA_EP0(x) (SHA_ROTR(x,2)^SHA_ROTR(x,13)^SHA_ROTR(x,22))
#define SHA_EP1(x) (SHA_ROTR(x,6)^SHA_ROTR(x,11)^SHA_ROTR(x,25))
#define SHA_SIG0(x) (SHA_ROTR(x,7)^SHA_ROTR(x,18)^((x)>>3))
#define SHA_SIG1(x) (SHA_ROTR(x,17)^SHA_ROTR(x,19)^((x)>>10))
static uint32_t sha256_k[64]={
   0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
   0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
   0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
   0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
   0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
   0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
   0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
   0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};
void sha256_t(SHA256_CTX *ctx)
{
   uint32_t a,b,c,d,e,f,g,h,i,j,t1,t2,m[64];
   for(i=0,j=0;i<16;i++,j+=4) m[i]=(ctx->d[j]<<24)|(ctx->d[j+1]<<16)|(ctx->d[j+2]<<8)|(ctx->d[j+3]);
   for(;i<64;i++) m[i]=SHA_SIG1(m[i-2])+m[i-7]+SHA_SIG0(m[i-15])+m[i-16];
   a=ctx->s[0];b=ctx->s[1];c=ctx->s[2];d=ctx->s[3];
   e=ctx->s[4];f=ctx->s[5];g=ctx->s[6];h=ctx->s[7];
   for(i=0;i<64;i++) {
       t1=h+SHA_EP1(e)+SHA_CH(e,f,g)+sha256_k[i]+m[i];
       t2=SHA_EP0(a)+SHA_MAJ(a,b,c);h=g;g=f;f=e;e=d+t1;d=c;c=b;b=a;a=t1+t2;
    }
   ctx->s[0]+=a;ctx->s[1]+=b;ctx->s[2]+=c;ctx->s[3]+=d;
   ctx->s[4]+=e;ctx->s[5]+=f;ctx->s[6]+=g;ctx->s[7]+=h;
}
void SHA256_Init(SHA256_CTX *ctx)
{
    ctx->l=0;ctx->b[0]=ctx->b[1]=0;
    ctx->s[0]=0x6a09e667;ctx->s[1]=0xbb67ae85;ctx->s[2]=0x3c6ef372;ctx->s[3]=0xa54ff53a;
    ctx->s[4]=0x510e527f;ctx->s[5]=0x9b05688c;ctx->s[6]=0x1f83d9ab;ctx->s[7]=0x5be0cd19;
}
void SHA256_Update(SHA256_CTX *ctx, const void *data, int len)
{
    uint8_t *d=(uint8_t *)data;
    for(;len--;d++) {
        ctx->d[ctx->l++]=*d;
        if(ctx->l==64) {sha256_t(ctx);SHA_ADD(ctx->b[0],ctx->b[1],512);ctx->l=0;}
    }
}
void SHA256_Final(unsigned char *h, SHA256_CTX *ctx)
{
    uint32_t i=ctx->l;
    ctx->d[i++]=0x80;
    if(ctx->l<56) {while(i<56) ctx->d[i++]=0x00;}
    else {while(i<64) ctx->d[i++]=0x00;sha256_t(ctx);memset(ctx->d,0,56);}
    SHA_ADD(ctx->b[0],ctx->b[1],ctx->l*8);
    ctx->d[63]=ctx->b[0];ctx->d[62]=ctx->b[0]>>8;ctx->d[61]=ctx->b[0]>>16;ctx->d[60]=ctx->b[0]>>24;
    ctx->d[59]=ctx->b[1];ctx->d[58]=ctx->b[1]>>8;ctx->d[57]=ctx->b[1]>>16;ctx->d[56]=ctx->b[1]>>24;
    sha256_t(ctx);
    for(i=0;i<4;i++) {
        h[i]   =(ctx->s[0]>>(24-i*8)); h[i+4] =(ctx->s[1]>>(24-i*8));
        h[i+8] =(ctx->s[2]>>(24-i*8)); h[i+12]=(ctx->s[3]>>(24-i*8));
        h[i+16]=(ctx->s[4]>>(24-i*8)); h[i+20]=(ctx->s[5]>>(24-i*8));
        h[i+24]=(ctx->s[6]>>(24-i*8)); h[i+28]=(ctx->s[7]>>(24-i*8));
    }
}

/**
 * precalculated CRC32c lookup table for polynomial 0x1EDC6F41 (castagnoli-crc)
 */
uint32_t crc32c_lookup[256]={
    0x00000000L, 0xF26B8303L, 0xE13B70F7L, 0x1350F3F4L, 0xC79A971FL, 0x35F1141CL, 0x26A1E7E8L, 0xD4CA64EBL,
    0x8AD958CFL, 0x78B2DBCCL, 0x6BE22838L, 0x9989AB3BL, 0x4D43CFD0L, 0xBF284CD3L, 0xAC78BF27L, 0x5E133C24L,
    0x105EC76FL, 0xE235446CL, 0xF165B798L, 0x030E349BL, 0xD7C45070L, 0x25AFD373L, 0x36FF2087L, 0xC494A384L,
    0x9A879FA0L, 0x68EC1CA3L, 0x7BBCEF57L, 0x89D76C54L, 0x5D1D08BFL, 0xAF768BBCL, 0xBC267848L, 0x4E4DFB4BL,
    0x20BD8EDEL, 0xD2D60DDDL, 0xC186FE29L, 0x33ED7D2AL, 0xE72719C1L, 0x154C9AC2L, 0x061C6936L, 0xF477EA35L,
    0xAA64D611L, 0x580F5512L, 0x4B5FA6E6L, 0xB93425E5L, 0x6DFE410EL, 0x9F95C20DL, 0x8CC531F9L, 0x7EAEB2FAL,
    0x30E349B1L, 0xC288CAB2L, 0xD1D83946L, 0x23B3BA45L, 0xF779DEAEL, 0x05125DADL, 0x1642AE59L, 0xE4292D5AL,
    0xBA3A117EL, 0x4851927DL, 0x5B016189L, 0xA96AE28AL, 0x7DA08661L, 0x8FCB0562L, 0x9C9BF696L, 0x6EF07595L,
    0x417B1DBCL, 0xB3109EBFL, 0xA0406D4BL, 0x522BEE48L, 0x86E18AA3L, 0x748A09A0L, 0x67DAFA54L, 0x95B17957L,
    0xCBA24573L, 0x39C9C670L, 0x2A993584L, 0xD8F2B687L, 0x0C38D26CL, 0xFE53516FL, 0xED03A29BL, 0x1F682198L,
    0x5125DAD3L, 0xA34E59D0L, 0xB01EAA24L, 0x42752927L, 0x96BF4DCCL, 0x64D4CECFL, 0x77843D3BL, 0x85EFBE38L,
    0xDBFC821CL, 0x2997011FL, 0x3AC7F2EBL, 0xC8AC71E8L, 0x1C661503L, 0xEE0D9600L, 0xFD5D65F4L, 0x0F36E6F7L,
    0x61C69362L, 0x93AD1061L, 0x80FDE395L, 0x72966096L, 0xA65C047DL, 0x5437877EL, 0x4767748AL, 0xB50CF789L,
    0xEB1FCBADL, 0x197448AEL, 0x0A24BB5AL, 0xF84F3859L, 0x2C855CB2L, 0xDEEEDFB1L, 0xCDBE2C45L, 0x3FD5AF46L,
    0x7198540DL, 0x83F3D70EL, 0x90A324FAL, 0x62C8A7F9L, 0xB602C312L, 0x44694011L, 0x5739B3E5L, 0xA55230E6L,
    0xFB410CC2L, 0x092A8FC1L, 0x1A7A7C35L, 0xE811FF36L, 0x3CDB9BDDL, 0xCEB018DEL, 0xDDE0EB2AL, 0x2F8B6829L,
    0x82F63B78L, 0x709DB87BL, 0x63CD4B8FL, 0x91A6C88CL, 0x456CAC67L, 0xB7072F64L, 0xA457DC90L, 0x563C5F93L,
    0x082F63B7L, 0xFA44E0B4L, 0xE9141340L, 0x1B7F9043L, 0xCFB5F4A8L, 0x3DDE77ABL, 0x2E8E845FL, 0xDCE5075CL,
    0x92A8FC17L, 0x60C37F14L, 0x73938CE0L, 0x81F80FE3L, 0x55326B08L, 0xA759E80BL, 0xB4091BFFL, 0x466298FCL,
    0x1871A4D8L, 0xEA1A27DBL, 0xF94AD42FL, 0x0B21572CL, 0xDFEB33C7L, 0x2D80B0C4L, 0x3ED04330L, 0xCCBBC033L,
    0xA24BB5A6L, 0x502036A5L, 0x4370C551L, 0xB11B4652L, 0x65D122B9L, 0x97BAA1BAL, 0x84EA524EL, 0x7681D14DL,
    0x2892ED69L, 0xDAF96E6AL, 0xC9A99D9EL, 0x3BC21E9DL, 0xEF087A76L, 0x1D63F975L, 0x0E330A81L, 0xFC588982L,
    0xB21572C9L, 0x407EF1CAL, 0x532E023EL, 0xA145813DL, 0x758FE5D6L, 0x87E466D5L, 0x94B49521L, 0x66DF1622L,
    0x38CC2A06L, 0xCAA7A905L, 0xD9F75AF1L, 0x2B9CD9F2L, 0xFF56BD19L, 0x0D3D3E1AL, 0x1E6DCDEEL, 0xEC064EEDL,
    0xC38D26C4L, 0x31E6A5C7L, 0x22B65633L, 0xD0DDD530L, 0x0417B1DBL, 0xF67C32D8L, 0xE52CC12CL, 0x1747422FL,
    0x49547E0BL, 0xBB3FFD08L, 0xA86F0EFCL, 0x5A048DFFL, 0x8ECEE914L, 0x7CA56A17L, 0x6FF599E3L, 0x9D9E1AE0L,
    0xD3D3E1ABL, 0x21B862A8L, 0x32E8915CL, 0xC083125FL, 0x144976B4L, 0xE622F5B7L, 0xF5720643L, 0x07198540L,
    0x590AB964L, 0xAB613A67L, 0xB831C993L, 0x4A5A4A90L, 0x9E902E7BL, 0x6CFBAD78L, 0x7FAB5E8CL, 0x8DC0DD8FL,
    0xE330A81AL, 0x115B2B19L, 0x020BD8EDL, 0xF0605BEEL, 0x24AA3F05L, 0xD6C1BC06L, 0xC5914FF2L, 0x37FACCF1L,
    0x69E9F0D5L, 0x9B8273D6L, 0x88D28022L, 0x7AB90321L, 0xAE7367CAL, 0x5C18E4C9L, 0x4F48173DL, 0xBD23943EL,
    0xF36E6F75L, 0x0105EC76L, 0x12551F82L, 0xE03E9C81L, 0x34F4F86AL, 0xC69F7B69L, 0xD5CF889DL, 0x27A40B9EL,
    0x79B737BAL, 0x8BDCB4B9L, 0x988C474DL, 0x6AE7C44EL, 0xBE2DA0A5L, 0x4C4623A6L, 0x5F16D052L, 0xAD7D5351L
};
uint32_t crc32_calc(char *start,int length)
{
    uint32_t crc32_val=0;
    while(length--) crc32_val=(crc32_val>>8)^crc32c_lookup[(crc32_val&0xff)^(unsigned char)*start++];
    return crc32_val;
}

/**
 * function to convert ascii to number
 */
int atoi(unsigned char*c)
{
    int r=0;
    while(*c>='0'&&*c<='9') {
        r*=10; r+=*c-'0';
        c++;
    }
    return r;
}

/**
 * convert ascii octal number to binary number
 */
int octbin(unsigned char *str,int size)
{
    int s=0;
    unsigned char *c=str;
    while(size-->0){
        s*=8;
        s+=*c-'0';
        c++;
    }
    return s;
}

/**
 * convert ascii hex number to binary number
 */
int hexbin(unsigned char *str, int size)
{
    int v=0;
    while(size-->0){
        v <<= 4;
        if(*str>='0' && *str<='9')
            v += (int)((unsigned char)(*str)-'0');
        else if(*str >= 'A' && *str <= 'F')
            v += (int)((unsigned char)(*str)-'A'+10);
        str++;
    }
    return v;
}

/**
 * print panic and halt
 */
void panic(char *str)
{
    printf("BOOTBOOT-PANIC: %s\n", str);
    halt();
}

/**
 * Read a line from console
 */
int ReadLine(unsigned char *buf, int l)
{
    int i=0;
    char c;
    while(1) {
        c=getchar();
        if(c=='\n' || c=='\r') {
            break;
        } else
        if(c==8) {
            if(i) i--;
            buf[i]=0;
            continue;
        } else
        if(c==27) {
            buf[0]=0;
            return 0;
        } else
        if(c && i<l-1) {
            buf[i++]=c;
            buf[i]=0;
        }
    }
    return i;
}

static size_t usbcount = 0,ahcicount = 0;
#if IS_ENABLED(CONFIG_LP_USB)
static usbdev_t* usbdevs[8];

void usbdisk_create(usbdev_t* dev)
{
    if (usbcount < (int)(sizeof(usbdevs)/sizeof(usbdevs[0])) - 1)
        usbdevs[usbcount++] = dev;
}
#endif

/**
 * read a block from disk and return the number of sectors read
 * returns 0 on error.
 */
ssize_t disk_read(size_t dev_num, lba_t start, size_t count, unsigned char *buf)
{
#if IS_ENABLED(CONFIG_LP_STORAGE)
    /* first try SATA, AHCI etc. */
    if(dev_num < ahcicount)
        return storage_read_blocks512(dev_num, start, count, buf);
    dev_num -= ahcicount;
#endif
#if IS_ENABLED(CONFIG_LP_USB)
    /* USB storages */
    if(dev_num < usbcount)
        return readwrite_blocks_512(usbdevs[dev_num], start, count, cbw_direction_data_in, buf) ? 0 : count;
    dev_num -= usbcount;
#endif
#ifndef BBNOIDEFALLBACK
    /* fallback primary ATA IDE */
    size_t i = inb(0x1F6);
    if(i != 0 && i != 0xFF) {
        for(i = 0; i < count; i++, buf+=512, start++) {
            while((inb(0x1F7) & 0xC0) != 0x40);
            outb(1, 0x1F2);
            outb(start, 0x1F3);
            outb(start >> 8, 0x1F4);
            outb(start >> 16, 0x1F5);
            outb((start >> 24) | 0xE0, 0x1F6);
            outb(0x20, 0x1F7);  // cmd 0x20 - read sectors
            while((inb(0x1F7) & 0xC0) != 0x40);
            insl(0x1F0, buf, 512/4);
        }
        return count;
    }
#endif
    return 0;
}

// get filesystem drivers for initrd
#include "fs.h"

/**
 * Parse FS0:\BOOTBOOT\CONFIG or /sys/config
 */
void ParseEnvironment(uint8_t *env)
{
    uint8_t *end=env+PAGESIZE, *start=env;
    DBG(" * Environment @%p %d bytes\n",env,env ? (int)strlen((char*)env) : 0);
    env--; env[PAGESIZE]=0; kne=NULL;
    while(env<end) {
        env++;
        // failsafe
        if(env[0]==0)
            break;
        // skip white spaces
        if(env[0]==' '||env[0]=='\t'||env[0]=='\r'||env[0]=='\n')
            continue;
        // skip comments
        if((env[0]=='/'&&env[1]=='/')||env[0]=='#') {
            while(env<end && env[0]!='\r' && env[0]!='\n' && env[0]!=0){
                env++;
            }
            env--;
            continue;
        }
        if(env[0]=='/'&&env[1]=='*') {
            env+=2;
            while(env[0]!=0 && env[-1]!='*' && env[0]!='/')
                env++;
        }
        // only match on beginning of line
        if(env>start && env[-1]!=' '&&env[-1]!='\t'&&env[-1]!='\r'&&env[-1]!='\n')
            continue;
        // parse screen dimensions
        if(!memcmp(env,"screen=",7)){
            env+=7;
            reqwidth=atoi(env);
            while(env<end && *env!=0 && *(env-1)!='x') env++;
            reqheight=atoi(env);
        }
        // get kernel's filename
        if(!memcmp(env,"kernel=",7)){
            env+=7;
            kernelname=(char*)env;
            while(env<end && env[0]!='\r' && env[0]!='\n' &&
                env[0]!=' ' && env[0]!='\t' && env[0]!=0)
                    env++;
            kne=env;
            *env=0;
            env++;
        }
        // skip SMP initialization
        if(!memcmp(env,"nosmp=1",7)){
            env+=7;
            nosmp=1;
        }
    }
}

/**
 * Get a linear frame buffer
 */
void GetLFB()
{
    /* FIXME: is there a way to set screen resolution with libpayload? */
    bootboot->fb_width=lib_sysinfo.framebuffer.x_resolution;
    bootboot->fb_height=lib_sysinfo.framebuffer.y_resolution;
    bootboot->fb_scanline=lib_sysinfo.framebuffer.bytes_per_line;
    bootboot->fb_ptr=(uint64_t)lib_sysinfo.framebuffer.physical_address;
    bootboot->fb_size=lib_sysinfo.framebuffer.y_resolution * lib_sysinfo.framebuffer.bytes_per_line;
    bootboot->fb_type=(!lib_sysinfo.framebuffer.blue_mask_pos ? FB_ARGB : (
        !lib_sysinfo.framebuffer.red_mask_pos ? FB_ABGR : (
        lib_sysinfo.framebuffer.blue_mask_pos == 8 ? FB_RGBA : FB_BGRA
    )));
    DBG(" * Screen %d x %d, scanline %d, fb @%llx %d bytes, type %d %s\n",
        bootboot->fb_width, bootboot->fb_height, bootboot->fb_scanline,
        bootboot->fb_ptr, bootboot->fb_size,bootboot->fb_type,
        bootboot->fb_type==FB_ARGB?"ARGB":(bootboot->fb_type==FB_ABGR?"ABGR":(
        bootboot->fb_type==FB_RGBA?"RGBA":"BGRA")));
}

/**
 * Locate and load the kernel in initrd
 */
void LoadCore()
{
    uint64_t bss = 0, ptr;
    uint32_t r = 0;

    entrypoint=0;
    core.ptr=NULL;
    while(core.ptr==NULL && fsdrivers[r]!=NULL) {
        core=(*fsdrivers[r++])(initrd.ptr,kernelname);
    }
    if(kne!=NULL)
        *kne='\n';
    // scan for the first executable
    if(core.ptr==NULL || core.size==0) {
        DBG(" * Autodetecting kernel%s\n","");
        core.size=0;
        r=initrd.size;
        core.ptr=initrd.ptr;
        while(r-->0) {
            Elf64_Ehdr *ehdr=(Elf64_Ehdr *)(core.ptr);
            pe_hdr *pehdr=(pe_hdr*)(core.ptr + ((mz_hdr*)(core.ptr))->peaddr);
            if((!memcmp(ehdr->e_ident,ELFMAG,SELFMAG)||!memcmp(ehdr->e_ident,"OS/Z",4))&&
                ehdr->e_ident[EI_CLASS]==ELFCLASS64&&
                ehdr->e_ident[EI_DATA]==ELFDATA2LSB&&
                ehdr->e_machine==EM_X86_64&&
                ehdr->e_phnum>0){
                    core.size=1;
                    break;
                }
            if(((mz_hdr*)(core.ptr))->magic==MZ_MAGIC && ((mz_hdr*)(core.ptr))->peaddr<65536 && pehdr->magic == PE_MAGIC &&
                pehdr->machine == IMAGE_FILE_MACHINE_AMD64 && pehdr->file_type == PE_OPT_MAGIC_PE32PLUS) {
                    core.size=1;
                    break;
                }
            core.ptr++;
        }
    }
    if(core.ptr==NULL || core.size==0) {
        panic("Kernel not found in initrd");
    } else {
        Elf64_Ehdr *ehdr=(Elf64_Ehdr *)(core.ptr);
        pe_hdr *pehdr=(pe_hdr*)(core.ptr + ((mz_hdr*)(core.ptr))->peaddr);
        if((!memcmp(ehdr->e_ident,ELFMAG,SELFMAG)||!memcmp(ehdr->e_ident,"OS/Z",4))&&
            ehdr->e_ident[EI_CLASS]==ELFCLASS64&&
            ehdr->e_ident[EI_DATA]==ELFDATA2LSB&&
            ehdr->e_machine==EM_X86_64&&
            ehdr->e_phnum>0){
                DBG(" * Parsing ELF64 @%p\n",core.ptr);
                Elf64_Phdr *phdr=(Elf64_Phdr *)((uint8_t *)ehdr+ehdr->e_phoff);
                for(r=0;r<ehdr->e_phnum;r++){
                    if(phdr->p_type==PT_LOAD && (phdr->p_vaddr >> 30) == 0x3FFFFFFFF) {
                        core.ptr += phdr->p_offset;
                        // hack to keep symtab and strtab for shared libraries
                        core.size = phdr->p_filesz + (ehdr->e_type==3?0x4000:0);
                        bss = phdr->p_memsz - core.size;
                        core_addr = phdr->p_vaddr;
                        entrypoint = ehdr->e_entry;
                        break;
                    }
                    phdr=(Elf64_Phdr *)((uint8_t *)phdr+ehdr->e_phentsize);
                }
                if(ehdr->e_shoff > 0) {
                    Elf64_Shdr *shdr=(Elf64_Shdr *)((uint8_t *)ehdr + ehdr->e_shoff), *sym_sh = NULL, *str_sh = NULL;
                    Elf64_Shdr *strt=(Elf64_Shdr *)((uint8_t *)shdr+(uint64_t)ehdr->e_shstrndx*(uint64_t)ehdr->e_shentsize);
                    Elf64_Sym *sym = NULL, *s;
                    char *strtable = (char *)ehdr + strt->sh_offset;
                    uint32_t strsz = 0, syment = 0, i;
                    for(i = 0; i < ehdr->e_shnum; i++){
                        /* checking shdr->sh_type is not enough, there can be multiple SHT_STRTAB records... */
                        if(!memcmp(strtable + shdr->sh_name, ".symtab", 8)) sym_sh = shdr;
                        if(!memcmp(strtable + shdr->sh_name, ".strtab", 8)) str_sh = shdr;
                        shdr = (Elf64_Shdr *)((uint8_t *)shdr + ehdr->e_shentsize);
                    }
                    if(str_sh && sym_sh) {
                        strtable = (char *)ehdr + str_sh->sh_offset; strsz = str_sh->sh_size;
                        sym = (Elf64_Sym *)((uint8_t*)ehdr + sym_sh->sh_offset); syment = sym_sh->sh_entsize;
                        if(str_sh->sh_offset && strsz > 0 && sym_sh->sh_offset && syment > 0)
                            for(s = sym, i = 0; i<(strtable-(char*)sym)/syment && s->st_name < strsz; i++, s++) {
                                if(!memcmp(strtable + s->st_name, "bootboot", 9)) bb_addr = s->st_value;
                                if(!memcmp(strtable + s->st_name, "environment", 12)) env_addr = s->st_value;
                                if(!memcmp(strtable + s->st_name, "mmio", 5)) mm_addr = s->st_value;
                                if(!memcmp(strtable + s->st_name, "fb", 3)) fb_addr = s->st_value;
                                if(!memcmp(strtable + s->st_name, "initstack", 10)) initstack = s->st_value;
                            }
                    }
                }
        } else
        if(((mz_hdr*)(core.ptr))->magic==MZ_MAGIC && ((mz_hdr*)(core.ptr))->peaddr<65536 && pehdr->magic == PE_MAGIC &&
            pehdr->machine == IMAGE_FILE_MACHINE_AMD64 && pehdr->file_type == PE_OPT_MAGIC_PE32PLUS &&
            (pehdr->code_base & 0xC0000000)) {
                DBG(" * Parsing PE32+ @%p\n",core.ptr);
                core.size = (pehdr->entry_point-pehdr->code_base) + pehdr->text_size + pehdr->data_size;
                bss = pehdr->bss_size;
                core_addr = (int64_t)pehdr->code_base;
                entrypoint = (int64_t)pehdr->entry_point;
                if(pehdr->sym_table > 0 && pehdr->numsym > 0) {
                    pe_sym *s;
                    char *strtable = (char *)pehdr + pehdr->sym_table + pehdr->numsym * 18 + 4, *name;
                    uint32_t i;
                    for(i = 0; i < pehdr->numsym; i++) {
                        s = (pe_sym*)((uint8_t *)pehdr + pehdr->sym_table + i * 18);
                        name = !s->iszero ? (char*)&s->iszero : strtable + s->nameoffs;
                        if(!memcmp(name, "bootboot", 9)) bb_addr = (int64_t)s->value;
                        if(!memcmp(name, "environment", 12)) env_addr = (int64_t)s->value;
                        if(!memcmp(name, "mmio", 5)) mm_addr = (int64_t)s->value;
                        if(!memcmp(name, "fb", 3)) fb_addr = (int64_t)s->value;
                        if(!memcmp(name, "initstack", 10)) initstack = (int64_t)s->value;
                        i += s->auxsyms;
                    }
                }
        }
    }
    if(core.ptr==NULL || core.size<2 || entrypoint==0 || (core_addr&(PAGESIZE-1)) || (bb_addr>>30)!=0x3FFFFFFFF ||
        (bb_addr & (PAGESIZE-1)) || (env_addr>>30)!=0x3FFFFFFFF || (env_addr&(PAGESIZE-1)) || (fb_addr>>30)!=0x3FFFFFFFF ||
        (mm_addr & (PAGESIZE-1)) || (mm_addr>>30)!=0x3FFFFFFFF || (fb_addr & (1024*1024*2-1)))
            panic("Kernel is not a valid executable");
    if(core.size+bss > 16*1024*1024)
        panic("Kernel is too big");
    if(initstack < 1024) initstack = 1024;
    if(initstack > 16384) initstack = 16384;
    // create core segment
    ptr = ((bootboot->initrd_ptr+bootboot->initrd_size) + PAGESIZE - 1) & ~(PAGESIZE-1);
    memcpy((void*)(uintptr_t)ptr, core.ptr, core.size);
    core.ptr=(uint8_t*)(uintptr_t)ptr;
    if(bss>0)
        memset(core.ptr + core.size, 0, bss);
    core.size += bss;
    DBG(" * fb          @%llx\n", fb_addr);
    DBG(" * bootboot    @%llx\n", bb_addr);
    DBG(" * environment @%llx\n", env_addr);
    DBG(" * Entry point @%llx, text @%p %d bytes\n",entrypoint, core.ptr, core.size);
    if(initstack != 1024)
        DBG(" * Stack size  %d bytes per core\n", (int)initstack);
    core.size = (core.size+PAGESIZE-1)&~(PAGESIZE-1);
}

/**
 * Add a mapping to paging tables
 */
int freep = 24;
void MapPage(uint64_t virt, uint64_t phys)
{
    int i,j;
    j = (virt>>(9+12)) & 0x1FF;
    if(!paging[22*512 + j] || (paging[22*512 + j] & 0xFF) == 0x83) {
        if(freep == 37) return;
        paging[22*512 + j]=(uint64_t)((uintptr_t)paging+freep*PAGESIZE+3);
        freep++;
    }
    i = (paging[22*512 + j] - (uint64_t)((uintptr_t)paging)) >> 12;
    j = (virt>>(12)) & 0x1FF;
    paging[i*512 + j] = phys | 1;
}

/**
 * bootboot entry point, run only on BSP core
 */
int main(void)
{
    unsigned int ret=0, i, dsk, bad_madt=0;
    uint8_t *pe, *ptr;
    uint32_t np, sp, r;
    unsigned char *data;

#if IS_ENABLED(CONFIG_LP_SERIAL_CONSOLE)
    /* ridiculous, libpayload is a huge library, yet has no function to initialize serial port properly... */
    __asm__ __volatile__(
        "movl %0, %%edx;"
        "xorb %%al, %%al;outb %%al, %%dx;"               /* IER int off */
        "movb $0x80, %%al;addb $2,%%dl;outb %%al, %%dx;" /* LCR set divisor mode */
        "movb $1, %%al;subb $3,%%dl;outb %%al, %%dx;"    /* DLL divisor lo 115200 */
        "xorb %%al, %%al;incb %%dl;outb %%al, %%dx;"     /* DLH divisor hi */
        "incb %%dl;outb %%al, %%dx;"                     /* FCR fifo off */
        "movb $0x43, %%al;incb %%dl;outb %%al, %%dx;"    /* LCR 8N1, break on */
        "movb $0x8, %%al;incb %%dl;outb %%al, %%dx;"     /* MCR Aux out 2 */
        "xorb %%al, %%al;subb $4,%%dl;inb %%dx, %%al"    /* clear receiver/transmitter */
    : : "a"(CONFIG_LP_SERIAL_IOBASE + 1): );
#endif
    video_init();
#if IS_ENABLED(CONFIG_LP_STORAGE)
    storage_initialize();
    ahcicount = storage_device_count();
#endif
#if IS_ENABLED(CONFIG_LP_USB)
    usb_initialize();
    usb_poll(); // this calls usbdisk_create() if it detects any USB storages
#endif
    video_console_clear();
    printf("Booting OS...\n");

    memset(bootboot, 0, 2*PAGESIZE);
    memcpy(bootboot->magic, BOOTBOOT_MAGIC, 4);
    bootboot->protocol = PROTOCOL_DYNAMIC | LOADER_COREBOOT;
    bootboot->size = 128;

    __asm__ __volatile__ (
        "movl $1, %%eax;"
        "cpuid;"
        "shrl $4, %%eax;"
        "shrl $24, %%ebx;"
        "movw %%bx,%0;"
        "movl %%eax,%1;"
        : "=b"(bootboot->bspid),"=a"(ret) : : );
    /* check processor */
    __asm__ __volatile__ (
        "xorl %%edx, %%edx;"
        "movl $0x80000001, %%eax;"
        "cpuid;"
        "movl %%edx,%0;"
        : "=d"(i) : : );
    if((ret & 0xFFFF) < 0x0600 || !(i & (1<<29))) panic("Hardware not supported");

    /* we check this here as early as possible, because GetLFB() can't change the resolution */
    if(!lib_sysinfo.framebuffer.physical_address || !lib_sysinfo.framebuffer.bytes_per_line)
        panic("coreboot compiled without LINEAR_FRAMEBUFFER");
    if(lib_sysinfo.framebuffer.bits_per_pixel != 32)
        panic("not 32 bit pixel format, no framebuffer");

    DBG(" * Locate initrd in Flash ROM%s\n","");

    /* if it's a Flashmap area */
    initrd.ptr = 0; initrd.size = 0;
    struct fmap *fm = (struct fmap*)lib_sysinfo.fmap_cache;
    if(fm && !memcmp(fm->signature, FMAP_SIGNATURE, 8) && fm->nareas > 0)
        for(i = 0; i < fm->nareas; i++)
            if(!memcmp(fm->areas[i].name, "INITRD", 7)) {
                initrd.ptr = (uint8_t*)lib_sysinfo.fmap_cache + fm->areas[i].offset;
                initrd.size = fm->areas[i].size;
                break;
            }
#if IS_ENABLED(CONFIG_LP_CBFS)
    /* if it's on the CBFS filesystem in the COREBOOT Flashmap area */
    if(!initrd.ptr || !initrd.size) {
        size_t s;
        initrd.ptr = cbfs_map("bootboot/initrd", &s);
        if(initrd.ptr && s > 1 && s < 16*1024*1024) initrd.size = s;
        else initrd.ptr = NULL;
    }
#endif
    // skip optional ROM header
    if(initrd.ptr && initrd.ptr[0]==0x55 && initrd.ptr[1]==0xAA && initrd.ptr[8]=='I') {
        initrd.ptr += 32;
        initrd.size -= 32;
    }

#if IS_ENABLED(CONFIG_LP_SERIAL_CONSOLE)
    /* try to receive the initrd from serial line */
    if(!initrd.ptr || !initrd.size) {
        while(serial_havechar()) serial_getchar();
        serial_putchar(3); serial_putchar(3); serial_putchar(3);
        mdelay(10);
        if(serial_havechar()) {
            // we got response from raspbootcom or USBImager
            sp=serial_getchar(); sp|=serial_getchar()<<8; sp|=serial_getchar()<<16; sp|=serial_getchar()<<24;
            if(sp>0 && sp<INITRD_MAXSIZE*1024*1024) {
                serial_putchar('O'); serial_putchar('K');
                initrd.size=sp;
                initrd.ptr=pe=(uint8_t*)INITRD_BASE;
                while(sp--) *pe++ = serial_getchar();
            } else {
                serial_putchar('S'); serial_putchar('E');
            }
        }
    }
#endif

    /* fall back to INITRD on filesystem */
    if(!initrd.ptr || !initrd.size) {
        char *fn = "INITRD     ";
        // if the user presses any key now, we fallback to backup initrd
        for(i = 0; i < 100; i++) {
            if((havechar() && getchar())) {
                fn = "INITRD  BAK";
                printf(" * Backup initrd\n");
                break;
            }
            mdelay(1);
        }
        DBG(" * Locate initrd in GPT%s\n","");
        for(dsk = 0; (size_t)dsk < ahcicount + usbcount + 1 && !initrd.ptr; dsk++) {
            pe=(uint8_t*)0x4000;
            memset(pe, 0, 512);
            if(!disk_read(dsk, 1, 1, pe) || memcmp(pe, "EFI PART", 8)) continue;
            // get number of partitions and size of partition entry
            np=*((uint32_t*)(pe+80)); sp=*((uint32_t*)(pe+84));
            if(np>127) np=127;
            // read GPT entries
            if(!disk_read(dsk, *((uint32_t*)(pe+72)), (np*sp+511)/512, pe)) continue;
            efipart_t *part=NULL;
            // first, look for a partition with bootable flag
            for(r=0;r<np;r++) {
                part = (efipart_t*)(pe+r*sp);
                if((part->type[0]==0 && part->type[1]==0 && part->type[2]==0 && part->type[3]==0) || part->start==0) {
                    r=np;
                    break;
                }
                // EFI_PART_USED_BY_OS?
                if(part->flags&4) break;
            }
            // if none, look for specific partition types
            if(part==NULL || r>=np) {
                for(r=0;r<np;r++) {
                    part = (efipart_t*)(pe+r*sp);
                    if((part->type[0]==0 && part->type[1]==0 && part->type[2]==0 && part->type[3]==0) || part->start==0) {
                        r=np;
                        break;
                    }
                        // ESP?
                    if((part->type[0]==0xC12A7328 && part->type[1]==0x11D2F81F) ||
                        // or OS/Z root partition for this architecture?
                        (part->type[0]==0x5A2F534F && (part->type[1]&0xFFFF)==0x8664 && part->type[3]==0x746F6F72))
                        break;
                }
            }
            if(part && r<np) {
                uint64_t start = part->start, end = part->end;
                if(!disk_read(dsk, start, 1, pe)) continue;
                //is it a FAT partition?
                bpb_t *bpb=(bpb_t*)pe;
                if(!memcmp((void*)bpb->fst,"FAT16",5) || !memcmp((void*)bpb->fst2,"FAT32",5)) {
                    // locate BOOTBOOT directory
                    uint64_t data_sec, root_sec, clu=0, cclu=0, s, s2, s3;
                    fatdir_t *dir;
                    uint32_t *fat32=(uint32_t*)(pe+512);
                    uint16_t *fat16=(uint16_t*)fat32;
                    uint8_t *ptr;
                    data_sec=root_sec=((bpb->spf16?bpb->spf16:bpb->spf32)*bpb->nf)+bpb->rsc;
                    s=(bpb->nr0+(bpb->nr1<<8))*sizeof(fatdir_t);
                    if(bpb->spf16>0) {
                        data_sec+=(s+511)>>9;
                    } else {
                        root_sec+=(bpb->rc-2)*bpb->spc;
                    }
                    s3=bpb->spc*512;
                    // load fat table
                    r=disk_read(dsk, start+1, (bpb->spf16?bpb->spf16:bpb->spf32)+bpb->rsc, pe + 512);
                    if(!r) continue;
                    pe+=(r+1)*512;
                    // load root directory
                    r=disk_read(dsk, start+root_sec, s/512+1, pe);
                    if(!r) continue;
                    dir=(fatdir_t*)pe;
                    while(dir->name[0]!=0 && memcmp(dir->name,"BOOTBOOT   ",11)) dir++;
                    if(dir->name[0]!='B') continue;
                    r=disk_read(dsk, start+(dir->cl+(dir->ch<<16)-2)*bpb->spc+data_sec, bpb->spc, pe);
                    if(!r) continue;
                    dir=(fatdir_t*)pe;
                    // locate environment and initrd
                    while(dir->name[0]!=0) {
                        if(!memcmp(dir->name,"CONFIG     ",11)) {
                            s=dir->size<PAGESIZE?dir->size:PAGESIZE; // round up to cluster size
                            cclu=dir->cl+(dir->ch<<16);
                            ptr=(uint8_t*)environment;
                            while(s>0) {
                                s2=s>s3?s3:s;
                                r=disk_read(dsk, start+(cclu-2)*bpb->spc+data_sec,s2<512?1:(s2+511)/512,ptr);
                                cclu=bpb->spf16>0?fat16[cclu]:fat32[cclu];
                                ptr+=s2;
                                s-=s2;
                            }
                        } else
                        if(!memcmp(dir->name,fn,11)) {
                            clu=dir->cl+(dir->ch<<16);
                            initrd.size=dir->size;
                        }
                        dir++;
                    }
                    // if initrd not found, try architecture specific name
                    if(clu==0) {
                        dir=(fatdir_t*)pe;
                        while(dir->name[0]!=0) {
                            if(!memcmp(dir->name,"X86_64     ",11)) {
                                clu=dir->cl+(dir->ch<<16);
                                initrd.size=dir->size;
                                break;
                            }
                            dir++;
                        }
                    }
                    // walk through cluster chain to load initrd
                    if(clu!=0 && initrd.size!=0) {
                        initrd.ptr=ptr=(uint8_t*)INITRD_BASE;
                        s=initrd.size;
                        while(s>0) {
                            s2=s>s3?s3:s;
                            r=disk_read(dsk, start+(clu-2)*bpb->spc+data_sec,s2<512?1:(s2+511)/512,ptr);
                            clu=bpb->spf16>0?fat16[clu]:fat32[clu];
                            ptr+=s2;
                            s-=s2;
                        }
                    }
                } else {
                    // initrd is on the entire partition
                    r=disk_read(dsk, start,end-start,(uint8_t*)INITRD_BASE);
                    if(!r) continue;
                    initrd.ptr=(uint8_t*)INITRD_BASE;
                    initrd.size=r*512;
                }
            }
        }
    }
    if(!initrd.ptr || !initrd.size)
        panic("Initrd not found");
    data = initrd.ptr == (uint8_t*)INITRD_BASE ?
        (uint8_t*)(((uintptr_t)initrd.ptr+initrd.size+PAGESIZE-1) & ~(PAGESIZE-1)) : (uint8_t*)INITRD_BASE;
    // check if initrd is gzipped
    if(initrd.ptr[0]==0x1f && initrd.ptr[1]==0x8b) {
        unsigned char *addr,f;
        int len=0, r;
        TINF_DATA d;
        DBG(" * Gzip compressed initrd @%p %d bytes\n",initrd.ptr,initrd.size);
        // skip gzip header
        addr=initrd.ptr+2;
        if(*addr++!=8) goto gzerr;
        f=*addr++; addr+=6;
        if(f&4) { r=*addr++; r+=(*addr++ << 8); addr+=r; }
        if(f&8) { while(*addr++ != 0); }
        if(f&16) { while(*addr++ != 0); }
        if(f&2) addr+=2;
        d.source = addr;
        memcpy(&len,initrd.ptr+initrd.size-4,4);
        // decompress
        d.bitcount = 0;
        d.bfinal = 0;
        d.btype = -1;
        d.curlen = 0;
        d.dest = data;
        d.destSize = len;
        do { r = uzlib_uncompress(&d); } while (!r);
        if (r != TINF_DONE) {
gzerr:      panic("Unable to uncompress");
        }
        // swap initrd.ptr with the uncompressed buffer
        initrd.ptr=data;
        initrd.size=len;
    }
#if IS_ENABLED(CONFIG_LP_LZMA)
    // check if initrd is compressed with xz
    if(initrd.ptr[0] == 0xFD && initrd.ptr[1] == '7' && initrd.ptr[2] == 'z' && initrd.ptr[3] == 'X' && initrd.ptr[4] == 'Z') {
        DBG(" * Xz (lzma) compressed initrd @%p %d bytes\n",initrd.ptr,initrd.size);
        initrd.size = ulzma(initrd.ptr, data);
        initrd.ptr = data;
        if(initrd.size < 1) goto gzerr;
    }
#endif
#if IS_ENABLED(CONFIG_LP_LZ4)
    // check if initrd is compressed with lz4
    if(initrd.ptr[0] == 0x04 && initrd.ptr[1] == 0x22 && initrd.ptr[2] == 0x4D && initrd.ptr[3] == 0x18) {
        DBG(" * Lz4 compressed initrd @%p %d bytes\n",initrd.ptr,initrd.size);
        initrd.size = ulz4f(initrd.ptr, data);
        initrd.ptr = data;
        if(initrd.size < 1) goto gzerr;
    }
#endif

    DBG(" * Initrd loaded @%p %d bytes\n",initrd.ptr,initrd.size);
    bootboot->initrd_ptr = (uint64_t)(uintptr_t)initrd.ptr;
    bootboot->initrd_size = initrd.size;

    if(!environment[0]) {
        // if there's no environment file on boot partition, find it inside the INITRD
        file_t ret;
        i=0; ret.ptr=NULL; ret.size=0;
        while(ret.ptr==NULL && fsdrivers[i]!=NULL) {
            ret=(*fsdrivers[i++])(initrd.ptr,cfgname);
        }
        if(ret.ptr!=NULL)
            memcpy(environment, ret.ptr, ret.size<PAGESIZE ? ret.size : PAGESIZE-1);
    }
#if IS_ENABLED(CONFIG_LP_CBFS)
    if(!environment[0]) {
        // if there's no environment on the boot partition neither inside the INITRD, but we have CBFS support, try that too
        size_t s;
        ptr = cbfs_map("bootboot/config", &s);
        if(ptr && s > 1 && s < PAGESIZE-1) memcpy(environment, ptr, s);
    }
#endif
    ParseEnvironment((uint8_t*)environment);

    GetLFB();

    DBG(" * System tables%s\n","");
    for(data = (unsigned char*)phys_to_virt(0x000f0000); data < (unsigned char*)phys_to_virt(0x00100000); data += 16) {
        if(!memcmp(data, "RSD PTR ", 8))
            bootboot->arch.x86_64.acpi_ptr = *((uint64_t*)(data + 24)) ? *((uint64_t*)(data + 24)) :
                (uint64_t)(*((uint32_t*)(data + 16)));
        if(!memcmp(data, "_SM_", 4))
            bootboot->arch.x86_64.smbi_ptr = (uint64_t)(uintptr_t)data;
        if(!memcmp(data, "_MP_", 4))
            bootboot->arch.x86_64.mp_ptr = (uint64_t)(uintptr_t)data;
    }

#if IS_ENABLED(CONFIG_LP_NVRAM)
    struct tm t;
    rtc_read_clock(&t);
    bootboot->datetime[0]=dec2bcd((t.tm_year+1900)/100);
    bootboot->datetime[1]=dec2bcd((t.tm_year+1900)%100);
    bootboot->datetime[2]=dec2bcd(t.tm_mon+1);
    bootboot->datetime[3]=dec2bcd(t.tm_mday);
    bootboot->datetime[4]=dec2bcd(t.tm_hour);
    bootboot->datetime[5]=dec2bcd(t.tm_min);
    bootboot->datetime[6]=dec2bcd(t.tm_sec);
    bootboot->datetime[7]=t.tm_isdst;
    DBG(" * System time %d-%02d-%02d %02d:%02d:%02d GMT%s%d:%02d %s\n",
        t.tm_year+1900,t.tm_mon+1,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec,
        bootboot->timezone>=0?"+":"",bootboot->timezone/60,bootboot->timezone%60,
        t.tm_isdst?"summertime":"");
#endif

    // locate sys/core
    LoadCore();

    /* Symmetric Multi Processing support */
    memset(lapic_ids, 0xFF, sizeof(lapic_ids));
    ptr = (uint8_t*)(uintptr_t)bootboot->arch.x86_64.acpi_ptr;
    if(ptr && (ptr[0]=='X' || ptr[0]=='R') && ptr[1]=='S' && ptr[2]=='D' && ptr[3]=='T') {
        pe = ptr; ptr += 36;
        // iterate on ACPI table pointers
        for(r = *((uint32_t*)(pe + 4)); ptr < pe + r; ptr += pe[0] == 'X' ? 8 : 4) {
            data = (uint8_t*)(uintptr_t)(pe[0] == 'X' ? *((uint64_t*)ptr) : *((uint32_t*)ptr));
            if(!memcmp(data, "APIC", 4)) {
                // found MADT, iterate on its variable length entries
                lapic_addr = (uint64_t)(*((uint32_t*)(data+0x24)));
                for(r = *((uint32_t*)(data + 4)), ptr = data + 44, i = 0; ptr < data + r &&
                    i < (int)(sizeof(lapic_ids)/sizeof(lapic_ids[0])); ptr += ptr[1]) {
                    switch(ptr[0]) {
                        case 0:                                             // found Processor Local APIC
                            if((ptr[4] & 1) && ptr[3] != 0xFF && lapic_ids[(int)ptr[3]] == 0xFFFF)
                                lapic_ids[(int)ptr[3]] = i++;
                            else bad_madt++;
                        break;
                        case 5: lapic_addr = *((uint64_t*)(ptr+4)); break;  // found 64 bit Local APIC Address
                    }
                }
                if(i && lapic_ids[bootboot->bspid] != 0xFFFF) bootboot->numcores = i;
                break;
            }
        }
    }

    // disable PIC and NMI
    __asm__ __volatile__ (
        "movb $0xFF, %%al; outb %%al, $0x21; outb %%al, $0xA1;"                 // disable PIC
        "inb $0x70, %%al; orb $0x80, %%al; outb %%al, $0x70;"                   // disable NMI
        : : :);

    if(!nosmp && bootboot->numcores > 1 && lapic_addr) {
        DBG(" * SMP numcores %d%s\n", bootboot->numcores, bad_madt ? " (bad MADT)" : "");
        memcpy((uint8_t*)0x1000, (void*)&ap_trampoline, 128);

        // enable Local APIC
        *((volatile uint32_t*)((uintptr_t)lapic_addr + 0x0D0)) = (1 << 24);
        *((volatile uint32_t*)((uintptr_t)lapic_addr + 0x0E0)) = 0xFFFFFFFF;
        *((volatile uint32_t*)((uintptr_t)lapic_addr + 0x0F0)) = *((volatile uint32_t*)((uintptr_t)lapic_addr + 0x0F0)) | 0x100;
        *((volatile uint32_t*)((uintptr_t)lapic_addr + 0x080)) = 0;
        // make sure we use the correct Local APIC ID for the BSP
        bootboot->bspid = *((volatile uint32_t*)((uintptr_t)lapic_addr + 0x20)) >> 24;

#if 0
        // use broadcast IPI if MADT is okay (no bcast id and all CPUs enabled)
        if(!bad_madt) {
            // send Broadcast INIT IPI
            *((volatile uint32_t*)((uintptr_t)lapic_addr + 0x300)) = 0x0C4500;
            mdelay(10);                                                     // wait 10 msec
            // send Broadcast STARTUP IPI
            *((volatile uint32_t*)((uintptr_t)lapic_addr + 0x300)) = 0x0C4601;// start at 0100:0000h
            udelay(200);                                                    // wait 10 msec
            // send second SIPI
            *((volatile uint32_t*)((uintptr_t)lapic_addr + 0x300)) = 0x0C4601;
            udelay(200);                                                    // wait 10 msec
        } else
#endif
        {
            // supports up to 255 cores (lapicid 255 is bcast address), requires x2APIC to have more
            for(i = 0; i < 255; i++) {
                if(i == bootboot->bspid || lapic_ids[i] == 0xFFFF) continue;
                *((volatile uint32_t*)((uintptr_t)lapic_addr + 0x280)) = 0; // clear APIC errors
                r = *((volatile uint32_t*)((uintptr_t)lapic_addr + 0x280));
                send_ipi(i, 0xfff00000, 0x00C500);                          // trigger INIT IPI
                udelay(200);
                send_ipi(i, 0xfff00000, 0x008500);                          // deassert INIT IPI
            }
            mdelay(10);                                                     // wait 10 msec
            for(i = 0; i < 255; i++) {
                if(i == bootboot->bspid || lapic_ids[i] == 0xFFFF) continue;
                *((volatile uint8_t*)0x1011) = 0;
                send_ipi(i, 0xfff0f800, 0x004601);                          // trigger SIPI, start at 0100:0000h
                for(r = 250; !*((volatile uint8_t*)0x1011) && r > 0; r--) udelay(200);// wait for AP with 50 msec timeout
                if(!*((volatile uint8_t*)0x1011)) {
                    send_ipi(i, 0xfff0f800, 0x004601);
                    mdelay(50);
                }
            }
        }
    } else {
        bootboot->numcores = 1;
        lapic_addr = 0;
        lapic_ids[bootboot->bspid] = 0;
    }

    /* Create paging tables */
    DBG(" * Pagetables PML4 @%p\n", (void*)paging);
    memset(paging, 0, (37+(bootboot->numcores*initstack+PAGESIZE-1)/PAGESIZE)*PAGESIZE);
    //PML4
    paging[0]=(uint64_t)((uintptr_t)paging+PAGESIZE)+3;  // pointer to 2M PDPE (16G RAM identity mapped)
    paging[511]=(uint64_t)((uintptr_t)paging+20*PAGESIZE)+3;   // pointer to 4k PDPE (core mapped at -2M)
    //identity mapping
    //2M PDPE
    for(i=0;i<16;i++)
        paging[512+i]=(uint64_t)((uintptr_t)paging+(3+i)*PAGESIZE+3);
    //first 2M mapped per page
    paging[3*512]=(uint64_t)((uintptr_t)paging+2*PAGESIZE+3);
    for(i=0;i<512;i++)
        paging[2*512+i]=(uint64_t)(i*PAGESIZE+3);
    //2M PDE
    for(i=1;i<512*16;i++)
        paging[3*512+i]=(uint64_t)((i<<21)+0x83);
    //kernel mapping
    //4k PDPE
    paging[20*512+511]=(uint64_t)((uintptr_t)paging+22*PAGESIZE+3);
    //4k PDE
    r = (fb_addr>>(9+12)) & 0x1FF;
    for(i=0;r+i<511 && i<63;i++)
        paging[22*512+r+i]=(uint64_t)(((uintptr_t)(bootboot->fb_ptr)+(i<<21))+0x83);   // map framebuffer
    paging[22*512+511]=(uint64_t)((uintptr_t)paging+23*PAGESIZE+3);
    //4k PT
    //dynamically map these. Main struct, environment string and code segment
    for(i=0;i<(core.size/PAGESIZE);i++)
        MapPage(core_addr + i*PAGESIZE, (uint64_t)((uintptr_t)core.ptr+i*PAGESIZE+3));
    MapPage(bb_addr, (uint64_t)((uintptr_t)bootboot)+1);
    MapPage(env_addr, (uint64_t)((uintptr_t)environment)+1);
    // stack at the top of the memory
    for(i=0; i<((bootboot->numcores*initstack+PAGESIZE-1)/PAGESIZE); i++) {
        if(paging[23*512+511-i])
            panic("Stack smash");
        paging[23*512+511-i]=(uint64_t)((uintptr_t)paging+(37+i)*PAGESIZE+3);  // core stacks
    }

    /* Get memory map */
    uint64_t srt, end, ldrend = (uintptr_t)paging + (37+(bootboot->numcores*initstack+PAGESIZE-1)/PAGESIZE)*PAGESIZE;
    uint64_t iniend = (uint64_t)(uintptr_t)core.ptr + core.size, a, b;
    MMapEnt *mmapent=(MMapEnt *)&(bootboot->mmap), *sort;
    for (i = 0; (int)i < lib_sysinfo.n_memranges; i++) {
        srt = lib_sysinfo.memrange[i].base;
        end = srt + lib_sysinfo.memrange[i].size;
        srt = (srt + PAGESIZE-1) & ~(PAGESIZE-1); end &= ~(PAGESIZE-1);
        r = lib_sysinfo.memrange[i].type == CB_MEM_RAM || lib_sysinfo.memrange[i].type == CB_MEM_TABLE ? MMAP_FREE : (
            lib_sysinfo.memrange[i].type == CB_MEM_ACPI || lib_sysinfo.memrange[i].type == CB_MEM_NVS ? MMAP_ACPI : MMAP_USED);
        /* exclude two intervals: 0 - ldrend and INITRD_BASE - iniend */
        // +--------+
        // +--------------+
        if(end <= ldrend) continue;
        //            +--------+
        // +--------------+
        if(srt < ldrend && ldrend < end) srt = ldrend;
        //            +---+
        //         +--------+
        if(srt >= INITRD_BASE && end <= iniend) continue;
        //            +----------+
        //         +--------+
        if(srt > INITRD_BASE && srt < iniend && iniend < end) srt = iniend; else
        //   +----------+
        //         +--------+
        if(srt < INITRD_BASE && end > INITRD_BASE && end < iniend) end = iniend; else
        //    +------------------+
        //         +--------+
        if(srt < INITRD_BASE && end > iniend) {
            // split into two regions
            mmapent->ptr = srt;
            mmapent->size = (INITRD_BASE - srt) | (r & 0xF);
            // bubble up record. It's okay to use an ineffective, but simple sort here, because there are no
            // more records than a couple hundred, and in most scenearios they are already sorted, meaning this
            // loop will never run at all. But in the unlikely event they aren't sorted, this will fix that.
            for(sort = mmapent - 1; sort >= (MMapEnt *)&(bootboot->mmap) && sort[0].ptr > sort[1].ptr; sort--) {
                a = sort[0].ptr; b = sort[0].size;
                sort[0].ptr = sort[1].ptr; sort[0].size = sort[1].size;
                sort[1].ptr = a; sort[1].size = b;
            }
            mmapent++;
            bootboot->size += sizeof(MMapEnt);
            srt = iniend;
        }
        if(srt >= end) continue;
        // merge contiguous areas */
        if(bootboot->size > 128 && MMapEnt_Type(mmapent) == r && MMapEnt_Ptr(mmapent) + MMapEnt_Size(mmapent) == srt) {
            mmapent->size += end - srt;
        } else {
            mmapent->ptr = srt;
            mmapent->size = (end - srt) | (r & 0xF);
            for(sort = mmapent - 1; sort >= (MMapEnt *)&(bootboot->mmap) && sort[0].ptr > sort[1].ptr; sort--) {
                a = sort[0].ptr; b = sort[0].size;
                sort[0].ptr = sort[1].ptr; sort[0].size = sort[1].size;
                sort[1].ptr = a; sort[1].size = b;
            }
            mmapent++;
            bootboot->size += sizeof(MMapEnt);
        }
    }
    DBG(" * Memory Map @%lx %d bytes\n",(uintptr_t)&(bootboot->mmap), bootboot->size - 128);

    /* clear the screen */
    for(np=sp=0;sp<bootboot->fb_height;sp++) {
        r=np;
        for(i=0;i<bootboot->fb_width;i+=2,r+=8)
            *((uint64_t*)(uintptr_t)(bootboot->fb_ptr + r))=0;
        np+=bootboot->fb_scanline;
    }

    /* continue in Assembly, enable long mode and jump to kernel's entry point */
#if __WORDSIZE == 64
    bsp64_init(bootboot->bspid);
#else
    bsp_init();
#endif
    return 0;
}
