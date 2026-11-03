# NEG - Negotiation game

A standalone foundation for a future minigame in the Saxon War campaign of
"DieFranken". The game core is a C++17 library; `NegInit` provides a command-line
interface for trying it out. No external libraries or runtime network access
are required. Program output, error messages, code comments, and documentation
are in English.

## Rules

At startup, distinct available goods are selected at random. Each position in the
secret sequence is filled independently and uniformly with one of these goods.
Repeated goods are allowed, and not every available good has to appear in the
sequence. The secret stays hidden during the game.

| Code | Good |
|---|---|
| W | Wood |
| F | Food |
| P | Pole |
| T | Timber |
| B | Board |
| S | Straw |

| Difficulty | Available goods | Positions | Maximum attempts |
|---|---:|---:|---:|
| 1 | 3 | 3 | 3 |
| 2 | 3 | 4 | 3 |
| 3 | 4 | 4 | 4 |
| 4 | 4 | 5 | 4 |
| 5 | 5 | 5 | 5 |
| 6 | 5 | 6 | 5 |
| 7 | 6 | 6 | 6 |

Enter exactly one available goods letter per position for each attempt. Uppercase
and lowercase are accepted, and leading and trailing whitespace is removed.
Internal whitespace, incorrect length, and unavailable goods are errors. The CLI
exits immediately on invalid input; the core does not consume an attempt.

`X` means the correct good at that exact position; `-` means an incorrect good.
There are no hints about other positions, and correct positions are not locked.
After each valid attempt, feedback and the remaining attempt count are displayed.
A completely correct guess wins, including on the final attempt. Otherwise, using
all attempts ends the game in defeat and reveals the secret. The attempt limit
matches the number of available goods, allowing systematic elimination. No solver
or automatic assistance is included.

## Build and test

Requirements: CMake 3.16 or later and a C++17 compiler.

On Windows with Visual Studio 2022 and the Desktop development with C++ workload,
run these commands from the repository directory:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
.\build\Release\NegInit.exe 1
```

On Linux with GCC:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
./build/NegInit 1
```

With CMake 3.16, run CTest directly from the build directory instead:
`cd build`, then `ctest -C Release --output-on-failure`.

## Example

Illustrative session with available goods `WFS` and secret sequence `WFF`:

```text
Available goods: WFS
W=Wood F=Food P=Pole T=Timber B=Board S=Straw
Positions: 3
Maximum attempts: 3
Input: SFW
Feedback: -X- | Remaining attempts: 2
Input: WFS
Feedback: XX- | Remaining attempts: 1
Input: WFF
Feedback: XXX | Remaining attempts: 0
You won!
```

Normal startup generates the available goods and secret at random. The session
above is covered by a fixed setup in the core tests. A reproducible CLI error
case in PowerShell is `"???" | .\build\Release\NegInit.exe 1`: startup information
is followed by an error message, and `$LASTEXITCODE` is 2. On Linux, use
`printf '???\n' | ./build/NegInit 1`.

| Exit code | Meaning |
|---|---|
| 0 | Win |
| 1 | Defeat |
| 2 | Invocation/input error, premature end of input, or initialization error |

Exactly one startup argument is accepted: a single digit from `1` to `7`.
Other forms such as `01` or `+1` are rejected. Invocation errors also display a
usage hint. EOF while a game is still running is an error.

## Structure and API

- `include/neg/game.hpp`, `src/game.cpp`: configuration, random generation,
  validation, evaluation, attempt counts, and status without console I/O.
- `src/main.cpp`: argument validation, input, output, and process exit codes.
- `tests/`: tests without an external test library, integrated with CTest.

`neg::Game(difficulty)` uses `std::mt19937` seeded by `std::random_device`.
`neg::Game(difficulty, seed)` provides reproducible initialization without an
additional CLI argument. The exact sequence may differ between standard library
implementations because shuffle and distribution implementations can differ.

`neg::Game(difficulty, availableGoods, secret)` supports fixed evaluation cases
and validates the goods count, distinct predefined goods codes, and the length
and contents of the secret. This constructor expects uppercase letters. Invalid
constructor arguments throw `std::invalid_argument`. `submit()` returns whether
an attempt was accepted, feedback, and an error message. Rejected input does not
change state. All attempts after the game has ended are rejected. Getters expose
the status and attempt counts. Read-only access through `secret()` supports tests
and revealing the secret after defeat; a future interface is responsible for
hiding it during play. An Unreal interface can use the library without changing
the game logic.

## Verification

The project was built in Release mode on Windows with Visual Studio 2022/MSVC
and verified with CTest. Core tests cover all configurations, generation
invariants for fixed seeds, reproducible initialization, specific feedback
cases, validation, and final states. CLI tests cover invalid arguments, invalid
input, and EOF. Linux/GCC was not tested in this environment.

Unreal integration, a graphical interface, goods consumption, saved games, a
database, and networking are outside the scope of this foundation.
