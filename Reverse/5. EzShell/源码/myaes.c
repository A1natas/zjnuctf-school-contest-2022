#include "myaes.h"

#define ERROR_FUNCEND(err_code) { \
		ret = err_code; \
		goto __func_end; \
	}

#define CHECK_RETURN(ret) \
		if ((ret) != 0) { \
			goto __func_end; \
		}

static u1 Actv_aes_tab_id = 0xff;
static u1 Actv_aes_tab_enc[AESTAB_SIZ];
static u1 Actv_aes_tab_dec[AESTAB_SIZ];
        /***
 * ikvce_aes.c: Implementation of AES algorithm, basically follow FIPS PUB 197
 *       -- The following modes of operation ("mop" for short) are supported:
 *          - ECB mode
 *          - CBC mode
 *          - CFB mode
 *          - CFB1 mode
 *          - CFB8 mode
 *          - OFB mode
 *
 *       -- All modes support key length 128, 192 and 256 bits;
 *          both encryption and decryption are supported
 *
 *       -- There are 2 highest-level APIs:
 *          - ikvce_aes_key_initialize
 *                 a) Set key and key length (do key expansion)
 *                 b) Set modes of operation
 *                 c) Set action (encrypt or decrypt)
 *                 d) Set initial vector if necessary
 *
 *          - ikvce_aes_crypt:
 *                 - encrypt or decrypt data, data should be of unit "block (16 bytes)"
 *                 - if data is NOT of unit block, the user should do padding (or something) first
 *
 *      -- There is sample code for these 2 APIs: aeststr.c
 */
typedef void (*aes_util_crypt_t) (
	u1 *out_buff,
	u1 *in_buff,
	u4 nblocks);

typedef struct aes_module_stat_s {
	u4 key_len;
	u4 mode;  /* Modes of operation */
	aes_util_crypt_t crypt;
	u1 init_vec[AES_BLK_SIZ];

} aes_module_stat_t;

static aes_module_stat_t Aes_module_stat;


/* AES loop-up tables for little-endian processor */
#define WORD8_TO_WORD32(b0, b1, b2, b3) (((u4) (b0) << 24 )| ((u4) (b1) << 16 )| ((u4) (b2) << 8) | (b3))

#define byte0(x) ((x) >> 24)
#define byte1(x) (((x) >> 16) & 0xFF)
#define byte2(x) (((x) >> 8) & 0xFF)
#define byte3(x) ((x) & 0xFF)

#define AES_BLKSIZ_WORDS   (4)   /* 4 words */
#define AES_BLKSIZ         (16)
#define WORDSIZ            (4)

static u4 CfbBits = 8;
static u4 Aes_key_len = 32;

/* Buffer (32 + 480 + 16 + 16 = 544 bytes) */
static u1 G_aes_module_buff[32];
static u1 G_aes_module_key_expanded_buff[480];
static u1 G_last_mixup[AES_BLKSIZ];
static u1 TempBuf[AES_BLKSIZ];

/***
 * aes_util_key_expansion, aes_util_encrypt and aes_util_decrypt are lowest level API
 * of AES algorithm implementation:
 *     -- aes_util_key_expansion: do key expansion (for both encrypting and decrypting round keys)
 *        - input: key, key length (16, 24 or 32 bytes), and round key buffer (480 bytes)
 *        - output: round key
 *        - no return vale and input check
 *
 *     -- aes_util_encrypt: encrypt 1 block (16 bytes) data
 *        - input: plaintext data, ciphertext buffer, round key, key length
 *        - output: ciphertext
 *        - no return vale and input check
 *
 *     -- aes_util_decrypt: decrypt 1 block (16 bytes) data
 *        - input: ciphertext data, plaintext buffer, round key, key length
 *        - output: plaintext
 *        - no return vale and input check
 */

static void aes_util_key_expansion(u4 *key, u4 *round_key, u4 key_len)
{
	u4 Temp; // temporary variable
	u4 i, j, k;
	u4 Nbround;// length of round_key in 32-bit word according to the key length
	u4 Steptest;//use to optimize the conditional tests
	u4 key_len_words;

	key_len_words = key_len / WORDSIZ;

	Nbround = 4 * (key_len_words + 7);

	i = 0;
	while (i < key_len_words) {
		round_key[i] = key[i];
		i++;
	}

	i = Steptest = key_len_words;
	k = 0;
	do {
		Temp = round_key[i - 1];
		//test if (i % key_len_words)==0
		if (key_len_words == Steptest) {
			Temp = (u4) WORD8_TO_WORD32(u1SBOX[byte3(Temp)],
			                            u1SBOX[byte0(Temp)],
						    u1SBOX[byte1(Temp)],
						    u1SBOX[byte2(Temp)]);
			Temp  ^= u4Rcon[k++];
		} else if ((key_len_words == 8) && (Steptest == 4)) {  // case key_len_words = 8 (AES_256) and i % 8 == 4
			Temp = (u4) WORD8_TO_WORD32(u1SBOX[byte0(Temp)],
			                            u1SBOX[byte1(Temp)],
						    u1SBOX[byte2(Temp)],
						    u1SBOX[byte3(Temp)]);
		}

		j = i - key_len_words;
		round_key[i++] = Temp ^ round_key[j];
		Steptest--;
		if (Steptest == 0) {
			Steptest = key_len_words;
		}
	} while (i < Nbround);

	// copy round key for decrypt
	for (i = 0; i < Nbround; i++) {
		round_key[i + 60]= round_key[i];
	}

	// apply inverse mixcolumn
	i = 4 * (key_len_words + 7) + 55;
	do {
		round_key[i] = Td3[u1SBOX[byte0(round_key[i])]] ^
		               Td2[u1SBOX[byte1(round_key[i])]] ^
			       Td1[u1SBOX[byte2(round_key[i])]] ^
			       Td0[u1SBOX[byte3(round_key[i])]];
		--i;
	} while (i > 63);
}


