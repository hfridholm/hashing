/*
 * SHA256
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define LSHIFT(a, b) ((a) << (b))
#define RSHIFT(a, b) ((a) >> (b))

#define RROTATE(a, b) (RSHIFT(a, b) | LSHIFT(a, 32 - (b)))

#define CHOISE(e, f, g) (((e) & (f)) ^ (~(e) & (g)))
#define MAJORITY(a, b, c) (((a) & (b)) ^ ((a) & (c)) ^ ((b) & (c)))

#define SUM0(x) (RROTATE(x, 2) ^ RROTATE(x, 13) ^ RROTATE(x, 22))
#define SUM1(x) (RROTATE(x, 6) ^ RROTATE(x, 11) ^ RROTATE(x, 25))

#define SIG0(x) (RROTATE(x, 7) ^ RROTATE(x, 18) ^ RSHIFT(x, 3))
#define SIG1(x) (RROTATE(x, 17) ^ RROTATE(x, 19) ^ RSHIFT(x, 10))

const uint32_t k[64] = {
  0x428a2f98,
  0x71374491,
  0xb5c0fbcf,
  0xe9b5dba5,
  0x3956c25b,
  0x59f111f1,
  0x923f82a4,
  0xab1c5ed5,
	0xd807aa98,
  0x12835b01,
  0x243185be,
  0x550c7dc3,
  0x72be5d74,
  0x80deb1fe,
  0x9bdc06a7,
  0xc19bf174,
	0xe49b69c1,
  0xefbe4786,
  0x0fc19dc6,
  0x240ca1cc,
  0x2de92c6f,
  0x4a7484aa,
  0x5cb0a9dc,
  0x76f988da,
	0x983e5152,
  0xa831c66d,
  0xb00327c8,
  0xbf597fc7,
  0xc6e00bf3,
  0xd5a79147,
  0x06ca6351,
  0x14292967,
	0x27b70a85,
  0x2e1b2138,
  0x4d2c6dfc,
  0x53380d13,
  0x650a7354,
  0x766a0abb,
  0x81c2c92e,
  0x92722c85,
	0xa2bfe8a1,
  0xa81a664b,
  0xc24b8b70,
  0xc76c51a3,
  0xd192e819,
  0xd6990624,
  0xf40e3585,
  0x106aa070,
	0x19a4c116,
  0x1e376c08,
  0x2748774c,
  0x34b0bcb5,
  0x391c0cb3,
  0x4ed8aa4a,
  0x5b9cca4f,
  0x682e6ff3,
	0x748f82ee,
  0x78a5636f,
  0x84c87814,
  0x8cc70208,
  0x90befffa,
  0xa4506ceb,
  0xbef9a3f7,
  0xc67178f2
};

/*
 * These bit manipulation macros is ment for 32-bit words
 *
 * BIT refers to the bit in the words at a specific index
 */
#define BIT_WORD(WORDS, BIT) (WORDS)[(BIT) / 32]
#define WORD_BIT(BIT) (31 - ((BIT) % 32))

#define BIT_GET(WORDS, BIT) RSHIFT(BIT_WORD(WORDS, BIT) & LSHIFT(1, WORD_BIT(BIT)), WORD_BIT(BIT))
#define BIT_SET(WORDS, BIT) (BIT_WORD(WORDS, BIT) |=  LSHIFT(1, WORD_BIT(BIT)))
#define BIT_OFF(WORDS, BIT) (BIT_WORD(WORDS, BIT) &= ~LSHIFT(1, WORD_BIT(BIT)))

/*
 *
 */
static char* hs_hash(char* hash, uint32_t hs[8])
{
  for(uint8_t index = 0; index < 8; index++)
  {
    char chash[10];

    sprintf(chash, "%08x", hs[index]);

    strncat(hash, chash, sizeof(char) * 8);
  }
  return hash;
}

/*
 * PARAMS
 * - uint32_t hs[8] |
 */
