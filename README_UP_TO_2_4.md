# DA Project 2 - implemented up to T2.4

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Batch mode required by the statement

```bash
./build/projeto2 -b input/ranges/ranges1.txt input/registers/registers2.txt output/allocation.txt
```

## Interactive menu

```bash
./build/projeto2
```

## Implemented algorithms

- `basic`: builds webs, builds the interference graph, and tries graph coloring from 1 up to the configured register limit.
- `spilling, K`: tries `basic` first, then spills up to `K` webs to memory, trying fewer spills before more spills.
- `splitting, K`: tries `basic` first, then splits up to `K` webs, rebuilding the interference graph after each split.
- `free`: custom DSATUR-based allocator. It tries exact coloring up to the register limit, then automatically spills the fewest webs it can find.

## Notes for the demo

The project still uses the provided `Graph<T>` as the primary interference-graph representation. Extra helper structures are only used to rank candidates, store colors, and run searches.
