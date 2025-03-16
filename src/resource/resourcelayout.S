
# prong resource type (version 2.7)
# the basic format is this..
# label: <- internal identifier
#    .asciz "<rtype> + <rlocation>"     <- 'a' for ascii, 'b' for binary, and 'e' for external.  e can be used with a and b
#    .asciz "<name>"    <- global identifier
#    .incbin "<path>"   <- the file itself
#    .byte 0            <- if this is text, have a byte 0
#    it must be followed IMMEDIATELY by another label
#    for external resources (those loaded from a file) replace the incbin with an asciz for the filepath you wish to use


spsmfrag:
    .asciz "-a -F"
    .asciz "qspFS" 
    .asciz "shaders/qspm-fragment.frag.txt"
    .incbin "qspm-fragment.frag.txt"
    .byte 0

spsmvert:
    .asciz "-a -F"
    .asciz "qspVS"
    .asciz "shaders/qspm-vertex.vert.txt"
    .incbin "qspm-vertex.vert.txt"
    .byte 0

sav:
    .asciz "-a -F"
    .asciz "qspAVS"
    .asciz "shaders/qspm-alias.vert.txt"
    .incbin "qspm-alias.vert.txt"
    .byte 0
saf:
    .asciz "-a -F"
    .asciz "qspAFS"
    .asciz "shaders/qspm-alias.frag.txt"
    .incbin "qspm-alias.frag.txt"
    .byte 0

spsmfrag_w:
    .asciz "-a -F"
    .asciz "qspFS_w" 
    .asciz "shaders/qspm-fragment-warp.frag.txt"
    .incbin "qspm-fragment-warp.frag.txt"
    .byte 0

spsmvert_w:
    .asciz "-a -F"
    .asciz "qspVS_w"
    .asciz "shaders/qspm-vertex-warp.vert.txt"
    .incbin "qspm-vertex-warp.vert.txt"
    .byte 0

spsmfrag_s:
    .asciz "-a -F"
    .asciz "qspFS_s" 
    .asciz "shaders/qspm-fragment-sky.frag.txt"
    .incbin "qspm-fragment-warp.frag.txt"
    .byte 0

spsmvert_s:
    .asciz "-a -F"
    .asciz "qspVS_s"
    .asciz "shaders/qspm-vertex-sky.vert.txt"
    .incbin "qspm-vertex-warp.vert.txt"
    .byte 0

resource_end:
    .asciz "-b"
    .asciz "rs-end"
    .byte 0



.global	resource_table
resource_table:
    .quad spsmfrag
    .quad spsmvert
    .quad sav # alias vertex shader
    .quad saf # alias fragment shader
    .quad spsmfrag_w
    .quad spsmvert_w
    .quad spsmfrag_s
    .quad spsmvert_s


    .quad resource_end # end
    .quad 0