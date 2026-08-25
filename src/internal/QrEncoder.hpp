/**
 * @file QrEncoder.hpp
 * @brief Internal QR Code encoder.
 *
 * Compliant with ISO/IEC 18004:2015
 * Supports: Numeric, Alphanumeric, Byte modes
 * Versions: 1–40 (auto-selection)
 * ECC Levels: L, M, Q, H
 * Full Reed-Solomon ECC, 8 mask patterns, penalty scoring
 * Terminal ASCII rendering only.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SOFTADASTRA_INTERNAL_QR_ENCODER_HPP
#define SOFTADASTRA_INTERNAL_QR_ENCODER_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace softadastra
{
  namespace internal
  {
  class QRCode;
  class QRMatrix;
  class BitBuffer;
  class ReedSolomon;

  enum class ECLevel : uint8_t
  {
    L = 0,
    M = 1,
    Q = 2,
    H = 3
  };
  enum class Mode : uint8_t
  {
    Numeric = 0,
    Alphanumeric = 1,
    Byte = 2
  };

  struct ASCIIOptions
  {
    int quiet_zone = 4;
    bool use_ansi = false;
  };

  // Number of data codewords per (version-1, ec_level)
  static constexpr std::array<std::array<int, 4>, 40> DATA_CODEWORDS = {{
      /* V1  */ {{19, 16, 13, 9}},
      /* V2  */ {{34, 28, 22, 16}},
      /* V3  */ {{55, 44, 34, 26}},
      /* V4  */ {{80, 64, 48, 36}},
      /* V5  */ {{108, 86, 62, 46}},
      /* V6  */ {{136, 108, 76, 60}},
      /* V7  */ {{156, 124, 88, 66}},
      /* V8  */ {{194, 154, 110, 86}},
      /* V9  */ {{232, 182, 132, 100}},
      /* V10 */ {{274, 216, 154, 122}},
      /* V11 */ {{324, 254, 180, 140}},
      /* V12 */ {{370, 290, 206, 158}},
      /* V13 */ {{428, 334, 244, 180}},
      /* V14 */ {{461, 365, 261, 197}},
      /* V15 */ {{523, 415, 295, 223}},
      /* V16 */ {{589, 453, 325, 253}},
      /* V17 */ {{647, 507, 367, 283}},
      /* V18 */ {{721, 563, 397, 313}},
      /* V19 */ {{795, 627, 445, 341}},
      /* V20 */ {{861, 669, 485, 385}},
      /* V21 */ {{932, 714, 512, 406}},
      /* V22 */ {{1006, 782, 568, 442}},
      /* V23 */ {{1094, 860, 614, 464}},
      /* V24 */ {{1174, 914, 664, 514}},
      /* V25 */ {{1276, 1000, 718, 538}},
      /* V26 */ {{1370, 1062, 754, 596}},
      /* V27 */ {{1468, 1128, 808, 628}},
      /* V28 */ {{1531, 1193, 871, 661}},
      /* V29 */ {{1631, 1267, 911, 701}},
      /* V30 */ {{1735, 1373, 985, 745}},
      /* V31 */ {{1843, 1455, 1033, 793}},
      /* V32 */ {{1955, 1541, 1115, 845}},
      /* V33 */ {{2071, 1631, 1171, 901}},
      /* V34 */ {{2191, 1725, 1231, 961}},
      /* V35 */ {{2306, 1812, 1286, 986}},
      /* V36 */ {{2434, 1914, 1354, 1054}},
      /* V37 */ {{2566, 1992, 1426, 1096}},
      /* V38 */ {{2702, 2102, 1502, 1142}},
      /* V39 */ {{2812, 2216, 1582, 1222}},
      /* V40 */ {{2956, 2334, 1666, 1276}},
  }};

  // Character capacity (version-1, ec_level) — Byte mode
  static constexpr std::array<std::array<int, 4>, 40> BYTE_CAPACITY = {{
      {{17, 14, 11, 7}},
      {{32, 26, 20, 14}},
      {{53, 42, 32, 24}},
      {{78, 62, 46, 34}},
      {{106, 84, 60, 44}},
      {{134, 106, 74, 58}},
      {{154, 122, 86, 64}},
      {{192, 152, 108, 84}},
      {{230, 180, 130, 98}},
      {{271, 213, 151, 119}},
      {{321, 251, 177, 137}},
      {{367, 287, 203, 155}},
      {{425, 331, 241, 177}},
      {{458, 362, 258, 194}},
      {{520, 412, 292, 220}},
      {{586, 450, 322, 250}},
      {{644, 504, 364, 280}},
      {{718, 560, 394, 310}},
      {{792, 624, 442, 338}},
      {{858, 666, 482, 382}},
      {{929, 711, 509, 403}},
      {{1003, 779, 565, 439}},
      {{1091, 857, 611, 461}},
      {{1171, 911, 661, 511}},
      {{1273, 997, 715, 535}},
      {{1367, 1059, 751, 593}},
      {{1465, 1125, 805, 625}},
      {{1528, 1190, 868, 658}},
      {{1628, 1264, 908, 698}},
      {{1732, 1370, 982, 742}},
      {{1840, 1452, 1030, 790}},
      {{1952, 1538, 1112, 842}},
      {{2068, 1628, 1168, 898}},
      {{2188, 1722, 1228, 958}},
      {{2303, 1809, 1283, 983}},
      {{2431, 1911, 1351, 1051}},
      {{2563, 1989, 1423, 1093}},
      {{2699, 2099, 1499, 1139}},
      {{2809, 2213, 1579, 1219}},
      {{2953, 2331, 1663, 1273}},
  }};

  // EC block info: {ec_codewords_per_block, blocks_in_group1, codewords_in_group1_block,
  //                 blocks_in_group2, codewords_in_group2_block}
  struct ECBlockInfo
  {
    int ec_cw_per_block;
    int g1_blocks;
    int g1_data_cw;
    int g2_blocks;
    int g2_data_cw;
  };

  // EC block table [version-1][ec_level]
  static constexpr std::array<std::array<ECBlockInfo, 4>, 40> EC_BLOCKS = {{
      /* V1  */ {{{7, 1, 19, 0, 0}, {10, 1, 16, 0, 0}, {13, 1, 13, 0, 0}, {17, 1, 9, 0, 0}}},
      /* V2  */ {{{10, 1, 34, 0, 0}, {16, 1, 28, 0, 0}, {22, 1, 22, 0, 0}, {28, 1, 16, 0, 0}}},
      /* V3  */ {{{15, 1, 55, 0, 0}, {26, 1, 44, 0, 0}, {18, 2, 17, 0, 0}, {22, 2, 13, 0, 0}}},
      /* V4  */ {{{20, 1, 80, 0, 0}, {18, 2, 32, 0, 0}, {26, 2, 24, 0, 0}, {16, 4, 9, 0, 0}}},
      /* V5  */ {{{26, 1, 108, 0, 0}, {24, 2, 43, 0, 0}, {18, 2, 15, 2, 16}, {22, 2, 11, 2, 12}}},
      /* V6  */ {{{18, 2, 68, 0, 0}, {16, 4, 27, 0, 0}, {24, 4, 19, 0, 0}, {28, 4, 15, 0, 0}}},
      /* V7  */ {{{20, 2, 78, 0, 0}, {18, 4, 31, 0, 0}, {18, 2, 14, 4, 15}, {26, 4, 13, 1, 14}}},
      /* V8  */ {{{24, 2, 97, 0, 0}, {22, 2, 38, 2, 39}, {22, 4, 18, 2, 19}, {26, 4, 14, 2, 15}}},
      /* V9  */ {{{30, 2, 116, 0, 0}, {22, 3, 36, 2, 37}, {20, 4, 16, 4, 17}, {24, 4, 12, 4, 13}}},
      /* V10 */ {{{18, 2, 68, 2, 69}, {26, 4, 43, 1, 44}, {24, 6, 19, 2, 20}, {28, 6, 15, 2, 16}}},
      /* V11 */ {{{20, 4, 81, 0, 0}, {30, 1, 50, 4, 51}, {28, 4, 22, 4, 23}, {24, 3, 12, 8, 13}}},
      /* V12 */ {{{24, 2, 92, 2, 93}, {22, 6, 36, 2, 37}, {26, 4, 20, 6, 21}, {28, 7, 14, 4, 15}}},
      /* V13 */ {{{26, 4, 107, 0, 0}, {22, 8, 37, 1, 38}, {24, 8, 20, 4, 21}, {22, 12, 11, 4, 12}}},
      /* V14 */ {{{30, 3, 115, 1, 116}, {24, 4, 40, 5, 41}, {20, 11, 16, 5, 17}, {24, 11, 12, 5, 13}}},
      /* V15 */ {{{22, 5, 87, 1, 88}, {24, 5, 41, 5, 42}, {30, 5, 24, 7, 25}, {24, 11, 12, 7, 13}}},
      /* V16 */ {{{24, 5, 98, 1, 99}, {28, 7, 45, 3, 46}, {24, 15, 19, 2, 20}, {30, 3, 15, 13, 16}}},
      /* V17 */ {{{28, 1, 107, 5, 108}, {28, 10, 46, 1, 47}, {28, 1, 22, 15, 23}, {28, 2, 14, 17, 15}}},
      /* V18 */ {{{30, 5, 120, 1, 121}, {26, 9, 43, 4, 44}, {28, 17, 22, 1, 23}, {28, 2, 14, 19, 15}}},
      /* V19 */ {{{28, 3, 113, 4, 114}, {26, 3, 44, 11, 45}, {26, 17, 21, 4, 22}, {26, 9, 13, 16, 14}}},
      /* V20 */ {{{28, 3, 107, 5, 108}, {26, 3, 41, 13, 42}, {30, 15, 24, 5, 25}, {28, 15, 15, 10, 16}}},
      /* V21 */ {{{28, 4, 116, 4, 117}, {26, 17, 42, 0, 0}, {28, 17, 22, 6, 23}, {30, 19, 16, 6, 17}}},
      /* V22 */ {{{28, 2, 111, 7, 112}, {28, 17, 46, 0, 0}, {30, 7, 24, 16, 25}, {24, 34, 13, 0, 0}}},
      /* V23 */ {{{30, 4, 121, 5, 122}, {28, 4, 47, 14, 48}, {30, 11, 24, 14, 25}, {30, 16, 15, 14, 16}}},
      /* V24 */ {{{30, 6, 117, 4, 118}, {28, 6, 45, 14, 46}, {30, 11, 24, 16, 25}, {30, 30, 16, 2, 17}}},
      /* V25 */ {{{26, 8, 106, 4, 107}, {28, 8, 47, 13, 48}, {30, 7, 24, 22, 25}, {30, 22, 15, 13, 16}}},
      /* V26 */ {{{28, 10, 114, 2, 115}, {28, 19, 46, 4, 47}, {28, 28, 22, 6, 23}, {30, 33, 16, 4, 17}}},
      /* V27 */ {{{30, 8, 122, 4, 123}, {28, 22, 45, 3, 46}, {30, 8, 23, 26, 24}, {30, 12, 15, 28, 16}}},
      /* V28 */ {{{30, 3, 117, 10, 118}, {28, 3, 45, 23, 46}, {30, 4, 24, 31, 25}, {30, 11, 15, 31, 16}}},
      /* V29 */ {{{30, 7, 116, 7, 117}, {28, 21, 45, 7, 46}, {30, 1, 23, 37, 24}, {30, 19, 15, 26, 16}}},
      /* V30 */ {{{30, 5, 115, 10, 116}, {28, 19, 45, 10, 46}, {30, 15, 24, 25, 25}, {30, 23, 15, 25, 16}}},
      /* V31 */ {{{30, 13, 115, 3, 116}, {28, 2, 45, 29, 46}, {30, 42, 24, 1, 25}, {30, 23, 15, 28, 16}}},
      /* V32 */ {{{30, 17, 115, 0, 0}, {28, 10, 45, 23, 46}, {30, 10, 24, 35, 25}, {30, 19, 15, 35, 16}}},
      /* V33 */ {{{30, 17, 115, 1, 116}, {28, 14, 45, 21, 46}, {30, 29, 24, 19, 25}, {30, 11, 15, 46, 16}}},
      /* V34 */ {{{30, 13, 115, 6, 116}, {28, 14, 45, 23, 46}, {30, 44, 24, 7, 25}, {30, 59, 16, 1, 17}}},
      /* V35 */ {{{30, 12, 121, 7, 122}, {28, 12, 45, 26, 46}, {30, 39, 24, 14, 25}, {30, 22, 15, 41, 16}}},
      /* V36 */ {{{30, 6, 121, 14, 122}, {28, 6, 45, 34, 46}, {30, 46, 24, 10, 25}, {30, 2, 15, 64, 16}}},
      /* V37 */ {{{30, 17, 122, 4, 123}, {28, 29, 45, 14, 46}, {30, 49, 24, 10, 25}, {30, 24, 15, 46, 16}}},
      /* V38 */ {{{30, 4, 122, 18, 123}, {28, 13, 45, 32, 46}, {30, 48, 24, 14, 25}, {30, 42, 15, 32, 16}}},
      /* V39 */ {{{30, 20, 117, 4, 118}, {28, 40, 45, 7, 46}, {30, 43, 24, 22, 25}, {30, 10, 15, 67, 16}}},
      /* V40 */ {{{30, 19, 118, 6, 119}, {28, 18, 45, 31, 46}, {30, 34, 24, 34, 25}, {30, 20, 15, 61, 16}}},
  }};

  // Alignment pattern center positions [version-1]
  static constexpr std::array<std::array<int, 7>, 40> ALIGNMENT_POSITIONS = {{
      {},                             // V1
      {6, 18},                        // V2
      {6, 22},                        // V3
      {6, 26},                        // V4
      {6, 30},                        // V5
      {6, 34},                        // V6
      {6, 22, 38},                    // V7
      {6, 24, 42},                    // V8
      {6, 26, 46},                    // V9
      {6, 28, 50},                    // V10
      {6, 30, 54},                    // V11
      {6, 32, 58},                    // V12
      {6, 34, 62},                    // V13
      {6, 26, 46, 66},                // V14
      {6, 26, 48, 70},                // V15
      {6, 26, 50, 74},                // V16
      {6, 30, 54, 78},                // V17
      {6, 30, 56, 82},                // V18
      {6, 30, 58, 86},                // V19
      {6, 34, 62, 90},                // V20
      {6, 28, 50, 72, 94},            // V21
      {6, 26, 50, 74, 98},            // V22
      {6, 30, 54, 78, 102},           // V23
      {6, 28, 54, 80, 106},           // V24
      {6, 32, 58, 84, 110},           // V25
      {6, 30, 58, 86, 114},           // V26
      {6, 34, 62, 90, 118},           // V27
      {6, 26, 50, 74, 98, 122},       // V28
      {6, 30, 54, 78, 102, 126},      // V29
      {6, 26, 52, 78, 104, 130},      // V30
      {6, 30, 56, 82, 108, 132},      // V31
      {6, 34, 60, 86, 112, 136},      // V32
      {6, 30, 58, 86, 114, 142},      // V33
      {6, 34, 62, 90, 118, 146},      // V34
      {6, 30, 54, 78, 102, 126, 150}, // V35
      {6, 24, 50, 76, 102, 128, 154}, // V36
      {6, 28, 54, 80, 106, 132, 158}, // V37
      {6, 32, 58, 84, 110, 136, 162}, // V38
      {6, 26, 54, 82, 110, 138, 166}, // V39
      {6, 30, 58, 86, 114, 142, 170}, // V40
  }};

  // Format info strings [ec_level][mask] — 15-bit, includes BCH ECC
  static constexpr std::array<std::array<uint16_t, 8>, 4> FORMAT_INFO = {{
      /* L */ {{0x77C4, 0x72F3, 0x7DAA, 0x789D, 0x662F, 0x6318, 0x6C41, 0x6976}},
      /* M */ {{0x5412, 0x5125, 0x5E7C, 0x5B4B, 0x45F9, 0x40CE, 0x4F97, 0x4AA0}},
      /* Q */ {{0x355F, 0x3068, 0x3F31, 0x3A06, 0x24B4, 0x2183, 0x2EDA, 0x2BED}},
      /* H */ {{0x1689, 0x13BE, 0x1CE7, 0x19D0, 0x0762, 0x0255, 0x0D0C, 0x083B}},
  }};

  // Version info [version-1] for versions 7+, 18-bit (only versions 7–40 need this)
  static constexpr std::array<uint32_t, 34> VERSION_INFO = {{
      0x07C94,
      0x085BC,
      0x09A99,
      0x0A4D3,
      0x0BBF6,
      0x0C762,
      0x0D847,
      0x0E60D,
      0x0F928,
      0x10B78,
      0x1145D,
      0x12A17,
      0x13532,
      0x149A6,
      0x15683,
      0x168C9,
      0x177EC,
      0x18EC4,
      0x191E1,
      0x1AFAB,
      0x1B08E,
      0x1CC1A,
      0x1D33F,
      0x1ED75,
      0x1F250,
      0x209D5,
      0x216F0,
      0x228BA,
      0x2379F,
      0x24B0B,
      0x2542E,
      0x26A64,
      0x27541,
      0x28C69,
  }};

  // BitBuffer
  [[nodiscard]] inline std::size_t checked_index(int value)
  {
    assert(value >= 0);
    return static_cast<std::size_t>(value);
  }

  class BitBuffer
  {
  public:
    BitBuffer() = default;
    explicit BitBuffer(std::size_t reserve_bits) { data_.reserve((reserve_bits + 7) / 8); }

    void append(uint32_t value, int num_bits)
    {
      assert(num_bits >= 0 && num_bits <= 32);
      for (int i = num_bits - 1; i >= 0; --i)
      {
        if ((value >> i) & 1)
        {
          ensure_bit(bit_length_);
          data_[checked_index(bit_length_ / 8)] |= static_cast<uint8_t>(1 << (7 - (bit_length_ % 8)));
        }
        else
        {
          ensure_bit(bit_length_);
        }
        ++bit_length_;
      }
    }

    void append_bit(bool b) { append(b ? 1 : 0, 1); }

    [[nodiscard]] int size() const noexcept { return bit_length_; }
    [[nodiscard]] bool empty() const noexcept { return bit_length_ == 0; }

    [[nodiscard]] bool get_bit(int index) const
    {
      assert(index >= 0 && index < bit_length_);
      return (data_[checked_index(index / 8)] >> (7 - (index % 8))) & 1;
    }

    [[nodiscard]] uint8_t get_byte(int byte_index) const
    {
      return data_.at(checked_index(byte_index));
    }

    [[nodiscard]] int byte_count() const noexcept { return (bit_length_ + 7) / 8; }
    [[nodiscard]] const std::vector<uint8_t> &bytes() const noexcept { return data_; }

    void pad_to_byte()
    {
      while (bit_length_ % 8 != 0)
        append_bit(false);
    }

    void pad_to_capacity(int total_bytes)
    {
      pad_to_byte();
      static constexpr std::array<uint8_t, 2> PAD_BYTES = {0xEC, 0x11};
      int i = 0;
      while (byte_count() < total_bytes)
      {
        append(PAD_BYTES[i & 1], 8);
        ++i;
      }
    }

  private:
    std::vector<uint8_t> data_;
    int bit_length_ = 0;

    void ensure_bit(int index)
    {
      int needed = (index / 8) + 1;
      while (static_cast<int>(data_.size()) < needed)
        data_.push_back(0);
    }
  };

  // Reed-Solomon ECC
  class ReedSolomon
  {
  public:
    explicit ReedSolomon(int num_ec_codewords) : num_ec_(num_ec_codewords)
    {
      generator_ = build_generator(num_ec_codewords);
    }

    [[nodiscard]] std::vector<uint8_t> compute(std::span<const uint8_t> data) const
    {
      assert(num_ec_ >= 0);
      std::vector<uint8_t> result(checked_index(num_ec_), 0);
      for (uint8_t b : data)
      {
        const uint8_t factor = b ^ result[0];
        result.erase(result.begin());
        result.push_back(0);
        for (int i = 0; i < num_ec_; ++i)
        {
          result[checked_index(i)] ^= gf_mul(generator_[checked_index(i + 1)], factor);
        }
      }
      return result;
    }

  private:
    int num_ec_;
    std::vector<uint8_t> generator_;

    // GF(2^8) log/exp tables with primitive polynomial x^8+x^4+x^3+x^2+1 (0x11D)
    static constexpr int GF_SIZE = 256;
    static constexpr int PRIM_POLY = 0x11D;

    static const std::array<uint8_t, 256> &exp_table()
    {
      static std::array<uint8_t, 256> tbl = []()
      {
        std::array<uint8_t, 256> t{};
        int v = 1;
        for (int i = 0; i < 255; ++i)
        {
          t[checked_index(i)] = static_cast<uint8_t>(v);
          v <<= 1;
          if (v >= 256)
            v ^= PRIM_POLY;
        }
        t[255] = t[0];
        return t;
      }();
      return tbl;
    }

    static const std::array<uint8_t, 256> &log_table()
    {
      static std::array<uint8_t, 256> tbl = []()
      {
        std::array<uint8_t, 256> t{};
        const auto &e = exp_table();
        for (int i = 0; i < 255; ++i)
          t[e[checked_index(i)]] = static_cast<uint8_t>(i);
        return t;
      }();
      return tbl;
    }

    static uint8_t gf_mul(uint8_t a, uint8_t b)
    {
      if (a == 0 || b == 0)
        return 0;
      const auto &exp = exp_table();
      const auto &log = log_table();
      return exp[static_cast<std::size_t>((log[a] + log[b]) % 255)];
    }

    static std::vector<uint8_t> build_generator(int degree)
    {
      std::vector<uint8_t> g = {1};
      const auto &exp = exp_table();
      for (int i = 0; i < degree; ++i)
      {
        std::vector<uint8_t> factor = {1, exp[checked_index(i)]};
        g = poly_mul(g, factor);
      }
      return g;
    }

    static std::vector<uint8_t> poly_mul(const std::vector<uint8_t> &a,
                                         const std::vector<uint8_t> &b)
    {
      std::vector<uint8_t> result(a.size() + b.size() - 1, 0);
      for (std::size_t i = 0; i < a.size(); ++i)
        for (std::size_t j = 0; j < b.size(); ++j)
          result[i + j] ^= gf_mul(a[i], b[j]);
      return result;
    }
  };

  // QRMatrix — internal grid representation
  class QRMatrix
  {
  public:
    explicit QRMatrix(int version)
        : version_(version), size_(17 + 4 * version),
          modules_(static_cast<std::size_t>(size_ * size_), false),
          is_function_(static_cast<std::size_t>(size_ * size_), false)
    {
    }

    [[nodiscard]] int version() const noexcept { return version_; }
    [[nodiscard]] int size() const noexcept { return size_; }

    void set(int row, int col, bool dark, bool function = false)
    {
      modules_[idx(row, col)] = dark;
      if (function)
        is_function_[idx(row, col)] = true;
    }

    [[nodiscard]] bool get(int row, int col) const noexcept
    {
      return modules_[idx(row, col)];
    }

    [[nodiscard]] bool is_function(int row, int col) const noexcept
    {
      return is_function_[idx(row, col)];
    }

    // Apply one of the 8 mask patterns to data modules only
    void apply_mask(int mask)
    {
      assert(mask >= 0 && mask <= 7);
      for (int r = 0; r < size_; ++r)
      {
        for (int c = 0; c < size_; ++c)
        {
          if (!is_function_[idx(r, c)] && mask_condition(mask, r, c))
          {
            modules_[idx(r, c)] = !modules_[idx(r, c)];
          }
        }
      }
    }

    // Flip back a mask (same operation since XOR)
    void remove_mask(int mask) { apply_mask(mask); }

    [[nodiscard]] const std::vector<bool> &raw() const noexcept { return modules_; }

  private:
    int version_, size_;
    std::vector<bool> modules_;
    std::vector<bool> is_function_;

    [[nodiscard]] std::size_t idx(int r, int c) const noexcept
    {
      return static_cast<std::size_t>(r * size_ + c);
    }

    static bool mask_condition(int mask, int r, int c) noexcept
    {
      switch (mask)
      {
      case 0:
        return (r + c) % 2 == 0;
      case 1:
        return r % 2 == 0;
      case 2:
        return c % 3 == 0;
      case 3:
        return (r + c) % 3 == 0;
      case 4:
        return (r / 2 + c / 3) % 2 == 0;
      case 5:
        return (r * c) % 2 + (r * c) % 3 == 0;
      case 6:
        return ((r * c) % 2 + (r * c) % 3) % 2 == 0;
      case 7:
        return ((r + c) % 2 + (r * c) % 3) % 2 == 0;
      default:
        return false;
      }
    }
  };

  // Penalty Calculator
  class PenaltyCalculator
  {
  public:
    [[nodiscard]] static int compute(const QRMatrix &m)
    {
      return rule1(m) + rule2(m) + rule3(m) + rule4(m);
    }

  private:
    // Rule 1: runs of 5+ same-colour modules in a row/col
    static int rule1(const QRMatrix &m)
    {
      int penalty = 0;
      const int sz = m.size();
      auto run_penalty = [&](int run)
      { if (run >= 5) penalty += 3 + (run - 5); };

      for (int r = 0; r < sz; ++r)
      {
        int run = 1;
        for (int c = 1; c < sz; ++c)
        {
          if (m.get(r, c) == m.get(r, c - 1))
            ++run;
          else
          {
            run_penalty(run);
            run = 1;
          }
        }
        run_penalty(run);
      }
      for (int c = 0; c < sz; ++c)
      {
        int run = 1;
        for (int r = 1; r < sz; ++r)
        {
          if (m.get(r, c) == m.get(r - 1, c))
            ++run;
          else
          {
            run_penalty(run);
            run = 1;
          }
        }
        run_penalty(run);
      }
      return penalty;
    }

    // Rule 2: 2×2 blocks of same colour
    static int rule2(const QRMatrix &m)
    {
      int penalty = 0;
      const int sz = m.size();
      for (int r = 0; r < sz - 1; ++r)
        for (int c = 0; c < sz - 1; ++c)
        {
          bool v = m.get(r, c);
          if (m.get(r, c + 1) == v && m.get(r + 1, c) == v && m.get(r + 1, c + 1) == v)
            penalty += 3;
        }
      return penalty;
    }

    // Rule 3: specific patterns 1:1:3:1:1 with 4 light modules
    static int rule3(const QRMatrix &m)
    {
      int penalty = 0;
      const int sz = m.size();
      static constexpr std::array<bool, 11> P1 = {true, false, true, true, true, false, true, false, false, false, false};
      static constexpr std::array<bool, 11> P2 = {false, false, false, false, true, false, true, true, true, false, true};

      auto check = [&](bool horiz)
      {
        for (int a = 0; a < sz; ++a)
          for (int b = 0; b <= sz - 11; ++b)
          {
            bool match1 = true, match2 = true;
            for (int k = 0; k < 11; ++k)
            {
              bool v = horiz ? m.get(a, b + k) : m.get(b + k, a);
              if (v != P1[checked_index(k)])
                match1 = false;
              if (v != P2[checked_index(k)])
                match2 = false;
            }
            if (match1)
              penalty += 40;
            if (match2)
              penalty += 40;
          }
      };
      check(true);
      check(false);
      return penalty;
    }

    // Rule 4: proportion of dark modules
    static int rule4(const QRMatrix &m)
    {
      const int sz = m.size();
      int dark = 0, total = sz * sz;
      for (int r = 0; r < sz; ++r)
        for (int c = 0; c < sz; ++c)
          if (m.get(r, c))
            ++dark;
      const int pct = (dark * 100) / total;
      const int prev5 = (pct / 5) * 5;
      const int next5 = prev5 + 5;
      return 10 * std::min(std::abs(prev5 - 50) / 5, std::abs(next5 - 50) / 5);
    }
  };

  // Data Encoder
  class DataEncoder
  {
  public:
    [[nodiscard]] static Mode detect_mode(std::string_view data)
    {
      if (is_numeric(data))
        return Mode::Numeric;
      if (is_alphanumeric(data))
        return Mode::Alphanumeric;
      return Mode::Byte;
    }

    [[nodiscard]] static BitBuffer encode(std::string_view data, Mode mode,
                                          int version, int total_data_bytes)
    {
      BitBuffer buf;
      // Mode indicator (4 bits)
      buf.append(mode_indicator(mode), 4);
      // Character count indicator
      buf.append(static_cast<uint32_t>(data.size()),
                 char_count_bits(mode, version));
      // Data
      switch (mode)
      {
      case Mode::Numeric:
        encode_numeric(buf, data);
        break;
      case Mode::Alphanumeric:
        encode_alphanumeric(buf, data);
        break;
      case Mode::Byte:
        encode_byte(buf, data);
        break;
      }
      // Terminator (up to 4 zero bits)
      int rem = total_data_bytes * 8 - buf.size();
      buf.append(0, std::min(4, rem));
      // Pad to byte boundary + pad bytes
      buf.pad_to_capacity(total_data_bytes);
      return buf;
    }

  private:
    static bool is_numeric(std::string_view s)
    {
      return std::ranges::all_of(s, [](char c)
                                 { return c >= '0' && c <= '9'; });
    }

    static bool is_alphanumeric(std::string_view s)
    {
      return std::ranges::all_of(s, [](char c)
                                 { return std::string_view("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:").find(c) != std::string_view::npos; });
    }

    static int alphanumeric_value(char c)
    {
      static constexpr std::string_view CHARS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:";
      auto pos = CHARS.find(c);
      return (pos == std::string_view::npos) ? -1 : static_cast<int>(pos);
    }

    static constexpr uint8_t mode_indicator(Mode m)
    {
      switch (m)
      {
      case Mode::Numeric:
        return 0b0001;
      case Mode::Alphanumeric:
        return 0b0010;
      case Mode::Byte:
        return 0b0100;
      }
      return 0;
    }

    static int char_count_bits(Mode m, int version)
    {
      if (version <= 9)
      {
        switch (m)
        {
        case Mode::Numeric:
          return 10;
        case Mode::Alphanumeric:
          return 9;
        case Mode::Byte:
          return 8;
        }
      }
      else if (version <= 26)
      {
        switch (m)
        {
        case Mode::Numeric:
          return 12;
        case Mode::Alphanumeric:
          return 11;
        case Mode::Byte:
          return 16;
        }
      }
      else
      {
        switch (m)
        {
        case Mode::Numeric:
          return 14;
        case Mode::Alphanumeric:
          return 13;
        case Mode::Byte:
          return 16;
        }
      }
      return 8;
    }

    static void encode_numeric(BitBuffer &buf, std::string_view data)
    {
      std::size_t i = 0;
      while (i + 3 <= data.size())
      {
        const auto first = static_cast<uint32_t>(data[i] - '0');
        const auto second = static_cast<uint32_t>(data[i + 1] - '0');
        const auto third = static_cast<uint32_t>(data[i + 2] - '0');
        const uint32_t v = first * 100U + second * 10U + third;
        buf.append(v, 10);
        i += 3;
      }
      if (data.size() - i == 2)
      {
        const auto first = static_cast<uint32_t>(data[i] - '0');
        const auto second = static_cast<uint32_t>(data[i + 1] - '0');
        const uint32_t v = first * 10U + second;
        buf.append(v, 7);
      }
      else if (data.size() - i == 1)
      {
        buf.append(static_cast<uint32_t>(data[i] - '0'), 4);
      }
    }

    static void encode_alphanumeric(BitBuffer &buf, std::string_view data)
    {
      std::size_t i = 0;
      while (i + 2 <= data.size())
      {
        uint32_t v = static_cast<uint32_t>(alphanumeric_value(data[i])) * 45 + static_cast<uint32_t>(alphanumeric_value(data[i + 1]));
        buf.append(v, 11);
        i += 2;
      }
      if (data.size() - i == 1)
        buf.append(static_cast<uint32_t>(alphanumeric_value(data[i])), 6);
    }

    static void encode_byte(BitBuffer &buf, std::string_view data)
    {
      for (char c : data)
        buf.append(static_cast<unsigned char>(c), 8);
    }
  };

  // Matrix Builder — places all function patterns and data
  class MatrixBuilder
  {
  public:
    static QRMatrix build(int version, ECLevel ec, const std::vector<uint8_t> &data_codewords)
    {
      QRMatrix m(version);
      place_finder_patterns(m);
      place_separators(m);
      place_alignment_patterns(m);
      place_timing_patterns(m);
      place_dark_module(m);
      reserve_format_area(m);
      if (version >= 7)
        reserve_version_area(m);
      place_data_bits(m, data_codewords);
      int best_mask = choose_best_mask(m, ec);
      m.apply_mask(best_mask);
      place_format_info(m, ec, best_mask);
      if (version >= 7)
        place_version_info(m, version);
      return m;
    }

  private:
    // Finder Pattern
    static void place_finder_pattern(QRMatrix &m, int top, int left)
    {
      for (int dr = -1; dr <= 7; ++dr)
      {
        for (int dc = -1; dc <= 7; ++dc)
        {
          int r = top + dr, c = left + dc;
          if (r < 0 || r >= m.size() || c < 0 || c >= m.size())
            continue;
          bool dark;
          if (dr == -1 || dr == 7 || dc == -1 || dc == 7)
            dark = false;
          else if (dr == 0 || dr == 6 || dc == 0 || dc == 6)
            dark = true;
          else if (dr >= 2 && dr <= 4 && dc >= 2 && dc <= 4)
            dark = true;
          else
            dark = false;
          m.set(r, c, dark, /*function=*/true);
        }
      }
    }

    static void place_finder_patterns(QRMatrix &m)
    {
      const int sz = m.size();
      place_finder_pattern(m, 0, 0);      // top-left
      place_finder_pattern(m, 0, sz - 7); // top-right
      place_finder_pattern(m, sz - 7, 0); // bottom-left
    }

    // Separators
    static void place_separators(QRMatrix &m)
    {
      const int sz = m.size();
      auto set_sep = [&](int r, int c)
      {
        if (r >= 0 && r < sz && c >= 0 && c < sz)
          m.set(r, c, false, true);
      };
      for (int i = 0; i < 8; ++i)
      {
        set_sep(7, i);
        set_sep(i, 7); // top-left
        set_sep(7, sz - 8 + i);
        set_sep(i, sz - 8); // top-right (partial)
        set_sep(sz - 8 + i, 7);
        set_sep(sz - 8, i); // bottom-left
      }
      // extra corners
      set_sep(7, sz - 8);
      set_sep(sz - 8, 7);
    }

    // Alignment Patterns
    static void place_alignment_pattern(QRMatrix &m, int cx, int cy)
    {
      for (int dr = -2; dr <= 2; ++dr)
      {
        for (int dc = -2; dc <= 2; ++dc)
        {
          int r = cx + dr, c = cy + dc;
          if (r < 0 || r >= m.size() || c < 0 || c >= m.size())
            continue;
          if (m.is_function(r, c))
            continue;
          bool dark = (std::abs(dr) == 2 || std::abs(dc) == 2 || (dr == 0 && dc == 0));
          m.set(r, c, dark, true);
        }
      }
    }

    static void place_alignment_patterns(QRMatrix &m)
    {
      const auto &positions = ALIGNMENT_POSITIONS[static_cast<std::size_t>(m.version() - 1)];
      for (int r : positions)
        for (int c : positions)
          if (r != 0 && c != 0 && !m.is_function(r, c))
            place_alignment_pattern(m, r, c);
    }

    // Timing Patterns
    static void place_timing_patterns(QRMatrix &m)
    {
      const int sz = m.size();
      for (int i = 8; i < sz - 8; ++i)
      {
        bool dark = (i % 2 == 0);
        m.set(6, i, dark, true);
        m.set(i, 6, dark, true);
      }
    }

    // Dark Module
    static void place_dark_module(QRMatrix &m)
    {
      m.set(4 * m.version() + 9, 8, true, true);
    }

    // Format Info Area (reserved, filled later)
    static void reserve_format_area(QRMatrix &m)
    {
      const int sz = m.size();
      // Around top-left finder
      for (int i = 0; i <= 8; ++i)
      {
        m.set(8, i, false, true);
        m.set(i, 8, false, true);
      }
      m.set(8, 8, false, true);
      // Top-right
      for (int i = 0; i < 8; ++i)
        m.set(8, sz - 1 - i, false, true);
      // Bottom-left
      for (int i = 0; i < 7; ++i)
        m.set(sz - 1 - i, 8, false, true);
    }

    // Version Info Area (versions 7+)
    static void reserve_version_area(QRMatrix &m)
    {
      const int sz = m.size();
      for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 3; ++j)
        {
          m.set(i, sz - 11 + j, false, true);
          m.set(sz - 11 + j, i, false, true);
        }
    }

    // Data Placement
    static void place_data_bits(QRMatrix &m, const std::vector<uint8_t> &cw)
    {
      const int sz = m.size();
      int bit_idx = 0;
      int total_bits = static_cast<int>(cw.size()) * 8;

      // Iterate columns right-to-left in pairs, skipping column 6 (timing)
      bool going_up = true;
      int col = sz - 1;
      while (col >= 1)
      {
        if (col == 6)
        {
          --col;
          continue;
        }
        for (int dy = 0; dy < sz; ++dy)
        {
          int row = going_up ? (sz - 1 - dy) : dy;
          for (int dc = 0; dc < 2; ++dc)
          {
            int c = col - dc;
            if (!m.is_function(row, c))
            {
              bool bit = false;
              if (bit_idx < total_bits)
              {
                int byte_i = bit_idx / 8;
                int bit_i = 7 - (bit_idx % 8);
                bit = (cw[checked_index(byte_i)] >> bit_i) & 1;
                ++bit_idx;
              }
              m.set(row, c, bit, false);
            }
          }
        }
        going_up = !going_up;
        col -= 2;
      }
    }

    // Mask Selection
    static int choose_best_mask(QRMatrix &m, ECLevel ec)
    {
      int best_mask = 0;
      int best_penalty = std::numeric_limits<int>::max();
      for (int mask = 0; mask < 8; ++mask)
      {
        m.apply_mask(mask);
        place_format_info(m, ec, mask);
        int p = PenaltyCalculator::compute(m);
        if (p < best_penalty)
        {
          best_penalty = p;
          best_mask = mask;
        }
        m.remove_mask(mask);
        // clear format area for next iteration
        reserve_format_area(m);
      }
      return best_mask;
    }

    // Format Information
    static void place_format_info(QRMatrix &m, ECLevel ec, int mask)
    {
      const int sz = m.size();
      const auto ec_index = checked_index(static_cast<int>(ec));
      const auto mask_index = checked_index(mask);
      uint16_t fi = FORMAT_INFO[ec_index][mask_index];
      // XOR with mask 101010000010010
      fi ^= 0x5412;

      // Top-left vertical (rows 0..5,7,8) and horizontal (cols 0..5,7,8)
      for (int i = 0; i < 6; ++i)
      {
        bool b = (fi >> i) & 1;
        m.set(8, i, b, true);
        m.set(i, 8, b, true);
      }
      m.set(8, 7, (fi >> 6) & 1, true);
      m.set(7, 8, (fi >> 6) & 1, true);
      m.set(8, 8, (fi >> 7) & 1, true);
      // skip row/col 6 (timing), continue at 7,8
      m.set(8, 8, (fi >> 7) & 1, true);

      // Top-right (cols sz-8 to sz-1) horizontal
      for (int i = 0; i < 8; ++i)
        m.set(8, sz - 1 - i, (fi >> i) & 1, true);

      // Bottom-left (rows sz-7 to sz-1) vertical
      for (int i = 0; i < 7; ++i)
        m.set(sz - 7 + i, 8, (fi >> (14 - i)) & 1, true);

      // Correct placement for top-left horizontal bits 8..14
      // row 8, cols 0..5, then 7
      for (int i = 0; i < 6; ++i)
        m.set(8, 5 - i, (fi >> (i + 9)) & 1, true);
      m.set(8, 7, (fi >> 15) & 1, true); // already written above but ensure

      // Actually use standard ISO table — rewrite cleanly
      // Top-left area: row 8 cols 0–8, row/col 8 rows 0–8
      static constexpr std::array<std::pair<int, int>, 15> TL_COORDS = {{{8, 0}, {8, 1}, {8, 2}, {8, 3}, {8, 4}, {8, 5}, {8, 7}, {8, 8}, {7, 8}, {5, 8}, {4, 8}, {3, 8}, {2, 8}, {1, 8}, {0, 8}}};
      // ISO standard layout:
      // bits 0..5 -> row 8, cols 0..5 (horizontal) and rows 0..5, col 8 (vertical)
      // bit 6 -> row 8, col 7 and row 7, col 8
      // bit 7 -> row 8, col 8 and (dark module stays)
      // bits 8..14 -> top-right (row 8) and bottom-left (col 8)
      uint16_t data = FORMAT_INFO[ec_index][mask_index];
      for (int i = 0; i < 15; ++i)
      {
        bool b = (data >> i) & 1;
        auto [r1, c1] = TL_COORDS[checked_index(i)];
        m.set(r1, c1, b, true);
      }
      // Top-right
      for (int i = 0; i < 8; ++i)
        m.set(8, sz - 1 - i, (data >> i) & 1, true);
      // Bottom-left
      for (int i = 8; i < 15; ++i)
        m.set(sz - 15 + i, 8, (data >> i) & 1, true);
      // Dark module stays dark
      m.set(4 * m.version() + 9, 8, true, true);
    }

    // Version Information (versions 7+)
    static void place_version_info(QRMatrix &m, int version)
    {
      if (version < 7)
        return;
      const int sz = m.size();
      uint32_t vi = VERSION_INFO[checked_index(version - 7)];
      for (int i = 0; i < 18; ++i)
      {
        bool b = (vi >> i) & 1;
        int r = i / 3, c = i % 3;
        m.set(r, sz - 11 + c, b, true);
        m.set(sz - 11 + c, r, b, true);
      }
    }
  };

  // Interleaver — handles EC block structure
  class Interleaver
  {
  public:
    [[nodiscard]] static std::vector<uint8_t> interleave(int version, ECLevel ec,
                                                         const std::vector<uint8_t> &data_bytes)
    {
      assert(version >= 1 && version <= 40);
      const auto &info = EC_BLOCKS[checked_index(version - 1)][checked_index(static_cast<int>(ec))];

      // Split data into blocks
      std::vector<std::vector<uint8_t>> blocks;
      int offset = 0;
      // Group 1
      for (int b = 0; b < info.g1_blocks; ++b)
      {
        blocks.emplace_back(data_bytes.begin() + offset,
                            data_bytes.begin() + offset + info.g1_data_cw);
        offset += info.g1_data_cw;
      }
      // Group 2
      for (int b = 0; b < info.g2_blocks; ++b)
      {
        blocks.emplace_back(data_bytes.begin() + offset,
                            data_bytes.begin() + offset + info.g2_data_cw);
        offset += info.g2_data_cw;
      }

      // Compute ECC for each block
      ReedSolomon rs(info.ec_cw_per_block);
      std::vector<std::vector<uint8_t>> ec_blocks;
      for (auto &blk : blocks)
        ec_blocks.push_back(rs.compute(blk));

      // Interleave data codewords
      std::vector<uint8_t> result;
      std::size_t max_data = 0;
      for (auto &b : blocks)
        max_data = std::max(max_data, b.size());
      for (std::size_t i = 0; i < max_data; ++i)
        for (auto &b : blocks)
          if (i < b.size())
            result.push_back(b[i]);

      // Interleave ECC codewords
      for (std::size_t i = 0; i < static_cast<std::size_t>(info.ec_cw_per_block); ++i)
        for (auto &e : ec_blocks)
          result.push_back(e[i]);

      return result;
    }
  };

  // Version Selector
  class VersionSelector
  {
  public:
    [[nodiscard]] static int select(std::string_view data, ECLevel ec, Mode mode)
    {
      const int ec_idx = static_cast<int>(ec);
      for (int v = 1; v <= 40; ++v)
      {
        if (fits(data, v, ec_idx, mode))
          return v;
      }
      throw std::overflow_error("Data too long for any QR code version");
    }

  private:
    static bool fits(std::string_view data, int version, int ec_idx, Mode)
    {
      assert(version >= 1 && version <= 40 && ec_idx >= 0 && ec_idx < 4);
      int cap = BYTE_CAPACITY[checked_index(version - 1)][checked_index(ec_idx)];
      return static_cast<int>(data.size()) <= cap;
    }
  };

  /**
   * @brief Renders compact terminal QR matrices.
   *
   * Two matrix rows are packed into each text line with Unicode half blocks.
   */
  class ASCIIRenderer
  {
  public:
    [[nodiscard]] static std::string render(const QRMatrix &m, const ASCIIOptions &opts = {})
    {
      const int sz = m.size();
      const int qs = opts.quiet_zone;
      const int total = sz + 2 * qs;

      std::string result;
      result.reserve(static_cast<std::size_t>(total) *
                     static_cast<std::size_t>((total + 1) / 2) * 20U);

      for (int row = 0; row < total; row += 2)
      {
        for (int col = 0; col < total; ++col)
        {
          const bool top = module(m, row - qs, col - qs);
          const bool bottom = module(m, row + 1 - qs, col - qs);
          append_cell(result, top, bottom, opts.use_ansi);
        }

        if (opts.use_ansi)
          result += "\x1b[0m";
        result += '\n';
      }

      return result;
    }

  private:
    [[nodiscard]] static bool module(const QRMatrix &matrix, int row, int col)
    {
      return row >= 0 && row < matrix.size() && col >= 0 && col < matrix.size() &&
             matrix.get(row, col);
    }

    static void append_cell(std::string &result, bool top, bool bottom, bool use_ansi)
    {
      if (use_ansi)
      {
        result += bottom ? "\x1b[30" : "\x1b[37";
        result += top ? ";40m" : ";47m";
        result += "\u2584";
        return;
      }

      if (top && bottom)
        result += "\u2588";
      else if (top)
        result += "\u2580";
      else if (bottom)
        result += "\u2584";
      else
        result += ' ';
    }
  };

  // QR code matrix and terminal renderer.
  class QRCode
  {
  public:
    explicit QRCode(QRMatrix matrix, std::string_view data)
        : matrix_(std::move(matrix)), data_(data)
    {
    }

    [[nodiscard]] std::string to_ascii(const ASCIIOptions &opts = {}) const
    {
      return ASCIIRenderer::render(matrix_, opts);
    }

    [[nodiscard]] std::string_view original_data() const noexcept
    {
      return data_;
    }

    [[nodiscard]] int size() const noexcept
    {
      return matrix_.size();
    }

    [[nodiscard]] bool module(int row, int col) const noexcept
    {
      return matrix_.get(row, col);
    }

    [[nodiscard]] bool is_function_module(int row, int col) const noexcept
    {
      return matrix_.is_function(row, col);
    }

  private:
    QRMatrix matrix_;
    std::string data_;
  };

  /**
   * @brief Generate a QR code from a string.
   *
   * @param data   The data to encode.
   * @param ec     Error correction level (default: M).
   * @return       A ready-to-use QRCode object.
   */
  [[nodiscard]] inline QRCode generate(std::string_view data, ECLevel ec = ECLevel::M)
  {
    if (data.empty())
      throw std::invalid_argument("QR content must not be empty");

    const Mode mode = DataEncoder::detect_mode(data);
    const int version = VersionSelector::select(data, ec, mode);
    const int data_codewords = DATA_CODEWORDS[checked_index(version - 1)][checked_index(static_cast<int>(ec))];
    BitBuffer encoded = DataEncoder::encode(data, mode, version, data_codewords);
    std::vector<uint8_t> data_bytes = encoded.bytes();
    data_bytes.resize(static_cast<std::size_t>(data_codewords), 0);

    return QRCode(
        MatrixBuilder::build(
            version,
            ec,
            Interleaver::interleave(version, ec, data_bytes)),
        data);
  }

  } // namespace internal
} // namespace softadastra

#endif // SOFTADASTRA_INTERNAL_QR_ENCODER_HPP
