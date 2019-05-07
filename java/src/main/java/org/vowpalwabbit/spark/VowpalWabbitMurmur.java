package org.vowpalwabbit.spark;

import java.nio.charset.StandardCharsets;

/**
 * Java port of VW-style hashing.
 * 
 * @author Borislav Nikolov
 */
public class VowpalWabbitMurmur {
  private static int rotl32(int x, int r) {
    return (x << r) | (x >>> (32L - r));
  }

  private static int fmix(int h) {
    h ^= h >>> 16;
    h *= 0x85ebca6b;
    h ^= h >>> 13;
    h *= 0xc2b2ae35;
    h ^= h >>> 16;
    return h;
  }

  public static int hash(String s, int seed) {
    byte[] d = s.getBytes(StandardCharsets.UTF_8);
    return hash(d, d.length, seed);
  }

  public static int hash(byte[] data, int len, int seed) {
    return hash(data, 0, len, seed);
  }

  public static int hashNative(byte[] data, int offset ,int len, int seed) {
    // don't have native code here to avoid native library load if not needed
    return VowpalWabbitNative.hash(data, offset, len, seed);
  }

  public static int hash(byte[] data, int offset, int len, int seed) {
    int nblocks = len / 4;
    int h1 = seed;
    int c1 = 0xcc9e2d51;
    int c2 = 0x1b873593;

    int i = offset;
    while (i <= offset + len - 4) {
      int k1 = ((data[i] & 0xFF) | (data[i + 1] & 0xFF) << 8 | (data[i + 2] & 0xFF) << 16 | (data[i + 3] & 0xFF) << 24);

      k1 *= c1;
      k1 = rotl32(k1, 15);
      k1 *= c2;

      h1 ^= k1;
      h1 = rotl32(h1, 13);
      h1 = h1 * 5 + 0xe6546b64;

      i += 4;
    }

    int k1 = 0;
    int end = offset + (nblocks * 4);
    switch (len & 3) {
    case 3:
      k1 ^= (data[end + 2] & 0xFF) << 16;
    case 2:
      k1 ^= (data[end + 1] & 0xFF) << 8;
    case 1:
      k1 ^= data[end] & 0xFF;

      k1 *= c1;
      k1 = rotl32(k1, 15);

      k1 *= c2;
      h1 ^= k1;
    }
    h1 ^= len;
    return fmix(h1);
  }
}