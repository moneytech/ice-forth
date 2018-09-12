
.ifndef __FORTH_INC__
.define __FORTH_INC__

; The address of the IO port in the 6502 emulator
.global IO_PORT

; Flags for the dictionary
.globalzp F_IMMED
.globalzp F_INLINE
.globalzp F_HIDDEN
.global F_END

; Temporary storage
.globalzp TMP1, TMP2, TMP3, TMP4, TMP5, TMP6, TMP7, TMP8

; The data stack
.globalzp Stack, Stack_End

; The control flow stack
.globalzp ControlFlowSp, ControlFlowStack, ControlFlowStackEnd

; The return stack
.global RStack

.global DUP

; Pushes the given value onto the stack.
.macro push val
  dex
  dex
  lda #<val
  sta Stack, x
  lda #>val
  sta Stack+1, x
.endmacro

; Removes the top of stack.
.macro pop
  inx
  inx
.endmacro

.macro toTMP1
  lda Stack, x
  sta TMP1
  lda Stack+1, x
  sta TMP2
.endmacro

.macro fromTMP1
  lda TMP1
  sta Stack, x
  lda TMP2
  sta Stack+1, x
.endmacro

; The structure of a dictionary entry.
.struct DictEntry
  Jmp .byte
  CodePtr .word
  Flags2 .byte
  ; The length of the name. Also includes the flags
  Len .byte
  ; The entry name
  Name .byte
.endstruct

; The simulator treats $FF as a command to start
; logging each instruction for debugging.
.macro DEBUG_START
  .byte $FF
.endmacro

; The simulator treats $EF as a command to stop
; logging each instruction.
.macro DEBUG_END
  .byte $EF
.endmacro

; The simulator treats $DF as a command to print
; out the next bytes until a zero character.
.macro DEBUG_TRACE str
   .byte $DF
   .asciiz str
.endmacro

; New dictionary format, unifying xt and dictionary entry.
; jmp CODE_IMPL
; .word LINK
; .word IMMEDIATE | HIDDEN | LEN
; .byte "DUP"
; 
; The dictionary entry length does not change - once it is created
; with a name, it is fixed. However, we can update the jmp CODE_IMPL
; instruction to move the code around.
.macro defword name, flags, label
  .segment "DICT"
    label:
    .export label
    jmp .ident(.concat(.string(label), "_IMPL"))
    ; length and name
    .byte >flags
    .byte .strlen(name) | <flags
    .byte name
  .segment "DICT_CODE"
    .ident(.concat(.string(label), "_IMPL")):
.endmacro

; Variables are a special kind of word which reserve
; two bytes of space in the VARIABLES segment, and when
; executed, push that address on the stack.
.macro defvar name, init, label
  defword name, 0, label
    push .ident(.concat(.string(label), "_VALUE"))
    rts
  .segment "VARIABLES"
  .ident(.concat(.string(label), "_VALUE")):
  .word init
.endmacro

; The same as a variable, except it is allocated in the zero
; page.
.macro defvarzp name, init, label
  defword name, 0, label
    push .ident(.concat(.string(label), "_VALUE"))
    rts
  .segment "ZEROPAGE": zeropage
  .ident(.concat(.string(label), "_VALUE")):
  .word init
.endmacro

; A constant is a word which pushes its value onto the stack
; when it is executed.
.macro defconst name, value, label
  defword name, 0, label
    push value
    rts
.endmacro

.macro defval name, init, label
  defword name, 0, label
    dex
    dex
    lda .ident(.concat(.string(label), "_VALUE"))
    sta Stack, x
    lda .ident(.concat(.string(label), "_VALUE")) + 1
    sta Stack + 1, x
    rts
  .segment "VARIABLES"
    .ident(.concat(.string(label), "_VALUE")):
    .word init
.endmacro

.macro defcval name, init, label
  defword name, 0, label
    dex
    dex
    lda .ident(.concat(.string(label), "_VALUE"))
    sta Stack, x
    lda #0
    sta Stack + 1, x
    rts
  .segment "VARIABLES"
  .ident(.concat(.string(label), "_VALUE")):
    .byte init
.endmacro

.endif