#pragma once

#include <iostream>
#include <string>

namespace encryption {

#define shift(x, n) (((x) << (n)) | ((x) >> (32 - (n))))  //右移的时候，高位一定要补零，而不是补充符号位
#define F(x, y, z)  (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z)  (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z)  ((x) ^ (y) ^ (z))
#define I(x, y, z)  ((y) ^ ((x) | (~z)))


class MD5 {
public:
    std::string ciphertext = "";
    std::string Encrypt(std::string plaintext)
    {
#define A 0x67452301
#define B 0xefcdab89
#define C 0x98badcfe
#define D 0x10325476

        atemp                 = A;
        btemp                 = B;
        ctemp                 = C;
        dtemp                 = D;
        unsigned int *strByte = add(plaintext);
        for (unsigned int i = 0; i < strlength / 16; i++)  //按16分组
        {
            unsigned int num[16];
            for (unsigned int j = 0; j < 16; j++) {
                num[j] = strByte[i * 16 + j];
            }
            mainLoop(num);
        }
        ciphertext = changeHex(atemp).append(changeHex(btemp)).append(changeHex(ctemp)).append(changeHex(dtemp));
        return ciphertext;
    }

private:
    //strByte的长度
    unsigned int strlength;
    //A,B,C,D的临时变量
    unsigned int       atemp;
    unsigned int       btemp;
    unsigned int       ctemp;
    unsigned int       dtemp;
    const char         str16[17] = "0123456789abcdef";
    const unsigned int t[4][16]  = {
        { 0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501, 0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821 },
        { 0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8, 0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a },
        { 0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70, 0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665 },
        { 0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1, 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391 }

    };  //常量ti unsigned int(abs(sin(i+1))*(2pow32))

    const unsigned int shiftBit[4][4] = {
        { 7, 12, 17, 22 },
        { 5, 9, 14, 20 },
        { 4, 11, 16, 23 },
        { 6, 10, 15, 21 }
    };  //循环左移-位数表

    const unsigned short num[4][16] = {
        { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },  // i % 16
        { 1, 6, 11, 0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12 },  //(5 * i + 1) % 16
        { 5, 8, 11, 14, 1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15, 2 },  //(3 * i + 5) % 16
        { 0, 7, 14, 5, 12, 3, 10, 1, 8, 15, 6, 13, 4, 11, 2, 9 }   //(7 * i) % 16
    };                                                             //数据坐标表

    /*
	  *填充函数
	  *处理后应满足bits≡448(mod512),字节就是bytes≡56（mode64)
	  *填充方式为先加一个1,其它位补零
	  *最后加上64位的原来长度
	*/
    unsigned int *add(std::string str)
    {
        unsigned int  num     = ((str.length() + 8) / 64) + 1;  //以512位,64个字节为一组 最终的组数
        unsigned int *strByte = new unsigned int[num * 16];     //一个数字 一个组分成16个32位子组
        strlength             = num * 16;
        for (unsigned int i = 0; i < num * 16; i++) {
            strByte[i] = 0;
        }
        for (unsigned int i = 0; i < str.length(); i++) {
            /*一个整数存储四个字节，i>>2表示i/4 一个unsigned int对应4个字节，保存4个字符信息*/
            strByte[i >> 2] |= (str[i]) << ((i % 4) * 8);
        }
        /*尾部添加1 一个unsigned int保存4个字符信息,所以用128左移*/
        strByte[str.length() >> 2] |= 0x80 << (((str.length() % 4)) * 8);
        /*添加原长度，长度指位的长度，所以要乘8，然后是小端序，所以放在倒数第二个,这里长度只用了32位*/
        strByte[num * 16 - 2] = str.length() * 8;
        return strByte;
    }

    void mainLoop(unsigned int M[])
    {
        unsigned int f, g;
        unsigned     a = atemp;
        unsigned     b = btemp;
        unsigned     c = ctemp;
        unsigned     d = dtemp;
        for (unsigned int i = 0; i < 64; i++) {
            if (i < 16) {
                f = F(b, c, d);
                g = i;
            } else if (i < 32) {
                f = G(b, c, d);
                g = (5 * i + 1) % 16;
            } else if (i < 48) {
                f = H(b, c, d);
                g = (3 * i + 5) % 16;
            } else {
                f = I(b, c, d);
                g = (7 * i) % 16;
            }
            unsigned int tmp = d;
            d                = c;
            c                = b;
            /* a = b + ( (a + F(b,c,d) + Mj + ti) << s)*/
            b = b + shift((a + f + t[i / 16][i % 16] + M[g]), shiftBit[i / 16][i % 4]);
            a = tmp;
        }
        atemp = a + atemp;
        btemp = b + btemp;
        ctemp = c + ctemp;
        dtemp = d + dtemp;
    }

    //转换为16进制数  （实在没看懂） 百科的代码
    std::string changeHex(int a)
    {
        int         b;
        std::string str1;
        std::string str = "";
        for (int i = 0; i < 4; i++) {
            str1 = "";
            b    = ((a >> i * 8) % (1 << 8)) & 0xff;  //逆序处理每个字节
            for (int j = 0; j < 2; j++) {
                str1.insert(0, 1, str16[b % 16]);
                b = b / 16;
            }
            str += str1;
        }
        return str;
    }
};
}  // namespace encryption