void aes_key_expansion(u4 *key, u4 key_len)
{
	aes_util_key_expansion(key, (u4 *) G_aes_module_key_expanded_buff, key_len);

	/* Setting AES module key length */
	Aes_key_len = key_len;
}


static void aes_util_encrypt(u4 *cipher, u4 *plain, u4 *round_key, u4 key_len)
{
	u4 t0, t1, t2, t3, s0, s1, s2, s3;  // temporary variables used to store the State
	u4 *round_key_p, *plain_p, *cipher_p;
	u4 nrounds;

	nrounds = key_len / WORDSIZ + 6;  /* 10, 12 or 14 */

	round_key_p = round_key;
	plain_p = plain;
	cipher_p = cipher;

	/* AddRoundKey */
	t0 = (*plain_p)     ^ (*round_key_p);
	t1 = (*(++plain_p)) ^ (*(++round_key_p));
	t2 = (*(++plain_p)) ^ (*(++round_key_p));
	t3 = (*(++plain_p)) ^ (*(++round_key_p));

	/* Middle (nrounds - 1) rounds */
	while (nrounds-- > 1) {
		/* Mixcolumns + Shiftrows + Subbytes */
		s0 = Te0[byte3(t0)] ^ Te1[byte2(t1)] ^ Te2[byte1(t2)] ^ Te3[byte0(t3)];
		s1 = Te0[byte3(t1)] ^ Te1[byte2(t2)] ^ Te2[byte1(t3)] ^ Te3[byte0(t0)];
		s2 = Te0[byte3(t2)] ^ Te1[byte2(t3)] ^ Te2[byte1(t0)] ^ Te3[byte0(t1)];
		s3 = Te0[byte3(t3)] ^ Te1[byte2(t0)] ^ Te2[byte1(t1)] ^ Te3[byte0(t2)];

		/* AddRoundKey */
		t0 = s0 ^ (*(++round_key_p));
		t1 = s1 ^ (*(++round_key_p));
		t2 = s2 ^ (*(++round_key_p));
		t3 = s3 ^ (*(++round_key_p));
	}

	/* Shiftrows + Subbytes */
	s0 = WORD8_TO_WORD32(u1SBOX[byte0(t3)], u1SBOX[byte1(t2)], u1SBOX[byte2(t1)], u1SBOX[byte3(t0)]);
	s1 = WORD8_TO_WORD32(u1SBOX[byte0(t0)], u1SBOX[byte1(t3)], u1SBOX[byte2(t2)], u1SBOX[byte3(t1)]);
	s2 = WORD8_TO_WORD32(u1SBOX[byte0(t1)], u1SBOX[byte1(t0)], u1SBOX[byte2(t3)], u1SBOX[byte3(t2)]);
	s3 = WORD8_TO_WORD32(u1SBOX[byte0(t2)], u1SBOX[byte1(t1)], u1SBOX[byte2(t0)], u1SBOX[byte3(t3)]);

	/* AddRoundKey */
	(*cipher_p)     = s0 ^ (*(++round_key_p));
	(*(++cipher_p)) = s1 ^ (*(++round_key_p));
	(*(++cipher_p)) = s2 ^ (*(++round_key_p));
	(*(++cipher_p)) = s3 ^ (*(++round_key_p));
}


static void aes_util_decrypt(u4 *plain, u4 *cipher, u4 *round_key, u4 key_len)
{
	u4 t0, t1, t2, t3, s0, s1, s2, s3;  // temporary variables used to store the State
	u4 *round_key_p, *cipher_p, *plain_p;
	u4 nrounds, key_len_words;

	key_len_words = key_len / WORDSIZ;

	nrounds = key_len_words + 6;  /* 10, 12 or 14 */

	round_key_p = round_key + (4 * (key_len_words + 7)) + 59;
	cipher_p = cipher + 3;
	plain_p = plain + 3;

	/* AddRoundKey */
	t3 = (*cipher_p)     ^ (*round_key_p);
	t2 = (*(--cipher_p)) ^ (*(--round_key_p));
	t1 = (*(--cipher_p)) ^ (*(--round_key_p));
	t0 = (*(--cipher_p)) ^ (*(--round_key_p));

	/* Middle (nrounds - 1) rounds */
	while (nrounds-- > 1) {
		/* InvMixcolumns + InvShiftrows + InvSubbyte */
		s0 = Td3[byte0(t1)] ^ Td2[byte1(t2)] ^ Td1[byte2(t3)] ^ Td0[byte3(t0)];
		s1 = Td3[byte0(t2)] ^ Td2[byte1(t3)] ^ Td1[byte2(t0)] ^ Td0[byte3(t1)];
		s2 = Td3[byte0(t3)] ^ Td2[byte1(t0)] ^ Td1[byte2(t1)] ^ Td0[byte3(t2)];
		s3 = Td3[byte0(t0)] ^ Td2[byte1(t1)] ^ Td1[byte2(t2)] ^ Td0[byte3(t3)];

		/* AddRoundKey */
		t3 = s3 ^ (*(--round_key_p));
		t2 = s2 ^ (*(--round_key_p));
		t1 = s1 ^ (*(--round_key_p));
		t0 = s0 ^ (*(--round_key_p));
	}

	/* InvSubbyte + InvShiftrows */
	s0 = WORD8_TO_WORD32(u1INVSBOX[byte0(t1)], u1INVSBOX[byte1(t2)], u1INVSBOX[byte2(t3)], u1INVSBOX[byte3(t0)]);
	s1 = WORD8_TO_WORD32(u1INVSBOX[byte0(t2)], u1INVSBOX[byte1(t3)], u1INVSBOX[byte2(t0)], u1INVSBOX[byte3(t1)]);
	s2 = WORD8_TO_WORD32(u1INVSBOX[byte0(t3)], u1INVSBOX[byte1(t0)], u1INVSBOX[byte2(t1)], u1INVSBOX[byte3(t2)]);
	s3 = WORD8_TO_WORD32(u1INVSBOX[byte0(t0)], u1INVSBOX[byte1(t1)], u1INVSBOX[byte2(t2)], u1INVSBOX[byte3(t3)]);

	/* AddRoundKey */
	*plain_p     = s3 ^ (*(--round_key_p));
	*(--plain_p) = s2 ^ (*(--round_key_p));
	*(--plain_p) = s1 ^ (*(--round_key_p));
	*(--plain_p) = s0 ^ (*(--round_key_p));
}

