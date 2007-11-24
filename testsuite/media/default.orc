; Default orchestra file for csound-generated audio
;
;

	sr = 44100
	kr = 100
	ksmps = 441
	nchnls = 1

	instr	1
k1      linen   p4, 0.05, p3, 0.08			; envelope
a3	oscil	k1, 3*cpspch(p5), 1			; sine wave at freq * 3
	out	a3
	endin

