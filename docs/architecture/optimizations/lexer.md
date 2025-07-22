# Lexer optimizations

My original lexer implementation's performance looked like this:

```shell
/home/areg/projects/tachyon/cmake-build-release-benchmarks-only/libs/lexer/lexer_benchmarks
2025-07-14T09:45:45-04:00
Running /home/areg/projects/tachyon/cmake-build-release-benchmarks-only/libs/lexer/lexer_benchmarks
Run on (8 X 3100.1 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x4)
  L1 Instruction 32 KiB (x4)
  L2 Unified 256 KiB (x4)
  L3 Unified 6144 KiB (x1)
Load Average: 3.12, 2.31, 1.89
-------------------------------------------------------------------------
Benchmark                               Time             CPU   Iterations
-------------------------------------------------------------------------
LexerDataFixture/LexBlank            63.8 ns         63.1 ns     11318084
LexerDataFixture/LexBasic            1381 ns         1369 ns       457484
LexerDataFixture/Lex1000Chars       18119 ns        17961 ns        39048
LexerDataFixture/Lex2000Chars       34216 ns        33844 ns        20672
LexerDataFixture/Lex5000Chars      151496 ns       149748 ns         4715
LexerDataFixture/Lex10000Chars     390965 ns       386551 ns         1768
LexerDataFixture/Lex20000Chars     786364 ns       778392 ns          865
LexerDataFixture/LexBigComment      21277 ns        21048 ns        33004

Process finished with exit code 0
```

Which, by itself, is not bad. It takes about 1ms to lex 20000 characters, which is already a
significantly large Tachyon program. The benchmark program `06-20000-chars.tachyon` is over 1000
lines of code. However, there are some key performance issues that I think can provide
significant performance benefits to Tachyon. By the end of this performance refactor, my
benchmarks will look like this (target `RelWithDebInfo`):

```shell
/home/areg/projects/tachyon/cmake-build-release-benchmarks-only/libs/lexer/lexer_benchmarks
2025-07-14T12:09:32-04:00
Running /home/areg/projects/tachyon/cmake-build-release-benchmarks-only/libs/lexer/lexer_benchmarks
Run on (8 X 3100.2 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x4)
  L1 Instruction 32 KiB (x4)
  L2 Unified 256 KiB (x4)
  L3 Unified 6144 KiB (x1)
Load Average: 1.72, 1.55, 1.49
-------------------------------------------------------------------------
Benchmark                               Time             CPU   Iterations
-------------------------------------------------------------------------
LexerDataFixture/LexBlank             113 ns          112 ns      6341061
LexerDataFixture/LexBasic             287 ns          286 ns      2402244
LexerDataFixture/Lex1000Chars        3662 ns         3650 ns       172662
LexerDataFixture/Lex2000Chars        6921 ns         6899 ns       102302
LexerDataFixture/Lex5000Chars       30090 ns        29996 ns        23380
LexerDataFixture/Lex10000Chars      71156 ns        70868 ns         9913
LexerDataFixture/Lex20000Chars     134971 ns       134296 ns         5357
LexerDataFixture/LexBigComment      17539 ns        17469 ns        39073

Process finished with exit code 0
```

Notably, lexing 20000 characters received a 619% performance boost, and now runs faster than it
used to take to lex just 5000 characters.[^1]

[^1]: the blank fixture is now running ~50ns slower. I think this is because of when I reserve
lots of memory for the token and constant vectors. Since this is only a 50ns loss for something
that saves lots more with larger inputs, I'm o.k. with this loss. You can see, for example, that
LexBasic is already much faster.