/***
 * The following functions implements encrypt/decrypt functionalities for each modes of operation,
 * multi-block data are supported
 *        -- aes_util_ecb_enc: ECB mode encryption
 *        -- aes_util_ecb_dec: ECB mode decryption
 *        -- aes_util_cbc_enc: CBC mode encryption
 *        -- aes_util_cbc_dec: CBC mode decryption
 *        -- aes_util_cfb_enc: CFB mode encryption (CFB, CFB1 and CFB8)
 *        -- aes_util_cfb_dec: CFB mode decryption (CFB, CFB1 and CFB8)
 *        -- aes_util_ofb_enc: OFB mode encryption
 *        -- aes_util_ofb_dec: OFB mode decryption
 */

void aes_util_ecb_enc(u1 *out_buff, u1 *in_buff, u4 nblocks)
{
	u4 *inblk_p, *outblk_p;

	inblk_p  = (u4 *) in_buff;
	outblk_p = (u4 *) out_buff;

	while (nblocks-- > 0) {
		aes_util_encrypt(outblk_p, inblk_p, (u4 *) G_aes_module_key_expanded_buff, Aes_key_len);
		inblk_p  += AES_BLKSIZ_WORDS;
		outblk_p += AES_BLKSIZ_WORDS;
	}
}

void aes_util_ecb_dec(u1 *out_buff, u1 *in_buff, u4 nblocks)
{
	u4 *inblk_p, *outblk_p;

	inblk_p  = (u4 *) in_buff;
	outblk_p = (u4 *) out_buff;

	while (nblocks-- > 0) {
		aes_util_decrypt(outblk_p, inblk_p, (u4 *) G_aes_module_key_expanded_buff, Aes_key_len);
		inblk_p  += AES_BLKSIZ_WORDS;
		outblk_p += AES_BLKSIZ_WORDS;
	}
}


void aes_util_cbc_enc(u1 *out_buff, u1 *in_buff, u4 nblocks)
{
	u4 *inblk_p, *outblk_p, *word_p1, *word_p2, block_size_words;

	inblk_p  = (u4 *) in_buff;
	outblk_p = (u4 *) out_buff;

	while (nblocks-- > 0) {
		memcpy(G_aes_module_buff, inblk_p, AES_BLKSIZ);

		block_size_words = AES_BLKSIZ_WORDS;
		word_p1 = (u4 *) G_aes_module_buff;
		word_p2 = (u4 *) G_last_mixup;

		while (block_size_words-- > 0) {
			*word_p1++ ^= *word_p2++;
		}

		aes_util_encrypt((u4 *) G_last_mixup, (u4 *) G_aes_module_buff, (u4 *) G_aes_module_key_expanded_buff, Aes_key_len);
		memcpy(outblk_p, G_last_mixup, AES_BLKSIZ);

		inblk_p  += AES_BLKSIZ_WORDS;
		outblk_p += AES_BLKSIZ_WORDS;
	}
}

void aes_util_cbc_dec(u1 *out_buff, u1 *in_buff, u4 nblocks)
{
	u4 *inblk_p, *outblk_p, *word_p1, *word_p2, block_size_words;

	inblk_p  = (u4 *) in_buff;
	outblk_p = (u4 *) out_buff;

	while (nblocks-- > 0) {
		aes_util_decrypt(outblk_p, inblk_p, (u4 *) G_aes_module_key_expanded_buff, Aes_key_len);

		block_size_words = AES_BLKSIZ_WORDS;
		word_p1 = (u4 *) outblk_p;
		word_p2 = (u4 *) G_last_mixup;

		while (block_size_words-- > 0) {
			*word_p1++ ^= *word_p2++;
		}

		memcpy(G_last_mixup, inblk_p, AES_BLKSIZ);

		inblk_p  += AES_BLKSIZ_WORDS;
		outblk_p += AES_BLKSIZ_WORDS;
	}
}


