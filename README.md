# C++ (Cyclic) Redundancy Checks

[ <<'```sh' ]: \]

<details>
<summary>
This is a self-executing documentation. It writes and runs example.cpp
when executed.
</summary>

```sh
# Extract C++ block at end
sed <"$0" -E -e '0,/^```cpp$/d' -e '/^```$/,$d' >example.cpp
# Compile it
c++ example.cpp -g3 -Wall -Wextra -std=c++20 -o example.elf
# And run it with the CRC RevEng check value
echo -n 123456789 | ./example.elf
exit
```
</details>

Implements CRC.

Which one?

Yes.

(It handles all the [RevEng
Catalogue](https://reveng.sourceforge.io/crc-catalogue/) for widths 8
to 64 inclusive. I decided to stop there because I didn't need >64 bits,
and my maths will be thrown off for <8 bits)

## Acknowledgements

With thanks to [Wikipedia for their article on computing
it](https://en.wikipedia.org/w/index.php?title=Computation_of_cyclic_redundancy_checks&oldid=1334498958)
and [CRC RevEng](https://reveng.sourceforge.io/crc-catalogue/) for their
catalogue and handy software to test against

## Tests

There are a few different kinds of tests:
- C++ static_assert in [./test/test.hpp](/test/test.hpp);
- Python cosimulation in [./test/test.py](/test/test.py);
- Using parameters from [CRC RevEng](https://reveng.sourceforge.io/)
  and checking against their output in
  [./test/mkreveng.sh](/test/mkreveng.sh).

## Use

```cpp
#include "cpprc/crc.hpp"

#include <array>
#include <cstdint>
#include <iomanip>
#include <ios>
#include <iostream>

using namespace std::literals; // For ""sv

// You don't actually need to overload operator>> to use this. This is
// just one way to do it.
template<class CharT, class Traits>
Crc::Pkzip & operator>>(std::basic_istream<CharT, Traits> & in, Crc::Pkzip & sum)
{
    std::array<uint8_t, 256> buf;
    while (in.good())
    {
        in.read(reinterpret_cast<CharT *>(buf.data()), buf.size());
        // You do need some range (e.g. span works).
        sum.tabled(std::span{buf}.first(in.gcount()));
    }
    return sum;
}

int main()
{
    Crc::Pkzip sum;
    std::cin >> sum;
    // And the Crc implementation casts to an integer for
    // printing/comparison.
    std::cout << "0x" << std::hex << std::uppercase << std::setw(8)
              << sum << "\n";
    // For varargs you do need the explicit cast otherwise it may
    // do bit-cast
    printf("0x%08X\n", static_cast<uint32_t>(sum));
}
```