void gg(uint32_t hs[8], const uint32_t chunk[16])
{
  uint32_t w[64];

  uint8_t windex = 0;

  for(; windex < 16; windex++)
  {
    w[windex] = chunk[windex];
  }

  for(; windex < 64; windex++)
  {
    uint32_t word1 = w[windex - 16];
    uint32_t word2 = SIG0(w[windex - 15]);
    uint32_t word3 = w[windex - 7];
    uint32_t word4 = SIG1(w[windex - 2]);

    w[windex] = word1 + word2 + word3 + word4;
  }

  uint32_t a, b, c, d, e, f, g, h;
  uint32_t t1, t2;

  a = hs[0];
  b = hs[1];
  c = hs[2];
  d = hs[3];
  e = hs[4];
  f = hs[5];
  g = hs[6];
  h = hs[7];

  for(uint8_t index = 0; index < 64; index++)
  {
    uint32_t majority = MAJORITY(a, b, c);
    uint32_t choise   = CHOISE(e, f, g);

    uint32_t sum0 = SUM0(a);
    uint32_t sum1 = SUM1(e);

    t1 = h + sum1 + choise + k[index] + w[index];
    t2 = sum0 + majority;

    h = g;
    g = f;
    f = e;
    e = d + t1;
    d = c;
    c = b;
    b = a;
    a = t1 + t2;
  }

  hs[0] += a;
  hs[1] += b;
  hs[2] += c;
  hs[3] += d;
  hs[4] += e;
  hs[5] += f;
  hs[6] += g;
  hs[7] += h;
}

/*
 *
 */
void hh(uint32_t* block, const void* message, size_t size)
{
  for(int index = 0; index < size; index++)
  {
    for(int bit = 0; bit < 8; bit++)
    {
      char word = ((char*) message)[index];

      if(word & LSHIFT(1, (7 - bit)))
      {
        BIT_SET(block, (index * 8) + bit);
      }
      else
      {
        BIT_OFF(block, (index * 8) + bit);
      }
    }
  }
}

static void chunk_print(uint32_t chunk[16])
{
  for(uint16_t bit = 0; bit < 512; bit++)
  {
    printf("%c", BIT_GET(chunk, bit) ? '1' : '0');

    if((bit + 1) % 8 == 0) printf(" ");

    if((bit + 1) % 32 == 0) printf("\n");
  }
}

static void block_print(uint32_t* block, size_t chunks)
{
  for(size_t index = 0; index < chunks; index++)
  {
    chunk_print(block + (index * 512));

    if(index < (chunks - 1)) printf("\n");
  }
}

// This is the equivalent to ceil(size / 64)
#define INIT_CHUNKS(SIZE) (((SIZE) & 0b111111) ? ((SIZE) >> 6) + 1 : (SIZE) >> 6)

#define MOD(SIZE) (((SIZE) * 8) % 512)

#define CHUNKS(SIZE) (MOD(SIZE) > 447 ? INIT_CHUNKS(SIZE) + 1 : INIT_CHUNKS(SIZE))
#define ZEROS(SIZE) (MOD(SIZE) > 447 ? 959 - MOD(SIZE) : 447 - MOD(SIZE))
/*
 * PARAMS
 * - char* hash          |
 * - const void* message |
 * - size_t size         | The amount of bytes (8 bit)
 */
extern char* sha256(char* hash, const void* message, size_t size)
{
  size_t chunks = CHUNKS(size);
  uint16_t zeros = ZEROS(size);

  uint32_t block[chunks * 16];

  // 1. Copy the encoded message to the message block
  hh(block, message, size);

  // 2. Append a single '1' to the encoded message
  BIT_SET(block, size * 8);

  // 3. Add zeros between the encoded message and the length integer
  for(size_t index = 0; index < zeros; index++)
  {
    BIT_OFF(block, size * 8 + 1 + index);
  }

  // The length is the amount of bits (1 byte = 8 bits)
  uint64_t length = (uint64_t) size * 8;

  // 4. Copy binary representation of length to end of block
  block[chunks * 16 - 2] = (uint32_t) (length >> 32);
  block[chunks * 16 - 1] = (uint32_t) length;

  block_print(block, chunks);

  uint32_t hs[8] = {
    0x6a09e667,
    0xbb67ae85,
    0x3c6ef372,
    0xa54ff53a,
    0x510e527f,
    0x9b05688c,
    0x1f83d9ab,
    0x5be0cd19
  };

  for(uint8_t bit = 0; bit < 32; bit++)
  {
    printf("%c", BIT_GET(hs, bit) ? '1' : '0');
  }

  printf("\n");

  for(size_t index = 0; index < chunks; index++)
  {
    gg(hs, block + (index * 16));
  }

  return hs_hash(hash, hs); 
}