void aes_util_cfb_enc(u1 *out_buff, u1 *in_buff, u4 nblocks)
{
	if (CfbBits == 0) {
		u4 *inblk_p, *outblk_p, *word_p1, *word_p2, block_size_words;

		inblk_p  = (u4 *) in_buff;
		outblk_p = (u4 *) out_buff;

		/* CFB128 */
		while (nblocks-- > 0) {
			aes_util_encrypt(outblk_p, (u4 *) G_last_mixup, (u4 *) G_aes_module_key_expanded_buff, Aes_key_len);

			block_size_words = AES_BLKSIZ_WORDS;
			word_p1 = (u4 *) outblk_p;
			word_p2 = (u4 *) inblk_p;

			while (block_size_words-- > 0) {
				*word_p1++ ^= *word_p2++;
			}

			memcpy(G_last_mixup, outblk_p, AES_BLKSIZ);

			inblk_p  += AES_BLKSIZ_WORDS;
			outblk_p += AES_BLKSIZ_WORDS;
		}
	} else if (CfbBits == 8) {
		/* CFB8 */
		u1 *ip, *op;

		ip = in_buff;
		op = out_buff;

		while (nblocks-- > 0) {
			u4 i;

			aes_util_encrypt((u4 *) TempBuf, (u4 *) G_last_mixup, (u4 *) G_aes_module_key_expanded_buff, Aes_key_len);

			/* XOR 1st-byte of plaintext with encryption result */
			op[0] = ip[0] ^ TempBuf[0];

			/* Update Mixup */
			for (i = 0; i < (AES_BLKSIZ - 1); i++) {
				G_last_mixup[i] = G_last_mixup[i + 1];
			}
			G_last_mixup[AES_BLKSIZ - 1] = op[0];

			op++;
			ip++;
		}
	} else {  /* CfbBits == 1 */
		/* CFB1 */
		u4 nb;    /* Number of bytes */
		u4 rbit;  /* Remaining bits */
		u4 i, j, k;
		u1 *ip, *op;
		u4 bv;  /* Byte value */

		ip = in_buff;
		op = out_buff;

		nb = nblocks / 8;
		rbit = nblocks % 8;

		for (i = 0; i < nb; i++) {
			bv = 0;
			for (j = 0; j < 8; j++) {
				u4 res;

				aes_util_encrypt((u4 *) TempBuf,
				                 (u4 *) G_last_mixup,
						 (u4 *) G_aes_module_key_expanded_buff,
						 Aes_key_len);

				/* Check heading first bit of encryption result */
				res = TempBuf[0] & 0x80;

				if (res) {
					bv |= 1 << (7 - j);
				}

				/* Update Mixup: shift left by 1 bit
				 * and put result at last bit */
				for (k = 0; k < (AES_BLKSIZ - 1); k++) {
					G_last_mixup[k] = (G_last_mixup[k] << 1) & 0xFE;

					if (G_last_mixup[k + 1] & 0x80) {
						G_last_mixup[k] |= 0x01;
					}
				}

				G_last_mixup[AES_BLKSIZ - 1] =
					(G_last_mixup[AES_BLKSIZ - 1] << 1) & 0xFE;

				if ((bv ^ *ip) & (1 << (7 - j))) {
					G_last_mixup[AES_BLKSIZ - 1] |= 0x01;
				}
			}

			*op = *ip ^ (u1) bv;

			op++;
			ip++;
		}

		bv = 0;
		for (j = 0; j < rbit; j++) {
			u4 res;

			aes_util_encrypt((u4 *) TempBuf,
					 (u4 *) G_last_mixup,
					 (u4 *) G_aes_module_key_expanded_buff,
					 Aes_key_len);

			/* Check heading first bit of encryption result */
			res = TempBuf[0] & 0x80;
			if (res) {
				bv |= 1 << (7 - j);
			}

			/* Update Mixup: shift left by 1 bit
			 * and put result at last bit */
			for (k = 0; k < (AES_BLKSIZ - 1); k++) {
				G_last_mixup[k] = (G_last_mixup[k] << 1) & 0xFE;

				if (G_last_mixup[k + 1] & 0x80) {
					G_last_mixup[k] |= 0x01;
				}
			}

			G_last_mixup[AES_BLKSIZ - 1] =
				(G_last_mixup[AES_BLKSIZ - 1] << 1) & 0xFE;

			if ((bv ^ *ip) & (1 << (7 - j))) {
				G_last_mixup[AES_BLKSIZ - 1] |= 0x01;
			}
		}

		*op = *ip ^ (u1) bv;
	}
}

void aes_util_cfb_dec(u1 *out_buff, u1 *in_buff, u4 nblocks)
{
	if (CfbBits == 0) {
		u4 *inblk_p, *outblk_p, *word_p1, *word_p2, block_size_words;

		inblk_p  = (u4 *) in_buff;
		outblk_p = (u4 *) out_buff;

		while (nblocks-- > 0) {
			aes_util_encrypt(outblk_p,
			                (u4 *) G_last_mixup,
					(u4 *) G_aes_module_key_expanded_buff,
					Aes_key_len);

			block_size_words = AES_BLKSIZ_WORDS;
			word_p1 = (u4 *) outblk_p;
			word_p2 = (u4 *) inblk_p;

			while (block_size_words-- > 0) {
				*word_p1++ ^= *word_p2++;
			}

			memcpy(G_last_mixup, inblk_p, AES_BLKSIZ);

			inblk_p  += AES_BLKSIZ_WORDS;
			outblk_p += AES_BLKSIZ_WORDS;
		}
	} else if (CfbBits == 8) {
		/* CFB8 */
		u1 *ip, *op;

		ip = in_buff;
		op = out_buff;

		while (nblocks-- > 0) {
			u4 i;

			aes_util_encrypt((u4 *) TempBuf,
			                 (u4 *) G_last_mixup,
					 (u4 *) G_aes_module_key_expanded_buff,
					 Aes_key_len);

			/* XOR 1st-byte of plaintext with encryption result */
			op[0] = ip[0] ^ TempBuf[0];

			/* Update Mixup */
			for (i = 0; i < (AES_BLKSIZ - 1); i++) {
				G_last_mixup[i] = G_last_mixup[i + 1];
			}
			G_last_mixup[AES_BLKSIZ - 1] = ip[0];

			op++;
			ip++;
		}
	} else {  /* CfbBits == 1 */
		/* CFB1 */
		u4 nb;  /* Number of bytes */
		u4 rbit;  /* Remaining bits */
		u4 i, j, k;
		u1 *ip, *op;
		u4 bv;  /* Byte value */

		ip = in_buff;
		op = out_buff;

		nb = nblocks / 8;
		rbit = nblocks % 8;

		for (i = 0; i < nb; i++) {
			bv = 0;
			for (j = 0; j < 8; j++) {
				u4 res;

				aes_util_encrypt((u4 *) TempBuf,
				                 (u4 *) G_last_mixup,
						 (u4 *) G_aes_module_key_expanded_buff,
						 Aes_key_len);

				/* Check heading first bit of encryption result */
				res = TempBuf[0] & 0x80;
				if (res) {
					bv |= 1 << (7 - j);
				}

				/* Update Mixup: shift left by 1 bit
				 * and put result at last bit */
				for (k = 0; k < (AES_BLKSIZ - 1); k++) {
					G_last_mixup[k] = (G_last_mixup[k] << 1) & 0xFE;

					if (G_last_mixup[k + 1] & 0x80) {
						G_last_mixup[k] |= 0x01;
					}
				}

				G_last_mixup[AES_BLKSIZ - 1] =
					(G_last_mixup[AES_BLKSIZ - 1] << 1) & 0xFE;

				if ((*ip) & (1 << (7 - j))) {
					G_last_mixup[AES_BLKSIZ - 1] |= 0x01;
				}
			}

			*op = *ip ^ (u1) bv;

			op++;
			ip++;
		}

		bv = 0;
		for (j = 0; j < rbit; j++) {
			u4 res;

			aes_util_encrypt((u4 *) TempBuf,
					 (u4 *) G_last_mixup,
					 (u4 *) G_aes_module_key_expanded_buff,
					 Aes_key_len);

			/* Check heading first bit of encryption result */
			res = TempBuf[0] & 0x80;
			if (res) {
				bv |= 1 << (7 - j);
			}

			/* Update Mixup: shift left by 1 bit
			 * and put result at last bit */
			for (k = 0; k < (AES_BLKSIZ - 1); k++) {
				G_last_mixup[k] = (G_last_mixup[k] << 1) & 0xFE;

				if (G_last_mixup[k + 1] & 0x80) {
					G_last_mixup[k] |= 0x01;
				}
			}

			G_last_mixup[AES_BLKSIZ - 1] =
				(G_last_mixup[AES_BLKSIZ - 1] << 1) & 0xFE;

			if ((*ip) & (1 << (7 - j))) {
				G_last_mixup[AES_BLKSIZ - 1] |= 0x01;
			}
		}

		*op = *ip ^ (u1) bv;
	}
}


