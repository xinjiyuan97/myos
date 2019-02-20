; my-os
; TAB=4

CYLS EQU 10

    ORG     0x7C00          ;起始地址
    JMP     start           ;短转移
    DB      0x90            ;无意义
    DB      "MY OS   "      ; BS_OEMName
    DW      512             ; BPB_BytesPerSec
    DB      1               ; BPB_SecPerClus
    DW      1               ; BPB_ResvdSecCnt
    DB      2               ; BPB_NumFATs
    DW      224             ; BPB_RootEntCnt
    DW      2880            ; BPB_TotSec16
    DB      0xF0            ; BPB_Media
    DW      9               ; BPB_FATSz16
    DW      18              ; BPB_SecPerTrk
    DW      2               ; BPB_NumHeads
    DD      0               ; BPB_HiddSec
    DD      2880            ; BPB_TotSec32
    DB      0,0,0x29        ; BS_DrvNum
    DD      0xffffffff      ; BS_VolID
    DB      "HELLO-OS   "   ; BS_VolLab
    DB      "FAT12   "      ; BS_FileSysType
    RESB    18              ; 无意义



start:
    MOV AX, 0
    MOV SS, AX
    MOV SP, 0x7C00
    MOV DS, AX

    MOV AX, 0x0820
    MOV ES, AX
    MOV CH, 0   ; 柱面
    MOV DH, 0   ; 磁头
    MOV CL, 2   ; 扇区
read:
    MOV SI, 0
retry:
    MOV AL, 1
    MOV AH, 0x02
    MOV BX, 0
    MOV DL, 0x00
    INT 0x13
    JNC next
    INC SI
    CMP SI, 5
    JAE error
    MOV AH, 0x00
    MOV DL, 0x00
    INT 0x13
    JMP retry
next:
    MOV AX, ES
    ADD AX, 0x0020
    MOV ES, AX
    INC CL
    CMP CL, 18
    JBE read
    MOV CL, 1
    INC DH
    CMP DH, 2
    JB read
    MOV DH, 0
    INC CH
    CMP CH, CYLS
    JB read
    ;JMP error

    MOV [0x0FF0], CH
    JMP 0xC200

    
final:
    HLT
    JMP final

error:
    MOV SI, msg
putloop:
    MOV AL, [SI]
    INC SI
    CMP AL, 0
    JE final
    MOV AH, 0x0E
    MOV BX, 15
    INT 0x10
    JMP putloop

msg:
    DB 0x0a, 0x0a
    DB "load error"
    DB 0x0a
    DB 0

    RESB 510 - ($ - $$)
    DB 0x55, 0xaa
