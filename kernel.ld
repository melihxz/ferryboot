OUTPUT_FORMAT("elf32-i386")
ENTRY(kmain)
SECTIONS
{
    . = 0x1000;
    .text : { *(.text) }
}