void aes_util_ofb_enc(u1 *out_buff, u1 *in_buff, u4 nblocks)
{
	u4 *inblk_p, *outblk_p, *word_p1, *word_p2, block_size_words;

	inblk_p  = (u4 *) in_buff;
	outblk_p = (u4 *) out_buff;

	while (nblocks-- > 0) {
		aes_util_encrypt(outblk_p, (u4 *) G_last_mixup, (u4 *) G_aes_module_key_expanded_buff, Aes_key_len);

		/* Next Mixup */
		memcpy(G_last_mixup, outblk_p, AES_BLKSIZ);

		block_size_words = AES_BLKSIZ_WORDS;
		word_p1 = (u4 *) outblk_p;
		word_p2 = (u4 *) inblk_p;

		while (block_size_words-- > 0) {
			*word_p1++ ^= *word_p2++;
		}

		inblk_p  += AES_BLKSIZ_WORDS;
		outblk_p += AES_BLKSIZ_WORDS;
	}
}


void aes_util_ofb_dec(u1 *out_buff, u1 *in_buff, u4 nblocks)
{
	u4 *inblk_p, *outblk_p, *word_p1, *word_p2, block_size_words;

	inblk_p  = (u4 *) in_buff;
	outblk_p = (u4 *) out_buff;

	while (nblocks-- > 0) {
		aes_util_encrypt(outblk_p, (u4 *) G_last_mixup, (u4 *) G_aes_module_key_expanded_buff, Aes_key_len);

		/* Next Mixup */
		memcpy(G_last_mixup, outblk_p, AES_BLKSIZ);

		block_size_words = AES_BLKSIZ_WORDS;
		word_p1 = (u4 *) outblk_p;
		word_p2 = (u4 *) inblk_p;

		while (block_size_words-- > 0) {
			*word_p1++ ^= *word_p2++;
		}

		inblk_p  += AES_BLKSIZ_WORDS;
		outblk_p += AES_BLKSIZ_WORDS;
	}
}

/* Temp buffer for cipher stealing functions */
static u1 Cs_tmpbuf[AES_BLKSIZ];

/* Data size should be > 1 block and <= 2 blocks */
static void aes_final_cs_ecb_enc(u1 *outbuf, u1 *inbuf, u4 size)
{
	u4 last_blk_len;

	/* Step 1. E_(n-1) = E(P_(n-1), K) */
	aes_util_ecb_enc(outbuf, inbuf, 1);

	/* Step 2. C_n = HEAD(E_(n-1), M) */
	last_blk_len = size - AES_BLKSIZ;
	memcpy(outbuf + AES_BLKSIZ, outbuf, last_blk_len);

	/* Step 3. D_n = P_n || TAIL(E_(n-1), B-M) */
	memcpy(Cs_tmpbuf, inbuf + AES_BLKSIZ, last_blk_len);
	memcpy(Cs_tmpbuf + last_blk_len, outbuf + last_blk_len, AES_BLKSIZ - last_blk_len);

	/* Step 4. C_(n-1) = E(D_n, K) */
	aes_util_ecb_enc(outbuf, Cs_tmpbuf, 1);
}


/* Data size should be > 1 block and <= 2 blocks */
static void aes_final_cs_ecb_dec(u1 *outbuf, u1 *inbuf, u4 size)
{
	u4 last_blk_len;

	/* Step 1. D_n = D(C_(n-1), K) */
	aes_util_ecb_dec(outbuf, inbuf, 1);

	/* Step 2. P_n = HEAD(D_n, M) */
	last_blk_len = size - AES_BLKSIZ;
	memcpy(outbuf + AES_BLKSIZ, outbuf, last_blk_len);

	/* Step 3. E_(n-1) = C_n || TAIL(D_n, B-M)) */
	memcpy(Cs_tmpbuf, inbuf + AES_BLKSIZ, last_blk_len);
	memcpy(Cs_tmpbuf + last_blk_len, outbuf + last_blk_len, AES_BLKSIZ - last_blk_len);

	/* Step 4. P_(n-1) = D(E_(n-1), K) */
	aes_util_ecb_dec(outbuf, Cs_tmpbuf, 1);
}


