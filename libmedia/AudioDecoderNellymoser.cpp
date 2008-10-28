// AudioDecoderNellymoser.cpp: Nellymoser decoding
// 
//   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//


// This file incorporates work covered by the following copyright and permission
// notice:


/*
 * Copyright (c) 2007 a840bda5870ba11f19698ff6eb9581dfb0f95fa5,
 *                    539459aeb7d425140b62a3ec7dbf6dc8e408a306, and
 *                    520e17cd55896441042b14df2566a6eb610ed444
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "AudioDecoderNellymoser.h"
#include "SoundInfo.h"
#include "log.h"

#include "VM.h" // for randonNumberGenerator

#include <ctime>
#include <cmath>

namespace gnash {
namespace media {

float nelly_neg_unpack_table[64] = {
-0.0061359000, -0.0306748003, -0.0551952012, -0.0796824023, -0.1041216031,
-0.1284981072, -0.1527972072, -0.1770042032, -0.2011045963, -0.2250839025,
-0.2489275932, -0.2726213932, -0.2961508930, -0.3195019960, -0.3426606953,
-0.3656130135, -0.3883450031, -0.4108431935, -0.4330937862, -0.4550836086,
-0.4767991900, -0.4982276857, -0.5193560123, -0.5401715040, -0.5606616139,
-0.5808140039, -0.6006165147, -0.6200572252, -0.6391243935, -0.6578066945,
-0.6760926843, -0.6939715147, -0.7114322186, -0.7284644246, -0.7450578213,
-0.7612023950, -0.7768884897, -0.7921066284, -0.8068475723, -0.8211025000,
-0.8348628879, -0.8481202722, -0.8608669043, -0.8730949759, -0.8847970963,
-0.8959661722, -0.9065957069, -0.9166790843, -0.9262102246, -0.9351835251,
-0.9435935020, -0.9514350295, -0.9587035179, -0.9653943777, -0.9715039134,
-0.9770280719, -0.9819638729, -0.9863080978, -0.9900581837, -0.9932119250,
-0.9957674146, -0.9977231026, -0.9990776777, -0.9998306036
};

float nelly_huff_table[127] = {
0.0000000000,

-0.8472560048, 0.7224709988,

-1.5247479677, -0.4531480074, 0.3753609955, 1.4717899561,

-1.9822579622, -1.1929379702, -0.5829370022, -0.0693780035, 0.3909569979,
0.9069200158, 1.4862740040, 2.2215409279,

-2.3887870312, -1.8067539930, -1.4105420113, -1.0773609877, -0.7995010018,
-0.5558109879, -0.3334020078, -0.1324490011, 0.0568020009, 0.2548770010,
0.4773550034, 0.7386850119, 1.0443060398, 1.3954459429, 1.8098750114,
2.3918759823,

-2.3893830776, -1.9884680510, -1.7514040470, -1.5643119812, -1.3922129869,
-1.2164649963, -1.0469499826, -0.8905100226, -0.7645580173, -0.6454579830,
-0.5259280205, -0.4059549868, -0.3029719889, -0.2096900046, -0.1239869967,
-0.0479229987, 0.0257730000, 0.1001340002, 0.1737180054, 0.2585540116,
0.3522900045, 0.4569880068, 0.5767750144, 0.7003160119, 0.8425520062,
1.0093879700, 1.1821349859, 1.3534560204, 1.5320819616, 1.7332619429,
1.9722349644, 2.3978140354,


-2.5756309032, -2.0573320389, -1.8984919786, -1.7727810144, -1.6662600040,
-1.5742180347, -1.4993319511, -1.4316639900, -1.3652280569, -1.3000990152,
-1.2280930281, -1.1588579416, -1.0921250582, -1.0135740042, -0.9202849865,
-0.8287050128, -0.7374889851, -0.6447759867, -0.5590940118, -0.4857139885,
-0.4110319912, -0.3459700048, -0.2851159871, -0.2341620028, -0.1870580018,
-0.1442500055, -0.1107169986, -0.0739680007, -0.0365610011, -0.0073290002,
0.0203610007, 0.0479039997, 0.0751969963, 0.0980999991, 0.1220389977,
0.1458999962, 0.1694349945, 0.1970459968, 0.2252430022, 0.2556869984,
0.2870100141, 0.3197099864, 0.3525829911, 0.3889069855, 0.4334920049,
0.4769459963, 0.5204820037, 0.5644530058, 0.6122040153, 0.6685929894,
0.7341650128, 0.8032159805, 0.8784040213, 0.9566209912, 1.0397069454,
1.1293770075, 1.2211159468, 1.3080279827, 1.4024800062, 1.5056819916,
1.6227730513, 1.7724959850, 1.9430880547, 2.2903931141
};

float nelly_pos_unpack_table[64] = {
0.9999812245, 0.9995294213, 0.9984756112, 0.9968202710, 0.9945645928,
0.9917098284, 0.9882575870, 0.9842100739, 0.9795697927, 0.9743394256,
0.9685220718, 0.9621214271, 0.9551411867, 0.9475855827, 0.9394592047,
0.9307669997, 0.9215139747, 0.9117059708, 0.9013488293, 0.8904486895,
0.8790122271, 0.8670461774, 0.8545579910, 0.8415549994, 0.8280450106,
0.8140363097, 0.7995373011, 0.7845566273, 0.7691032887, 0.7531868219,
0.7368165851, 0.7200024724, 0.7027546763, 0.6850836873, 0.6669998765,
0.6485143900, 0.6296381950, 0.6103827953, 0.5907596946, 0.5707806945,
0.5504580140, 0.5298035741, 0.5088300705, 0.4875501990, 0.4659765065,
0.4441221058, 0.4220002890, 0.3996241987, 0.3770073950, 0.3541634977,
0.3311063051, 0.3078495860, 0.2844074965, 0.2607941031, 0.2370236069,
0.2131102979, 0.1890687048, 0.1649131030, 0.1406581998, 0.1163185984,
0.0919089988, 0.0674438998, 0.0429382995, 0.0184067003
};

int nelly_copy_count[23] = {
2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 4, 4, 5, 6, 6, 7, 8, 9, 10, 12, 14, 15
};

float nelly_signal_table[64] = {
0.1250000000, 0.1249623969, 0.1248494014, 0.1246612966, 0.1243980974,
0.1240599006, 0.1236471012, 0.1231596991, 0.1225982010, 0.1219628006,
0.1212539002, 0.1204719990, 0.1196174994, 0.1186909974, 0.1176929995,
0.1166241020, 0.1154849008, 0.1142762005, 0.1129987016, 0.1116530001,
0.1102401987, 0.1087609008, 0.1072160974, 0.1056066975, 0.1039336994,
0.1021981016, 0.1004009023, 0.0985433012, 0.0966262966, 0.0946511030,
0.0926188976, 0.0905309021, 0.0883883014, 0.0861926004, 0.0839449018,
0.0816465989, 0.0792991966, 0.0769039020, 0.0744623989, 0.0719759986,
0.0694463030, 0.0668746978, 0.0642627999, 0.0616123006, 0.0589246005,
0.0562013984, 0.0534444004, 0.0506552011, 0.0478353985, 0.0449868999,
0.0421111993, 0.0392102003, 0.0362856016, 0.0333391018, 0.0303725004,
0.0273876991, 0.0243862998, 0.0213702004, 0.0183412991, 0.0153013002,
0.0122520998, 0.0091955997, 0.0061335000, 0.0030677000
};

short nelly_init_table[64] = {
3134, 5342, 6870, 7792, 8569, 9185, 9744, 10191, 10631, 11061, 11434, 11770,
12116, 12513, 12925, 13300, 13674, 14027, 14352, 14716, 15117, 15477, 15824,
16157, 16513, 16804, 17090, 17401, 17679, 17948, 18238, 18520, 18764, 19078,
19381, 19640, 19921, 20205, 20500, 20813, 21162, 21465, 21794, 22137, 22453,
22756, 23067, 23350, 23636, 23926, 24227, 24521, 24819, 25107, 25414, 25730,
26120, 26497, 26895, 27344, 27877, 28463, 29426, 31355
};

float nelly_state_table[128] = {
0.0061359000, 0.0184067003, 0.0306748003, 0.0429382995, 0.0551952012,
0.0674438998, 0.0796824023, 0.0919089988, 0.1041216031, 0.1163185984,
0.1284981072, 0.1406581998, 0.1527972072, 0.1649131030, 0.1770042032,
0.1890687048, 0.2011045963, 0.2131102979, 0.2250839025, 0.2370236069,
0.2489275932, 0.2607941031, 0.2726213932, 0.2844074965, 0.2961508930,
0.3078495860, 0.3195019960, 0.3311063051, 0.3426606953, 0.3541634977,
0.3656130135, 0.3770073950, 0.3883450031, 0.3996241987, 0.4108431935,
0.4220002890, 0.4330937862, 0.4441221058, 0.4550836086, 0.4659765065,
0.4767991900, 0.4875501990, 0.4982276857, 0.5088300705, 0.5193560123,
0.5298035741, 0.5401715040, 0.5504580140, 0.5606616139, 0.5707806945,
0.5808140039, 0.5907596946, 0.6006165147, 0.6103827953, 0.6200572252,
0.6296381950, 0.6391243935, 0.6485143900, 0.6578066945, 0.6669998765,
0.6760926843, 0.6850836873, 0.6939715147, 0.7027546763, 0.7114322186,
0.7200024724, 0.7284644246, 0.7368165851, 0.7450578213, 0.7531868219,
0.7612023950, 0.7691032887, 0.7768884897, 0.7845566273, 0.7921066284,
0.7995373011, 0.8068475723, 0.8140363097, 0.8211025000, 0.8280450106,
0.8348628879, 0.8415549994, 0.8481202722, 0.8545579910, 0.8608669043,
0.8670461774, 0.8730949759, 0.8790122271, 0.8847970963, 0.8904486895,
0.8959661722, 0.9013488293, 0.9065957069, 0.9117059708, 0.9166790843,
0.9215139747, 0.9262102246, 0.9307669997, 0.9351835251, 0.9394592047,
0.9435935020, 0.9475855827, 0.9514350295, 0.9551411867, 0.9587035179,
0.9621214271, 0.9653943777, 0.9685220718, 0.9715039134, 0.9743394256,
0.9770280719, 0.9795697927, 0.9819638729, 0.9842100739, 0.9863080978,
0.9882575870, 0.9900581837, 0.9917098284, 0.9932119250, 0.9945645928,
0.9957674146, 0.9968202710, 0.9977231026, 0.9984756112, 0.9990776777,
0.9995294213, 0.9998306036, 0.9999812245
};

short nelly_delta_table[32] = {
-11725, -9420, -7910, -6801, -5948, -5233, -4599, -4039, -3507, -3030, -2596,
-2170, -1774, -1383, -1016, -660, -329, -1, 337, 696, 1085, 1512, 1962, 2433,
2968, 3569, 4314, 5279, 6622, 8154, 10076, 12975
};

float nelly_inv_dft_table[129] = {
0.0000000000, 0.0122715384, 0.0245412290, 0.0368072242, 0.0490676723,
0.0613207370, 0.0735645667, 0.0857973099, 0.0980171412, 0.1102222130,
0.1224106774, 0.1345807165, 0.1467304677, 0.1588581353, 0.1709618866,
0.1830398887, 0.1950903237, 0.2071113735, 0.2191012353, 0.2310581058,
0.2429801822, 0.2548656464, 0.2667127550, 0.2785196900, 0.2902846932,
0.3020059466, 0.3136817515, 0.3253102899, 0.3368898630, 0.3484186828,
0.3598950505, 0.3713171780, 0.3826834261, 0.3939920366, 0.4052413106,
0.4164295495, 0.4275550842, 0.4386162460, 0.4496113360, 0.4605387151,
0.4713967443, 0.4821837842, 0.4928981960, 0.5035383701, 0.5141027570,
0.5245896578, 0.5349976420, 0.5453249812, 0.5555702448, 0.5657318234,
0.5758081675, 0.5857978463, 0.5956993103, 0.6055110693, 0.6152315736,
0.6248595119, 0.6343932748, 0.6438315511, 0.6531728506, 0.6624158025,
0.6715589762, 0.6806010008, 0.6895405650, 0.6983762383, 0.7071067691,
0.7157308459, 0.7242470980, 0.7326542735, 0.7409511209, 0.7491363883,
0.7572088242, 0.7651672959, 0.7730104327, 0.7807372212, 0.7883464098,
0.7958369255, 0.8032075167, 0.8104572296, 0.8175848126, 0.8245893121,
0.8314695954, 0.8382247090, 0.8448535800, 0.8513551950, 0.8577286005,
0.8639728427, 0.8700869679, 0.8760700822, 0.8819212317, 0.8876396418,
0.8932242990, 0.8986744881, 0.9039893150, 0.9091680050, 0.9142097831,
0.9191138744, 0.9238795042, 0.9285060763, 0.9329928160, 0.9373390079,
0.9415440559, 0.9456073046, 0.9495281577, 0.9533060193, 0.9569403529,
0.9604305029, 0.9637760520, 0.9669764638, 0.9700312614, 0.9729399681,
0.9757021070, 0.9783173800, 0.9807852507, 0.9831054807, 0.9852776527,
0.9873014092, 0.9891765118, 0.9909026623, 0.9924795032, 0.9939069748,
0.9951847196, 0.9963126183, 0.9972904325, 0.9981181026, 0.9987954497,
0.9993223548, 0.9996988177, 0.9999247193, 1.0000000000
};

unsigned char nelly_center_table[64] = {
0, 64, 32, 96, 16, 80, 48, 112, 8, 72, 40, 104, 24, 88, 56, 120,
4, 68, 36, 100, 20, 84, 52, 116, 12, 76, 44, 108, 28, 92, 60, 124,
2, 66, 34, 98, 18, 82, 50, 114, 10, 74, 42, 106, 26, 90, 58, 122,
6, 70, 38, 102, 22, 86, 54, 118, 14, 78, 46, 110, 30, 94, 62, 126
};

static void center(float *audio)
{
	int i, j;
	float ftmp;

	for (i = 0; i < NELLY_BUF_LEN; i+=2) {
		j = nelly_center_table[i/2];
		if (j > i) {
			ftmp = audio[j];
			audio[j] = audio[i];
			audio[i] = ftmp;
			ftmp = audio[j+1];
			audio[j+1] = audio[i+1];
			audio[i+1] = ftmp;
		}
	}
}

static void inverse_dft(float *audio)
{
	int i, j, k, advance;
	float *aptr, a, b, c, d, e, f;

	aptr = audio;
	for (i = 0; i < NELLY_BUF_LEN/4; i++) {
		a = *aptr;
		b = *(aptr+2);
		c = *(aptr+1);
		d = *(aptr+3);

		*(aptr+2) = a-b;
		*aptr = a+b;
		*(aptr+3) = c-d;
		*(aptr+1) = c+d;

		aptr += 4;
	}

	aptr = audio;
	for (i = 0; i < NELLY_BUF_LEN/8; i++) {
		a = *aptr;
		b = *(aptr+4);
		c = *(aptr+1);
		d = *(aptr+5);

		*(aptr+4) = a-b;
		*(aptr+5) = c-d;
		*aptr = a+b;
		*(aptr+1) = c+d;

		aptr += 2;

		a = *aptr;
		b = *(aptr+5);
		c = *(aptr+1);
		d = *(aptr+4);

		*(aptr+4) = a-b;
		*aptr = a+b;
		*(aptr+5) = c+d;
		*(aptr+1) = c-d;

		aptr += 6;
	}

	i = 0;
	for (advance = 8; advance < NELLY_BUF_LEN; advance *= 2) {
		aptr = audio;

		for (k = 0; k < NELLY_BUF_LEN/(2*advance); k++) {
			for (j = 0; j < advance/4; j++) {
				a = nelly_inv_dft_table[128-i];
				b = *(aptr+advance);
				c = nelly_inv_dft_table[i];
				d = *(aptr+advance+1);
				e = *aptr;
				f = *(aptr+1);

				*(aptr+advance) = e-(a*b+c*d);
				*aptr = e+(a*b+c*d);
				*(aptr+advance+1) = f+(b*c-a*d);
				*(aptr+1) = f-(b*c-a*d);

				i += 512/advance;
				aptr += 2;
			}

			for (j = 0; j < advance/4; j++) {
				a = nelly_inv_dft_table[128-i];
				b = *(aptr+advance);
				c = nelly_inv_dft_table[i];
				d = *(aptr+advance+1);
				e = *aptr;
				f = *(aptr+1);

				*(aptr+advance) = e+(a*b-c*d);
				*aptr = e-(a*b-c*d);
				*(aptr+advance+1) = f+(a*d+b*c);
				*(aptr+1) = f-(a*d+b*c);

				i -= 512/advance;
				aptr += 2;
			}

			aptr += advance;
		}
	}
}

static void unpack_coeffs(float *buf, float *audio)
{
	int i, end, mid_hi, mid_lo;
	float a, b, c, d, e, f;

	end = NELLY_BUF_LEN-1;
	mid_hi = NELLY_BUF_LEN/2;
	mid_lo = mid_hi-1;

	for (i = 0; i < NELLY_BUF_LEN/4; i++) {
		a = buf[end-(2*i)];
		b = buf[2*i];
		c = buf[(2*i)+1];
		d = buf[end-(2*i)-1];
		e = nelly_pos_unpack_table[i];
		f = nelly_neg_unpack_table[i];

		audio[2*i] = b*e-a*f;
		audio[(2*i)+1] = a*e+b*f;

		a = nelly_neg_unpack_table[mid_lo-i];
		b = nelly_pos_unpack_table[mid_lo-i];

		audio[end-(2*i)-1] = b*d-a*c;
		audio[end-(2*i)] = b*c+a*d;
	}
}

static void complex2signal(float *audio)
{
	int i, end, mid_hi, mid_lo;
	float *aptr, *sigptr, a, b, c, d, e, f, g;

	end = NELLY_BUF_LEN-1;
	mid_hi = NELLY_BUF_LEN/2;
	mid_lo = mid_hi-1;

	a = audio[end];
	b = audio[end-1];
	c = audio[1];
	d = nelly_signal_table[0];
	e = audio[0];
	f = nelly_signal_table[mid_lo];
	g = nelly_signal_table[1];

	audio[0] = d*e;
	audio[1] = b*g-a*f;
	audio[end-1] = a*g+b*f;
	audio[end] = c*(-d);

	aptr = audio+end-2;
	sigptr = nelly_signal_table+mid_hi-1;

	for (i = 3; i < NELLY_BUF_LEN/2; i += 2) {
		a = audio[i-1];
		b = audio[i];
		c = nelly_signal_table[i/2];
		d = *sigptr;
		e = *(aptr-1);
		f = *aptr;

		audio[i-1] = a*c+b*d;
		*aptr = a*d-b*c;

		a = nelly_signal_table[(i/2)+1];
		b = *(sigptr-1);

		*(aptr-1) = b*e+a*f;
		audio[i] = a*e-b*f;

		sigptr--;
		aptr -= 2;
	}
}

static void apply_state(float *state, float *audio)
{
	int bot, mid_up, mid_down, top;
	float s_bot, s_top;
	float *t = nelly_state_table;

	bot = 0;
	top = NELLY_BUF_LEN-1;
	mid_up = NELLY_BUF_LEN/2;
	mid_down = (NELLY_BUF_LEN/2)-1;

	while (bot < NELLY_BUF_LEN/4) {
		s_bot = audio[bot];
		s_top = audio[top];

		audio[bot] = audio[mid_up]*t[bot]+state[bot]*t[top];
		audio[top] = state[bot]*t[bot]-audio[mid_up]*t[top];
		state[bot] = -audio[mid_down];

		audio[mid_down] = s_top*t[mid_down]+state[mid_down]*t[mid_up];
		audio[mid_up] = state[mid_down]*t[mid_down]-s_top*t[mid_up];
		state[mid_down] = -s_bot;

		bot++;
		mid_up++;
		mid_down--;
		top--;
	}
}

static int sum_bits(short *buf, short shift, short off)
{
	int b, i = 0, ret = 0;

	for (i = 0; i < NELLY_FILL_LEN; i++) {
		b = buf[i] - off;
		if (b < 0)
			b = 0;
		b = ((b>>(shift-1))+1)>>1;
		if (b > NELLY_BIT_CAP)
			ret += NELLY_BIT_CAP;
		else
			ret += b;
	}

	return ret;
}

static int headroom(int *la, short *sa)
{
	if (*la == 0)
		*sa += 31;
	else if (*la < 0) {
		while (*la > -1<<30) {
			*la <<= 1;
			(*sa)++;
		}
	} else {
		while (*la < 1<<30) {
			*la <<= 1;
			(*sa)++;
		}
	}

	return *la;
}

static void get_sample_bits(float *buf, int *bits)
{
	int i = 0, j;
	short sbuf[128];
	int bitsum = 0, last_bitsum, small_bitsum, big_bitsum;
	short shift = -16, shift_saved;
	int tmp = 0;
	int big_off;
	int off, diff;

	for (; i < NELLY_FILL_LEN; i++) {
		if (buf[i] > tmp)
			tmp = static_cast<int>(buf[i]);
	}

	headroom(&tmp, &shift);

	if (shift < 0)
		for (i = 0; i < NELLY_FILL_LEN; i++)
			sbuf[i] = ((int)buf[i]) >> -shift;
	else
		for (i = 0; i < NELLY_FILL_LEN; i++)
			sbuf[i] = ((int)buf[i]) << shift;

	for (i = 0; i < NELLY_FILL_LEN; i++)
		sbuf[i] = (3*sbuf[i])>>2;

	tmp = 0;
	for (i = 0; i < NELLY_FILL_LEN; i++)
		tmp += sbuf[i];

	shift += 11;
	shift_saved = shift;
	tmp -= NELLY_DETAIL_BITS << shift;
	headroom(&tmp, &shift);
	off = (NELLY_BASE_OFF * (tmp>>16)) >> 15;
	shift = shift_saved - (NELLY_BASE_SHIFT+shift-31);

	if (shift < 0)
		off >>= -shift;
	else
		off <<= shift;

	bitsum = sum_bits(sbuf, shift_saved, off);

	if (bitsum != NELLY_DETAIL_BITS) {
		shift = 0;
		diff = bitsum - NELLY_DETAIL_BITS;

		if (diff > 0) {
			while (diff <= 16383) {
				shift++;
				diff *= 2;
			}
		} else {
			while (diff >= -16383) {
				shift++;
				diff *= 2;
			}
		}

		diff = (diff * NELLY_BASE_OFF) >> 15;
		shift = shift_saved-(NELLY_BASE_SHIFT+shift-15);

		if (shift > 0) {
			diff <<= shift;
		} else {
			diff >>= -shift;
		}

		for (j = 1; j < 20; j++) {
			tmp = off;
			off += diff;
			last_bitsum = bitsum;

			bitsum = sum_bits(sbuf, shift_saved, off);

			if ((bitsum-NELLY_DETAIL_BITS) * (last_bitsum-NELLY_DETAIL_BITS) <= 0)
				break;
		}

		if (bitsum != NELLY_DETAIL_BITS) {
			if (bitsum > NELLY_DETAIL_BITS) {
				big_off = off;
				off = tmp;
				big_bitsum=bitsum;
				small_bitsum=last_bitsum;
			} else {
				big_off = tmp;
				big_bitsum=last_bitsum;
				small_bitsum=bitsum;
			}

			while (bitsum != NELLY_DETAIL_BITS && j <= 19) {
				diff = (big_off+off)>>1;
				bitsum = sum_bits(sbuf, shift_saved, diff);
				if (bitsum > NELLY_DETAIL_BITS) {
					big_off=diff;
					big_bitsum=bitsum;
				} else {
					off = diff;
					small_bitsum=bitsum;
				}
				j++;
			}

			if (abs(big_bitsum-NELLY_DETAIL_BITS) >=
			    abs(small_bitsum-NELLY_DETAIL_BITS)) {
				bitsum = small_bitsum;
			} else {
				off = big_off;
				bitsum = big_bitsum;
			}

		}
	}

	for (i = 0; i < NELLY_FILL_LEN; i++) {
		tmp = sbuf[i]-off;
		if (tmp < 0)
			tmp = 0;
		else
			tmp = ((tmp>>(shift_saved-1))+1)>>1;

		if (tmp > NELLY_BIT_CAP)
			tmp = NELLY_BIT_CAP;
		bits[i] = tmp;
	}

	if (bitsum > NELLY_DETAIL_BITS) {
		tmp = i = 0;
		while (tmp < NELLY_DETAIL_BITS) {
			tmp += bits[i];
			i++;
		}

		tmp -= bits[i-1];
		bits[i-1] = NELLY_DETAIL_BITS-tmp;
		bitsum = NELLY_DETAIL_BITS;
		while (i < NELLY_FILL_LEN) {
			bits[i] = 0;
			i++;
		}
	}
}

static unsigned char get_bits(unsigned char block[NELLY_BLOCK_LEN], int *off, int n)
{
	char ret;
	int boff = *off/8, bitpos = *off%8, mask = (1<<n)-1;

	if (bitpos+n > 8) {
		ret = block[boff%NELLY_BLOCK_LEN] >> bitpos;
		mask >>= 8-bitpos;
		ret |= (block[(boff+1)%NELLY_BLOCK_LEN] & mask) << (8-bitpos);
	} else {
		ret = (block[boff%NELLY_BLOCK_LEN] >> bitpos) & mask;
	}
	
	*off += n;
	return ret;
}

static int
gimme_random()
{
	using namespace boost;
	VM::RNG& rnd = VM::get().randomNumberGenerator();

	uniform_int<> dist(0, std::numeric_limits<int>::max());
	variate_generator<VM::RNG&, uniform_int<> > uni(rnd, dist);

	return uni();
}

static void nelly_decode_block(nelly_handle* nh, unsigned char block[NELLY_BLOCK_LEN], float audio[256])
{
	int i,j;
	float buf[NELLY_BUF_LEN], pows[NELLY_BUF_LEN];
	float *aptr, *bptr, *pptr, val, pval;
	int bits[NELLY_BUF_LEN];
	unsigned char v;
	int bit_offset = 0;

	bptr = buf;
	pptr = pows;
	val = nelly_init_table[get_bits(block, &bit_offset, 6)];
	for (i = 0; i < 23; i++) {
		if (i > 0)
			val += nelly_delta_table[get_bits(block, &bit_offset, 5)];
		pval = pow(2, val/2048);
		for (j = 0; j < nelly_copy_count[i]; j++) {
			*bptr = val;
			*pptr = pval;
			bptr++;
			pptr++;
		}

	}

	for (i = NELLY_FILL_LEN; i < NELLY_BUF_LEN; i++)
		buf[i] = pows[i] = 0.0;

	get_sample_bits(buf, bits);

	for (i = 0; i < 2; i++) {
		aptr = audio+i*128;
		bit_offset = NELLY_HEADER_BITS + i*NELLY_DETAIL_BITS;

		for (j = 0; j < NELLY_FILL_LEN; j++) {
			if (bits[j] <= 0) {
				buf[j] = M_SQRT1_2*pows[j];

        
				if (gimme_random() % 2)
					buf[j] *= -1.0;
			} else {
				v = get_bits(block, &bit_offset, bits[j]);
				buf[j] = nelly_huff_table[(1<<bits[j])-1+v]*pows[j];
			}
		}

		unpack_coeffs(buf, aptr);
		center(aptr);
		inverse_dft(aptr);
		complex2signal(aptr);
		apply_state(nh->state, aptr);
	}
}

static void nelly_util_floats2shorts(float audio[256], short shorts[256])
{
	int i;

	for (i = 0; i < 256; i++) {
		if (audio[i] >= 32767.0)
			shorts[i] = 32767;
		else if (audio[i] <= -32768.0)
			shorts[i] = -32768;
		else
			shorts[i] = (short)audio[i];
	}
}

static nelly_handle *nelly_get_handle()
{
	int i;
	nelly_handle *nh;

	nh = new nelly_handle;

	if (nh != NULL)
		for (i = 0; i < 64; i++)
			nh->state[i] = 0.0;

	return nh;
}

static void nelly_free_handle(nelly_handle *nh)
{
	delete nh;
}


AudioDecoderNellymoser::AudioDecoderNellymoser()
	:
	_sampleRate(0),
	_stereo(false)
{
	_nh = nelly_get_handle();
}

	
AudioDecoderNellymoser::AudioDecoderNellymoser(AudioInfo& info)
	:
	_sampleRate(0),
	_stereo(false)
{
    setup(info);
	_nh = nelly_get_handle();

    assert(info.type == FLASH); // or we'd have thrown an exception
    audioCodecType codec = (audioCodecType)info.codec;
  	log_debug(_("AudioDecoderNellymoser: initialized FLASH codec %s (%d)"),
		(int)codec, codec);
}


AudioDecoderNellymoser::AudioDecoderNellymoser(SoundInfo& info)
	:
	_sampleRate(0),
	_stereo(false)
{
    setup(info);
	_nh = nelly_get_handle();

    audioCodecType codec = info.getFormat();
  	log_debug(_("AudioDecoderNellymoser: initialized FLASH codec %s (%d)"),
		(int)codec, codec);
}


AudioDecoderNellymoser::~AudioDecoderNellymoser()
{
	nelly_free_handle(_nh);
}

void
AudioDecoderNellymoser::setup(SoundInfo& info)
{
	audioCodecType codec = info.getFormat();
    switch (codec)
    {
        case AUDIO_CODEC_NELLYMOSER:
        case AUDIO_CODEC_NELLYMOSER_8HZ_MONO:
            _sampleRate = info.getSampleRate();
            _stereo = info.isStereo();
            break;

        default:
            boost::format err = boost::format(
                _("AudioDecoderNellymoser: attempt to use with flash codec %d (%s)"))
                % (int)codec % codec;
            throw MediaException(err.str());
	}
}

void AudioDecoderNellymoser::setup(AudioInfo& info)
{
	if (info.type != FLASH)
    {
        boost::format err = boost::format(
            _("AudioDecoderNellymoser: unable to intepret custom audio codec id %s"))
            % info.codec;
        throw MediaException(err.str());
    }

	audioCodecType codec = static_cast<audioCodecType>(info.codec);
    switch (codec)
    {
        case AUDIO_CODEC_NELLYMOSER:
        case AUDIO_CODEC_NELLYMOSER_8HZ_MONO:
            _sampleRate = info.sampleRate;
            _stereo = info.stereo;
            break;

        default:
            boost::format err = boost::format(
                _("AudioDecoderNellymoser: attempt to use with flash codec %d (%s)"))
                % (int)codec % codec;
            throw MediaException(err.str());
	}
}

float*
AudioDecoderNellymoser::decode(boost::uint8_t* in_buf, boost::uint32_t inputSize, boost::uint32_t* outputSize)
{
        size_t out_buf_size = (inputSize / NELLY_BLOCK_LEN) * 256;
	float* out_buf = new float[out_buf_size];
	
	boost::uint8_t* input_ptr = in_buf;
	float* output_ptr = out_buf;

	while (inputSize > 0) {
		nelly_decode_block(_nh, input_ptr, output_ptr);
		input_ptr += NELLY_BLOCK_LEN;
		inputSize -= NELLY_BLOCK_LEN;
		output_ptr += 256;
	}
	
	*outputSize = out_buf_size;
	
	return out_buf;	
}

boost::uint8_t*
AudioDecoderNellymoser::decode(boost::uint8_t* input,
        boost::uint32_t inputSize, boost::uint32_t& outputSize,
        boost::uint32_t& decodedBytes, bool /*parse*/)
{

	float float_buf[256];
	boost::uint32_t out_buf_size = (inputSize / 64) * 256;
	boost::int16_t* out_buf = new boost::int16_t[out_buf_size];
	boost::int16_t* out_buf_start = out_buf;

	while (inputSize > 0) {
		nelly_decode_block(_nh, input, float_buf);
		nelly_util_floats2shorts(float_buf, out_buf);
		out_buf += 256;
		input += 64;
		inputSize -= 64;
	}
			
	boost::uint8_t* tmp_raw_buffer = reinterpret_cast<boost::uint8_t*>(out_buf_start);
	boost::uint32_t tmp_raw_buffer_size = out_buf_size * 2;
#if 0
	// If we need to convert samplerate or/and from mono to stereo...
	if (out_buf_size > 0 && (_sampleRate != 44100 || !_stereo)) {

		boost::int16_t* adjusted_data = 0;
		int	adjusted_size = 0;
		int sample_count = out_buf_size / (_stereo ? 2 : 1);

		// Convert to needed samplerate - this converter only support standard flash samplerates
		convert_raw_data(&adjusted_data, &adjusted_size, tmp_raw_buffer, sample_count, 0, 
				_sampleRate, _stereo,
				44100,  true /* stereo */);

		// Hopefully this wont happen
		if (!adjusted_data) {
			log_error(_("Error in sound sample conversion"));
			delete[] tmp_raw_buffer;
			outputSize = 0;
			decodedBytes = 0;
			return NULL;
		}

		// Move the new data to the sound-struct
		delete[] tmp_raw_buffer;
		tmp_raw_buffer = reinterpret_cast<boost::uint8_t*>(adjusted_data);
		tmp_raw_buffer_size = adjusted_size;

	} else {
#endif
		tmp_raw_buffer_size = out_buf_size;
#if 0
	}
#endif

	outputSize = tmp_raw_buffer_size;
	decodedBytes = inputSize;
	return tmp_raw_buffer;
}

} // gnash.media namespace 
} // namespace gnash

