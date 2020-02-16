.MODEL MEDIUM, C

EXTRN status:WORD, primAd:WORD, backAd:WORD

.CODE

PUBLIC OPLWriteImpl
OPLWriteImpl PROC FAR C argument:DWORD, regnum:WORD, regval:BYTE

mov bx,regnum
mov cl,regval
cmp status,0        ; if status == 0, go to OPL3
je three
cmp status,2        ; if status == DUAL, also go to OPL3
jne two             ; otherwise, go to OPL2

three:
mov dx,primAd       ; put OPL3 primary bank in dx
cmp bx,100h         ; check if regnum <= to threshold (256)
jle prim            ; if so, go straight to primary bank
    add dx,2        ; otherwise, writing to secondary bank (primary+2)
prim:
    mov al,bl
    mov ah,cl
    out dx,ax       ; output number and value word
    cmp status,2
    jne done
        add dx,2
        out dx,ax
    jmp done        ; finished
two:
    mov dx,backAd
    mov al,bl
    out dx,al
    mov cx,6
loopOne:
    in al,dx
    loop loopOne

    inc dx
    mov al,regval   
    out dx,al
    dec dx          ; go back to register address
    mov cx,36
loopTwo:
    in al,dx
    loop loopTwo
done:
ret             ; return to C
OPLWriteImpl ENDP
END