/* Ciphertext stealing ECB mode, size should be > 1 block and <= 2 blocks */
int aes_final_cs_ecb(u1 *outbuf, u1 *inbuf, u4 size, u4 enc)
{
	/* Check pointer and data size */
	if ((outbuf == NULL) || (inbuf == NULL)) {
		return -1;
	}

	if ((size <= AES_BLKSIZ) || (size > (2 * AES_BLKSIZ))) {
		return -2;
	}

	if (enc) {
		aes_final_cs_ecb_enc(outbuf, inbuf, size);
	} else {
		aes_final_cs_ecb_dec(outbuf, inbuf, size);
	}

	return 0;
}

/* Data size should be > 1 block and <= 2 blocks */
static void aes_final_cs_cbc_enc(u1 *outbuf, u1 *inbuf, u4 size)
{
	u4 last_blk_len = size - AES_BLKSIZ;

	/* Step 1. C_n = HEAD(E(P_(n-1), K), M) */
	aes_util_cbc_enc(Cs_tmpbuf, inbuf, 1);
	memcpy(outbuf + AES_BLKSIZ, Cs_tmpbuf, last_blk_len);

	/* Step 2. C_(n-1) = E(P_n || 00...0, K)*/
	memset(Cs_tmpbuf, 0x00, AES_BLKSIZ);
	memcpy(Cs_tmpbuf, inbuf + AES_BLKSIZ, last_blk_len);
	aes_util_cbc_enc(outbuf, Cs_tmpbuf, 1);
}


/* Data size should be > 1 block and <= 2 blocks */
static void aes_final_cs_cbc_dec(u1 *outbuf, u1 *inbuf, u4 size)
{
	u4 last_blk_len = size - AES_BLKSIZ;
	u4 i;

	/* Step 1. D_n = D(C_(n-1), K) */
	aes_util_ecb_dec(Cs_tmpbuf, inbuf, 1);

	/* Step 2. P_n = HEAD(D_n ^ (C_n || 0), M) */
	for (i = 0; i < last_blk_len; i++) {
		outbuf[AES_BLKSIZ + i] = Cs_tmpbuf[i] ^ inbuf[AES_BLKSIZ + i];
	}

	/* Step 3. E_(n-1) = C_n || TAIL(D_n, B-M) */
	memcpy(Cs_tmpbuf, inbuf + AES_BLKSIZ, last_blk_len);

	/* Step 4. P_(n-1) = D(E_(n-1), K) ^ C_(n-2) */
	aes_util_cbc_dec(outbuf, Cs_tmpbuf, 1);
}


/* Ciphertext stealing CBC mode, size should be > 1 block and <= 2 blocks */
int aes_final_cs_cbc(u1 *outbuf, u1 *inbuf, u4 size, u4 enc)
{
	/* Check pointer and data size */
	if ((outbuf == NULL) || (inbuf == NULL)) {
		return -1;
	}

	if ((size <= AES_BLKSIZ) || (size > (2 * AES_BLKSIZ))) {
		return -2;
	}

	if (enc) {
		aes_final_cs_cbc_enc(outbuf, inbuf, size);
	} else {
		aes_final_cs_cbc_dec(outbuf, inbuf, size);
	}

	return 0;
}


/* Set feedback length for CFB mode (in AES impl module) */
int aes_set_cfb_bits(u4 cfb_bits)
{
	/* Check inputs */
	if ((cfb_bits != 0) && (cfb_bits != 1) && (cfb_bits != 8)) {
		return -1;
	}

	CfbBits = cfb_bits;

	return 0;
}


/* Set IV in AES impl module */
int aes_util_set_iv(u1 *init_vec)
{
	if (init_vec == NULL) {
		return -1;
	}

	memcpy(G_last_mixup, init_vec, AES_BLKSIZ);

	return 0;
}


/* Get current mask vector (16 bytes) */
int aes_util_get_curr_mask_vec(u1 *mv)
{
	if (mv == NULL) {
		return -1;
	}

	memcpy(mv, G_last_mixup, AES_BLKSIZ);

	return 0;
}



#undef AES_BLKSIZ
#undef AES_BLKSIZ_WORDS
#undef WORDSIZ
/* Activate AES table */
int aes_table_activate (
	IN     u1  tab_id,
	IN     u1  force)
{
	int ret = 0;
	u1  tmp_aestab[AESTAB_SIZ];

	if (tab_id > AES_TAB_IDMAX) {
		ERROR_FUNCEND(ERR_AES_TAB_ID);
	}

	if ((tab_id != Actv_aes_tab_id) || (force == FORCE)) {
		if (tab_id == AES_TAB_STD) {
			memcpy(Actv_aes_tab_enc, Te0, AESTAB_SIZ);
			memcpy(Actv_aes_tab_dec, Td0, AESTAB_SIZ);
		} else {
            exit(0);
		}

		Actv_aes_tab_id = tab_id;
	}

__func_end:
	return ret;
}


/* Set key value */
int aes_set_key(IN u4 key_len, IN u1 *key, IN u4 mode)
{
	/* Check modes of operation */
	if (!AES_MODE_OK (mode)) {
		return ERR_AES_MODE;
	}

	/* Check key pointer */
	if (key == NULL) {
		return ERR_AES_PARAM;
	}

	/* Check key length */
	if (!AES_KEYLEN_OK (key_len)) {
		return ERR_AES_KEYLEN;
	}

	/***
	 * 16 + 8 * key_len :
	 * 0x00 -> 16 bytes
	 * 0x01 -> 24 bytes
	 * 0x02 -> 32 bytes
	 */
	aes_key_expansion ((u4*) key, 16 + 8 * key_len);

	Aes_module_stat.key_len = key_len;
	Aes_module_stat.mode = mode;

	return 0;
}


