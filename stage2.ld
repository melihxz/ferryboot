ENTRY(_start)

SECTIONS
{
    . = 0x7E00;
    
    .text : {
        *(.text)
    }
    
    .data : {
        *(.data)
    }
    
    .rodata : {
        *(.rodata)
    }
    
    .bss : {
        *(.bss)
    }
    
    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
    }
}