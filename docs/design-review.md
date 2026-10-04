# Design review: learning C++ through chess

## Assessment

The array board, value-based moves, CMake library/CLI split, and incremental
correctness-first approach are a good foundation. Keep them. The initial review identified undo as the urgent repair. It is now tested and
starting Perft depths 0–3 pass; attack detection and legal generation are next.

This page records the initial design review; implementation progress is tracked
in [roadmap.md](roadmap.md) and [bootstrap-review.md](bootstrap-review.md).

## Improvements adopted in the design guidance

| Previous gap | Updated decision | Why it helps learning |
| --- | --- | --- |
| A statement called side-to-move check illegal | Distinguish valid check from a move leaving its own king attacked | Teaches check evasions correctly |
| Two architecture diagrams looked like sequential execution pipelines | Treat Perft and search as separate consumers of board/move APIs | Clarifies dependency direction and module responsibilities |
| Large duplicate roadmaps obscured current status | Keep durable rules in AGENTS and status/checklists in one roadmap | Makes the next task and evidence of completion visible |
| Undo guidance did not specify saved original pieces/capture squares | Define exact saved-state contracts and full-state comparisons | Connects value semantics to information preservation |
| Parsing failure could mutate the board | Parse into a temporary and commit on success | Gives a concrete transaction and failure-guarantee exercise |
| `assert` tests could disappear in Release | Plan checks active under `NDEBUG`, diagnostics, and sanitizers | Teaches build-mode behavior and observable failures |
| Default move ambiguously meant no move | Plan explicit optional move and search-result metadata | Teaches modeling absence rather than guessing from values |
| Search sketches omitted terminal leaves | Require mate/stalemate detection before static evaluation | Prevents false scores at the horizon |
| Capture-only quiescence guidance lacked check handling | Search evasions in check and prohibit stand-pat there | Connects tactical search to legal chess rules |
| Draw history and hashing semantics were missing | Separate position/history, repetition identity, and TT policy | Prevents assuming all equal boards have equal search context |
| C++ concepts were a broad list | Attach concepts and short explanations to each milestone | Creates exercises with a natural reason to use each feature |
| `std::expected` was suggested without a standard constraint | Probe compiler support; use C++23 expected for parsing errors | Keeps examples compatible with the actual build |

## Implementation tradeoffs to retain

A 64-square array is easy to inspect, serialize, and compare. Copying a board is
also useful in tests as an independent snapshot; a make/undo core need not avoid
all copies at the expense of clarity. Save only the reversible information needed
for each move in production traversal, with tests comparing the complete board.

Start with a square scan to locate kings. Cache king squares only if profiling
justifies the extra invariant, then include the cache in undo and validation.
Keep attack detection independent of generated legal moves to avoid recursion
and to handle pinned-piece attacks correctly.

Retain a narrow trusted internal make API and validate user/GUI moves at the
boundary. Document its preconditions. Avoid making every recursive node parse or
revalidate external input, while keeping unsafe coordinates out of board access.

Bitboards are a useful later systems exercise, not a prerequisite for connecting
a GUI. Responsive UCI may need one worker and cancellation; that does not require
parallel tree search. Keep both experiments independent so their complexity is
introduced for a clear purpose.

## A practical learning rhythm

Choose one unchecked task. Draw a small chess position, predict the result,
write a failing test, implement the smallest readable change, then explain why
it works. Inspect the diff and check state restoration before moving on.
For later performance work, record a baseline and compare identical positions
and limits. Count nodes separately from elapsed time to distinguish search-tree
improvements from faster execution.

## References

The check, attack, and castling guidance follows
[FIDE Laws of Chess, Articles 3.1.3, 3.8, and 3.9](https://handbook.fide.com/chapter/E012023).
The ownership and lifetime guidance follows the
[C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines),
especially Rule of Zero and RAII. These references describe the target design;
current engine limitations remain documented in [architecture.md](architecture.md).