/* Set IV */
int aes_set_iv(u1 *iv)
{
	if (iv == NULL) {
		return ERR_AES_PARAM;
	}

	memcpy (Aes_module_stat.init_vec, iv, AES_BLK_SIZ);
	if (aes_util_set_iv (iv) != 0) {
		return ERR_AES_SET_IV;
	}

	return 0;
}


/***
 * Ready to do AES calculations: make setting effect
 *   -- NOTE: here key_len and mode should match with
 *            setting in aes_set_key, otherwise
 *            result is unpredictable
 */
int aes_calc_ready(u4 dir, u4 key_len, u4 mode)
{
	if (!AES_DIR_OK(dir)) {
		return ERR_AES_DIR;
	}

	if (!AES_KEYLEN_OK(key_len)) {
		return ERR_AES_KEYLEN;
	}

	if (!AES_MODE_OK(mode)) {
		return ERR_AES_MODE;
	}

	if (key_len != Aes_module_stat.key_len ||
		mode    != Aes_module_stat.mode) {
		return ERR_AES_PARAM;
	}

	if (!AES_TAB_SETUP_OK()) {
		return ERR_AES_TAB_SETUP;
	}

	/* Set encrypt/decrypt function pointer, which will be used in kvcy_aes_crypt */
	switch (mode) {
	case AES_MODE_ECB :
		if (dir == AES_DIR_ENC) {
			Aes_module_stat.crypt = aes_util_ecb_enc;
		} else {
			Aes_module_stat.crypt = aes_util_ecb_dec;
		}
		break;
	case AES_MODE_CBC :
		if (dir == AES_DIR_ENC) {
			Aes_module_stat.crypt = aes_util_cbc_enc;
		} else {
			Aes_module_stat.crypt = aes_util_cbc_dec;
		}
		break;
	default :
		return ERR_AES_MODE;
	}

	return 0;
}

/* AES calculation */
int aes_calc(IN u4 in_len, IN u1 *in, OUT u1 *out)
{
	/* Check inputs */
	if ((out == NULL) ||
        (in == NULL) ||
        (in_len == 0)) {
		return ERR_AES_PARAM;
	}

	if (in_len % AES_BLK_SIZ != 0) {
		return ERR_AES_PARAM;
	}

	Aes_module_stat.crypt (out, in, in_len / AES_BLK_SIZ);

	return 0;
}

#define A 0x67452301
#define B 0xefcdab89
#define C 0x98badcfe
#define D 0x10325476

static uint32_t S[] = {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
                       5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
                       4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
                       6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};

static uint32_t K[] = {0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
                       0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
                       0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
                       0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
                       0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
                       0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
                       0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
                       0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
                       0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
                       0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
                       0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
                       0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
                       0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
                       0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
                       0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
                       0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};

/*
 * Padding used to make the size (in bits) of the input congruent to 448 mod 512
 */
