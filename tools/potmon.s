;
; Compile it using cc65: http://cc65.github.io/cc65/
;
; Command line:
;    cl65 -o potmon.prg -u __EXEHDR__ -t c64 -C c64-asm.cfg potmon.s
;

.macpack cbm

.code
        jsr $e544
        lda #0
        sta $d020
        sta $d021

        lda #%11100000
        sta $dc02               ; disable keyboard


l0:
        lda #%01000000
        sta $dc00

        ldx $d419
        stx p1x
        ldy $d41a
        sty p1y


        lda #%10000000
        sta $dc00

        ldx $d419
        stx p2x
        ldy $d41a
        sty p2y

        ldx #0
        lda p1x
        jsr print_value

        ldx #3
        lda p1y
        jsr print_value

        ldx #40
        lda p2x
        jsr print_value

        ldx #43
        lda p2y
        jsr print_value


        ldx #0                  ; wait at least 512 cycles
l1:     dex
        bne l1

        jmp l0

print_value:
        pha
        lsr
        lsr
        lsr
        lsr
        tay

        lda hextable,y
        sta $0400,x

        pla
        and #%00001111
        tay

        lda hextable,y
        sta $0401,x
        rts

hextable:
        scrcode "0123456789abcdef"

p1x:
        .byte 0
p1y:
        .byte 0
p2x:
        .byte 0
p2y:
        .byte 0
