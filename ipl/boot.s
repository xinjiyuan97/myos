; my-os
; TAB=4

CYLS EQU 10

    ORG     0x7C00



    JMP     entry
    DB      0x90
    DB      "HELLOIPL"      ; 扇区名称 8字节
    DW      512             ; 一个扇区的大小 必须是512
    DB      1               ; 集群大小 必须是1个扇区
    DW      1               ; FAT从哪里开始，一般取第一扇区
    DB      2               ; FAT的个数，必须为2
    DW      224             ; 根目录的大小，一般为224
    DW      2880            ; 此驱动器大小(必须为2880扇区)
    DB      0xF0            ; 介质类型(必须是0XF0)
    DW      9               ; FAT区域的长度（必须设置为9个扇区）
    DW      18              ; 必须是18
    DW      2               ; 
    DD      0               ; 
    DD      2880            ; 再写一次这个驱动器的大小
    DB      0,0,0x29        ; 
    DD      0xffffffff      ; 
    DB      "HELLO-OS   "   ; 磁盘名称(11字节)
    DB      "FAT12   "      ; 格式名称(8字节)
    RESB    18              ; 暂且空开18字节



entry:
    MOV AX, 0
    MOV SS, AX
    MOV SP, 0x7C00
    MOV DS, AX

    MOV AX, 0x0820
    MOV ES, AX
    MOV CH, 0   ; 柱面
    MOV DH, 0   ; 磁头
    MOV CL, 2   ; 扇区
readpool:
    MOV SI, 0
retry:
    MOV AH, 0x02
    MOV AL, 1
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
    JBE readpool
    MOV CL, 1
    INC DH
    CMP DH, 2
    JB readpool
    MOV DH, 0
    INC CH
    CMP CH, CYLS
    JB readpool
    ;JMP error

    MOV [0x0FF0], CH
    JMP 0xC200

    
fin:
    HLT
    JMP fin

error:
    MOV SI, msg
putloop:
    MOV AL, [SI]
    INC SI
    CMP AL, 0
    JE fin
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