static uint8_t PADDING[] = {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/*
 * Initialize a context
 */
void md5Init(MD5Context *ctx){
	ctx->size = (uint64_t)0;

	ctx->buffer[0] = (uint32_t)A;
	ctx->buffer[1] = (uint32_t)B;
	ctx->buffer[2] = (uint32_t)C;
	ctx->buffer[3] = (uint32_t)D;
}

/*
 * Add some amount of input to the context
 *
 * If the input fills out a block of 512 bits, apply the algorithm (md5Step)
 * and save the result in the buffer. Also updates the overall size.
 */

void md5Update(MD5Context *ctx, uint8_t *input_buffer, size_t input_len){
	uint32_t input[16];
	unsigned int offset = ctx->size % 64;
	ctx->size += (uint64_t)input_len;

	// Copy each byte in input_buffer into the next space in our context input
	for(unsigned int i = 0; i < input_len; ++i){
		ctx->input[offset++] = (uint8_t)*(input_buffer + i);

		// If we've filled our context input, copy it into our local array input
		// then reset the offset to 0 and fill in a new buffer
		// The local array input is a list of 16 32-bit words for use in the algorithm
		if(offset % 64 == 0){
			for(unsigned int j = 0; j < 16; ++j){
				// Convert to little-endian
				input[j] = (uint32_t)(ctx->input[(j * 4) + 3]) << 24 |
						   (uint32_t)(ctx->input[(j * 4) + 2]) << 16 |
						   (uint32_t)(ctx->input[(j * 4) + 1]) <<  8 |
						   (uint32_t)(ctx->input[(j * 4)]);
			}
			md5Step(ctx->buffer, input);
			offset = 0;
		}
	}
}

/*
 * Pad the current input to get to 448 bytes, append the size in bits to the very end,
 * and save the result of the final iteration into digest.
 */
void md5Finalize(MD5Context *ctx){
	uint32_t input[16];
	unsigned int offset = ctx->size % 64;
	unsigned int padding_length = offset < 56 ? 56 - offset : (56 + 64) - offset;

	// Fill in the padding andndo the changes to size that resulted from the update
	md5Update(ctx, PADDING, padding_length);
	ctx->size -= (uint64_t)padding_length;

	// Do a final update (internal to this function)
	// Last two 32-bit words are the two halves of the size (converted from bytes to bits)
	for(unsigned int j = 0; j < 14; ++j){
		input[j] = (uint32_t)(ctx->input[(j * 4) + 3]) << 24 |
		           (uint32_t)(ctx->input[(j * 4) + 2]) << 16 |
		           (uint32_t)(ctx->input[(j * 4) + 1]) <<  8 |
		           (uint32_t)(ctx->input[(j * 4)]);
	}
	input[14] = (uint32_t)(ctx->size * 8);
	input[15] = (uint32_t)((ctx->size * 8) >> 32);

	md5Step(ctx->buffer, input);

	// Move the result into digest
	// (Convert from little-endian)
	for(unsigned int i = 0; i < 4; ++i){
		ctx->digest[(i * 4) + 0] = (uint8_t)((ctx->buffer[i] & 0x000000FF));
		ctx->digest[(i * 4) + 1] = (uint8_t)((ctx->buffer[i] & 0x0000FF00) >>  8);
		ctx->digest[(i * 4) + 2] = (uint8_t)((ctx->buffer[i] & 0x00FF0000) >> 16);
		ctx->digest[(i * 4) + 3] = (uint8_t)((ctx->buffer[i] & 0xFF000000) >> 24);
	}
}

/*
 * Step on 512 bits of input with the main MD5 algorithm.
 */
void md5Step(uint32_t *buffer, uint32_t *input){
	uint32_t AA = buffer[0];
	uint32_t BB = buffer[1];
	uint32_t CC = buffer[2];
	uint32_t DD = buffer[3];

	uint32_t E;

	unsigned int j;

	for(unsigned int i = 0; i < 64; ++i){
		switch(i / 16){
			case 0:
				E = F(BB, CC, DD);
				j = i;
				break;
			case 1:
				E = G(BB, CC, DD);
				j = ((i * 5) + 1) % 16;
				break;
			case 2:
				E = H(BB, CC, DD);
				j = ((i * 3) + 5) % 16;
				break;
			default:
				E = I(BB, CC, DD);
				j = (i * 7) % 16;
				break;
		}

		uint32_t temp = DD;
		DD = CC;
		CC = BB;
		BB = BB + rotate_left(AA + E + K[i] + input[j], S[i]);
		AA = temp;
	}

	buffer[0] += AA;
	buffer[1] += BB;
	buffer[2] += CC;
	buffer[3] += DD;
}

/*
 * Functions that will return a pointer to the hash of the provided input
 */
uint8_t* md5String(char *input, int len){
	MD5Context ctx;
	md5Init(&ctx);
	md5Update(&ctx, (uint8_t *)input, len);
	md5Finalize(&ctx);

	uint8_t *result = malloc(16);
	memcpy(result, ctx.digest, 16);
	return result;
}

uint8_t* md5File(FILE *file){
	char *input_buffer = malloc(1024);
	size_t input_size = 0;

	MD5Context ctx;
	md5Init(&ctx);

	while((input_size = fread(input_buffer, 1, 1024, file)) > 0){
		md5Update(&ctx, (uint8_t *)input_buffer, input_size);
	}

	md5Finalize(&ctx);

	free(input_buffer);

	uint8_t *result = malloc(16);
	memcpy(result, ctx.digest, 16);
	return result;
}

/*
 * Bit-manipulation functions defined by the MD5 algorithm
 */
uint32_t F(uint32_t X, uint32_t Y, uint32_t Z){
	return (X & Y) | (~X & Z);
}

uint32_t G(uint32_t X, uint32_t Y, uint32_t Z){
	return (X & Z) | (Y & ~Z);
}

uint32_t H(uint32_t X, uint32_t Y, uint32_t Z){
	return X ^ Y ^ Z;
}

uint32_t I(uint32_t X, uint32_t Y, uint32_t Z){
	return Y ^ (X | ~Z);
}

/*
 * Rotates a 32-bit word left by n bits
 */
uint32_t rotate_left(uint32_t x, uint32_t n){
	return (x << n) | (x >> (32 - n));
}

/*
 * Printing bytes from buffers or the hash
 */
void print_bytes(void *p, size_t length){
	uint8_t *pp = (uint8_t *)p;
	for(unsigned int i = 0; i < length; ++i){
		if(i && !(i % 16)){
			printf("\n");
		}
		printf("%02X ", pp[i]);
	}
	printf("\n");
}

void print_hash(uint8_t *p){
	for(unsigned int i = 0; i < 16; ++i){
		printf("%02x", p[i]);
	}
	printf("\n");
}


u1 should_out[32]={0x9e, 0xa7, 0x2b, 0xe8, 0xde, 0x91, 0xea, 0x83, 0xfe, 0xcc, 0x1b, 0x24, 0x3b, 0x97, 0x36, 0x28, 0x2d, 0xc9, 0x86, 0x5f, 0x88, 0x4e, 0x09, 0xc8, 0xb0, 0x1b, 0xc8, 0xfe, 0x23, 0x76, 0x27, 0xd5};
int main(int argc, char **argv)
{
    int i;
    if(argc != 2)
    {
        return 0;
    }
    char *flag = argv[1];
    aes_table_activate (AES_TAB_STD, NOT_FORCE);
    if (aes_set_key(AES_KEYLEN_256, (char*)0x403f49, AES_MODE_ECB) != 0) {
        return 0;
    }

    /* The 1st 16 bytes in page as IV (which is set when setting DMK) */
    u1 iv[16];
    for(i = 0; i < 16; i++)
    {
        iv[i] = i;
    }

    if (aes_set_iv(iv) != 0) {
        return 0;
    }

    if (aes_calc_ready(AES_DIR_DEC, AES_KEYLEN_256, AES_MODE_ECB) != 0) {
        return 0;
    }

    /* 4. Decrypt page and output page data */
    if(strlen(flag) != 38)
    {
        return 0;
    }
   if((flag[0] != 0x66)||(flag[1] != 0x6C)||(flag[2] != 0x61)||(flag[3] != 0x67)||(flag[4] != 0x7b)||(flag[37] != 0x7d))
   {
       return 0;
   }
    u1 out_buff[32];
    if (aes_calc(32, &flag[5], out_buff) != 0) {
        return 0;
    }

    for(i = 0; i < 32; i++)
    {
        if(out_buff[i] != should_out[i])
        {
            printf("fail\n");
            return 0;
        }
    }
    printf("success\n");
    return 1;
}

