; Csound score file for a simple major scale
; This is just an initial example, we may generate
; all needed sounds...
;
; Generate a majorscale.wav with the following command:
;
;    csound --nodisplays --wave -o majorscale.wav default.orc majorscale.sco
;

f1 0 8192 10 1			; sine wave table
i1 0.0 1 5000 7.00              ; C (below middle)
i1 1.0 1 5000 7.04		; E
i1 2.0 1 5000 7.07		; G
e