Below, you can find a quick walkthrough behind the thought process for each optimization. You
can find the pre-optimization code at or before commit
[`48f2f4a260c7e877209d46f064bea549b335d03b`](https://github.com/kostareg/tachyon/tree/48f2f4a260c7e877209d46f064bea549b335d03b),
and the post-optimization code at or after commit
[`1770d4c62dcce461e8c9ede78bfea75183c03e06`](https://github.com/kostareg/tachyon/tree/1770d4c62dcce461e8c9ede78bfea75183c03e06).

## 1. Keyword hash tables

One such key performance issue was the keyword lexing process. In the old implementation,
keyword recognition consisted of:

```c++
// ...
else if (s.substr(pos, 4) == "True" && !isalpha(s[pos + 4]))     { /* ... */ }
else if (s.substr(pos, 5) == "False" && !isalpha(s[pos + 5]))    { /* ... */ }
else if (s.substr(pos, 6) == "import" && !isalpha(s[pos + 6]))   { /* ... */ }
else if (s.substr(pos, 2) == "fn" && !isalpha(s[pos + 2]))       { /* ... */ }
else if (s.substr(pos, 6) == "return" && !isalpha(s[pos + 6]))   { /* ... */ }
else if (s.substr(pos, 5) == "while" && !isalpha(s[pos + 5]))    { /* ... */ }
else if (s.substr(pos, 5) == "break" && !isalpha(s[pos + 5]))    { /* ... */ }
else if (s.substr(pos, 8) == "continue" && !isalpha(s[pos + 8])) { /* ... */ }
```

Which has way too many instructions per keyword occurence:

* take a substring of a certain length
* compare it to the keyword string
* ensure that the next character is not an alphanumeric value[^2]
* repeat for each kind of keyword (import, fn, return, ...)

[^2]: I did this so that identifier such as `returned` or `continued` were possible. As I 
discover in this implementation, it was much easier to just parse the whole thing as an 
identifier first, then attempt match it to a keyword based on length.

I was inspired by a [YouTube video by Strager](https://www.youtube.com/watch?v=DMQ_HcNSOAI) to use
a perfect hash table to increase performance. For this, I used a GNU tool called
[gperf](https://www.gnu.org/software/gperf/). Gperf generates a perfect hash function given a set
of keywords (which you can find in `/libs/lexer/keywords.gperf`). If the input string matches the
generated hash, it returns a `Keyword` type that has the corresponding `TokenType` that we are
looking for.

This reduces the instructions per keyword occurence to:

* hash the keyword
* compare the input to the hashed pair

Which is significantly faster. And since this in a "hot path" of the code (the loop where the lexer
does work the most often), performance gains add up very quickly, causing a large boost in
performance.

In Strager's video, he continues to beat gperf through various other techniques. However, since I
only need a static map, I'm happy with the system that I have now. It may be worth looking into
other techniques that he uses later.

One huge advantage of gperf (as opposed to manually writing a perfect hash function, for example)
is that it's a generator tool, so adding new keywords to the lexer just consists of describing them
in the gperf input (keywords.gperf) and regenerating the source. Hand writing and testing a perfect
hash function like in Strager's video is doable for a language like JavaScript, where the
standard-defined keyword set is unlikely to change tomorrow. Tachyon, however, is still very much a
work in progress, so it helps a lot that modifying keywords is a fast part of my development
workflow.

## 2. Reserving capacity for token and constant lists

Another thing that I thought of was that we must be spending so much time on vector reallocations
for token and constant lists. When a vector is initialized with `std::vector<T> v;`, it starts
with a capacity of 0. It then scales geometrically when values are inserted, usually growing by
1.5-2x. As an example, let's assume a growth factor of 2x[^3], and chart the sizes of the vector as
the lexer continues lexing tokens.

[^3]: implementation defined.

| Step                | Capacity |
|---------------------|----------|
| Initial             | 0        |
| Push first token... | 4        |
| Push fifth token... | 8        |
| Push ninth token... | 16       |
| Push 17th token...  | 32       |
| Push 33rd token...  | 64       |
| Push 65th token...  | 128      |
| Push 129th token... | 256      |
| Push 257th token... | 512      |
| Push 513th token... | 1024     |

Each row is a reallocation. Reallocations are expensive because they consist of:

* a `malloc` call to allocate new heap data at the larger capacity
* a `memcpy` call or a move/copy constructor to move existing data to the new address

Therefore, a significant performance boost can be found by reserving some size during the vector's
initialization, avoiding reallocations (and if there *are* reallocations, they will be less
frequent, since the capacity is already large and the vector scales geometrically). This comes with
the cost of a larger memory footprint, especially for smaller source codes that will never reach
the pre-allocated limit. Since Tachyon aims to be performant over memory efficient, I'll accept
this cost. If I was working with embedded or other memory-limited devices, this decision may not
have been so straightforward.

From some manual benchmarking, I found a good capacity of around 20000 tokens and 10000 constants.

Now, my performance looks like this, which is over a 3.4x improvement for `Lex20000Chars` just with
these last two optimizations:

```shell
/home/areg/projects/tachyon/cmake-build-release-benchmarks-only/libs/lexer/lexer_benchmarks
2025-07-14T09:53:22-04:00
Running /home/areg/projects/tachyon/cmake-build-release-benchmarks-only/libs/lexer/lexer_benchmarks
Run on (8 X 400.007 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x4)
  L1 Instruction 32 KiB (x4)
  L2 Unified 256 KiB (x4)
  L3 Unified 6144 KiB (x1)
Load Average: 2.51, 2.42, 2.13
-------------------------------------------------------------------------
Benchmark                               Time             CPU   Iterations
-------------------------------------------------------------------------
LexerDataFixture/LexBlank            1303 ns         1251 ns       459511
LexerDataFixture/LexBasic            6082 ns         5859 ns       100000
LexerDataFixture/Lex1000Chars        6900 ns         6858 ns       101324
LexerDataFixture/Lex2000Chars       12601 ns        12530 ns        52799
LexerDataFixture/Lex5000Chars       53187 ns        52913 ns        13641
LexerDataFixture/Lex10000Chars     126448 ns       125637 ns         5658
LexerDataFixture/Lex20000Chars     226623 ns       225215 ns         2963
LexerDataFixture/LexBigComment      14150 ns        14020 ns        50649

Process finished with exit code 0
```

## 3. Replacing `std::stod`

These two optimizations are the only things that came to mind off the bat. From now on, I'll use
`gbench` and `perf` to get a better understanding of the lexer and where I can gain performance.

```shell
perf record -g ./lexer_benchmarks --benchmark_filter=LexerDataFixture/Lex20000Chars
perf report


Samples: 3K of event 'cycles:Pu', Event count (approx.): 3299930785
  Children      Self  Command          Shared Object         Symbol
+   55.46%    54.33%  lexer_benchmark  lexer_benchmarks      [.] tachyon::lexer::Lexer::lex(std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> > const&)                                                    ◆
+   12.83%    12.83%  lexer_benchmark  libc.so.6             [.] __GI_____strtod_l_internal                                                                                                                                                  ▒
+    6.51%     0.00%  lexer_benchmark  [unknown]             [.] 0x005555555fa66000                                                                                                                                                          ▒
+    5.32%     3.12%  lexer_benchmark  lexer_benchmarks      [.] LexerDataFixture_Lex20000Chars_Benchmark::BenchmarkCase(benchmark::State&)                                                                                                  ▒
+    4.53%     4.18%  lexer_benchmark  libc.so.6             [.] round_and_return                                                                                                                                                            ▒
+    4.30%     3.15%  lexer_benchmark  libc++.so.1.0         [.] std::__1::stod(std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> > const&, unsigned long*)                                                ▒
+    3.35%     3.35%  lexer_benchmark  libc.so.6             [.] __strlen_avx2                                                                                                                                                               ▒
+    3.16%     2.58%  lexer_benchmark  libc.so.6             [.] __memmove_avx_unaligned_erms                                                                                                                                                ▒
+    3.13%     3.11%  lexer_benchmark  libc.so.6             [.] str_to_mpn.part.0.constprop.0                                                                                                                                               ▒
+    3.08%     0.00%  lexer_benchmark  [unknown]             [.] 0xaa8000736e6f0073                                                                                                                                                          ▒
+    2.89%     1.85%  lexer_benchmark  libc.so.6             [.] __ctype_b_loc                                                                                                                                                               ▒
+    2.81%     2.16%  lexer_benchmark  lexer_benchmarks      [.] decltype(auto) std::__1::__variant_detail::__visitation::__base::__dispatcher<2ul>::__dispatch[abi:ne190107]<std::__1::__variant_detail::__dtor<std::__1::__variant_detail::▒
+    2.56%     1.04%  lexer_benchmark  lexer_benchmarks      [.] __ctype_b_loc@plt                                                                                                                                                           ▒
+    1.79%     1.45%  lexer_benchmark  lexer_benchmarks      [.] decltype(auto) std::__1::__variant_detail::__visitation::__base::__dispatcher<1ul>::__dispatch[abi:ne190107]<std::__1::__variant_detail::__dtor<std::__1::__variant_detail::▒
+    1.75%     1.46%  lexer_benchmark  libc.so.6             [.] __mpn_construct_double                                                                                                                                                      ▒
+    1.11%     1.11%  lexer_benchmark  libc.so.6             [.] __mpn_lshift                                                                                                                                                                ▒
     1.01%     0.58%  lexer_benchmark  lexer_benchmarks      [.] memmove@plt                                                                                                                                                                 ▒
     1.01%     0.88%  lexer_benchmark  libc.so.6             [.] round_away                                                                                                                                                                  ▒
+    0.66%     0.00%  lexer_benchmark  [unknown]             [.] 0x8000736e6f007300                                                                                                                                                          ▒
     0.64%     0.54%  lexer_benchmark  libc.so.6             [.] __strncmp_avx2                                                                                                                                                              ▒
     0.62%     0.44%  lexer_benchmark  libc++.so.1.0         [.] std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> >::~basic_string()                                                                      ▒
+    0.57%     0.00%  lexer_benchmark  [unknown]             [.] 0xaa8000736e6f0000                                                                                                                                                          ▒
     0.55%     0.34%  lexer_benchmark  libc.so.6             [.] __errno_location                                                                                                                                                            ▒
     0.53%     0.34%  lexer_benchmark  lexer_benchmarks      [.] std::__1::stod(std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> > const&, unsigned long*)@plt                                            ▒
     0.48%     0.19%  lexer_benchmark  libc++.so.1.0         [.] std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> >::~basic_string()@plt                                                                  ▒
```

Here, I notice that we spend a lot of time in `std::stod` (to lex doubles). This is valuable
information. Stubbing the `stod` call with `0.` gives over a 1.5x performance improvement. Some
research into `stod` shows that it does a lot, like locale handling, compliance, error checking,
etc. Since Tachyon is a relatively small language, I don't need a lot of those features (especially
the locales), so I can look into finding a replacement.

I found a few key options:
1. [fast_float](https://github.com/fastfloat/fast_float)::from_chars (header-only external library)
2. std::from_chars (C++17 standard)
3. strtod (C-style)

Of these options, I found fast_float to be the best mix of accuracy and performance. The difference
in code is:

```diff
- constants.emplace_back(std::stod(std::string(start, current - start)));

+ double value;
+ auto answer = fast_float::from_chars(start, current, value);
+ if (answer.ec != std::errc())
+     errors.emplace_back(
+         Error::create(ErrorKind::LexError, SourceSpan(0, 0), "failed to read number"));
+ constants.emplace_back(value);
tokens.emplace_back(NUMBER, std::string_view(start, current), constants.size() - 1);
```

Which results in:

```shell
/home/areg/projects/tachyon/cmake-build-release-benchmarks-only/libs/lexer/lexer_benchmarks
2025-07-14T10:00:54-04:00
Running /home/areg/projects/tachyon/cmake-build-release-benchmarks-only/libs/lexer/lexer_benchmarks
Run on (8 X 2900.47 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x4)
  L1 Instruction 32 KiB (x4)
  L2 Unified 256 KiB (x4)
  L3 Unified 6144 KiB (x1)
Load Average: 2.85, 4.96, 3.81
-------------------------------------------------------------------------
Benchmark                               Time             CPU   Iterations
-------------------------------------------------------------------------
LexerDataFixture/LexBlank             101 ns          101 ns      6235338
LexerDataFixture/LexBasic             281 ns          281 ns      2441141
LexerDataFixture/Lex1000Chars        6004 ns         5981 ns       104296
LexerDataFixture/Lex2000Chars       10140 ns        10105 ns        69330
LexerDataFixture/Lex5000Chars       41537 ns        41318 ns        19305
LexerDataFixture/Lex10000Chars      76672 ns        76398 ns         8880
LexerDataFixture/Lex20000Chars     150769 ns       150184 ns         4643
LexerDataFixture/LexBigComment      13869 ns        13829 ns        51221

Process finished with exit code 0
```

Which is a 56% improvement. Another optimization that I'd look into in the future is having it
parse strings without a `.` as an integer. I'll leave that for now, though, because I may or may
not decide to have native integer types in the virtual machine.

## 4. `isalpha`

Let's see another section of the perf results (I'm now using `--call-graph=dwarf` for a fuller call
graph).

```shell
      - 1.56% tachyon::lexer::is_identifier_char(char const*) (inlined)                                                                                                                                                                      ▒
           isalpha (inlined)                                                                                                                                                                                                                 ▒
           __ctype_b_loc@plt                                                                                                                                                                                                                 ▒
```

`__ctype_b_loc@plt` is a hint that the program is reading the locale. Again, I don't want to
support locales for Tachyon, so I shouldn't need all of these details in `is_identifier_char`. So
let's try to move towards using something like this:

```diff
inline bool is_identifier_char(const char *ch) {
-    return isalpha(*ch) || *ch == '_' || *ch == '\'';
+    return (*ch >= 'A' && *ch <= 'Z') || (*ch >= 'a' && *ch <= 'z') || *ch == '_' || *ch == '\'';
}
```

And I did the same for `is_starting_identifier_char`, `is_starting_number_char` and
`is_number_char`. This gives us:

```shell
-    0.00%  lexer_benchmark  lexer_benchmarks  [.] tachyon::lexer::is_identifier_char(char const*) (inlined)                                                                                                                                 ◆
```

Which is a nice improvement from 1.56%. Something to look into in the future is using a lookup
table for reading all of these characters instead.

## Conclusion

I analyzed my existing codebase to see where my initial implementation could benefit in
optimizations. I used tools like the standard `std::vector::reserve`, GNU's `gperf`, and the Linux
`perf` tool to optimize and better understand my program. In the end, I ended up with a 619% boost
in performance, and the program is now able to lex 20000 characters in just 0.135 milliseconds.

```shell
/home/areg/projects/tachyon/cmake-build-release-benchmarks-only/libs/lexer/lexer_benchmarks
2025-07-14T12:09:32-04:00
Running /home/areg/projects/tachyon/cmake-build-release-benchmarks-only/libs/lexer/lexer_benchmarks
Run on (8 X 3100.2 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x4)
  L1 Instruction 32 KiB (x4)
  L2 Unified 256 KiB (x4)
  L3 Unified 6144 KiB (x1)
Load Average: 1.72, 1.55, 1.49
-------------------------------------------------------------------------
Benchmark                               Time             CPU   Iterations
-------------------------------------------------------------------------
LexerDataFixture/LexBlank             113 ns          112 ns      6341061
LexerDataFixture/LexBasic             287 ns          286 ns      2402244
LexerDataFixture/Lex1000Chars        3662 ns         3650 ns       172662
LexerDataFixture/Lex2000Chars        6921 ns         6899 ns       102302
LexerDataFixture/Lex5000Chars       30090 ns        29996 ns        23380
LexerDataFixture/Lex10000Chars      71156 ns        70868 ns         9913
LexerDataFixture/Lex20000Chars     134971 ns       134296 ns         5357
LexerDataFixture/LexBigComment      17539 ns        17469 ns        39073

Process finished with exit code 0
```

Some next steps for improving the performance of the lexer are:

* Parse integers separately
* Constant lookup table for `is_identifier_char` and friends
* Look into other kinds of list storage structures or memory allocators
