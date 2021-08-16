SECTIONS
{
    .rodata.ctor : {
        PROVIDE(ctor_start = .);
        *(.ctor)
        PROVIDE(ctor_end = .);
    }
    .rodata.dtor : {
        PROVIDE(dtor_start = .);
        *(.dtor)
        PROVIDE(dtor_end = .);
    }
}
/*
INSERT AFTER .text;
*/
