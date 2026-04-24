build:
    ccpsx -O3 -Xo$80010000 ./SRC/*.c -omain.cpe,main.sym,mem.map
    cpe2x /ce main.cpe

