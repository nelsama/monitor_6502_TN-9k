; simple_vectors.s - Vectores básicos sin interrupciones
; Solo para programas que NO usan I2C

.segment "CODE"

nmi_handler:
    rti

irq_handler:
    rti

.segment "VECTORS"

; Vectores de interrupción del 6502
.addr   nmi_handler  ; NMI vector ($9FFA-$9FFB) 
.addr   $8000        ; RESET vector ($9FFC-$9FFD) - Inicio de ROM donde está el startup de CC65
.addr   irq_handler  ; IRQ vector ($9FFE-$9FFF)
