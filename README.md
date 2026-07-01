# cbloom

A fast Bloom filter for Python, backed by a C extension.

Answers one question efficiently: **"Have I seen this before?"**

- No false negatives — if it says `False`, the item is definitely absent
- Tunable false positives — configure your own error rate (e.g. `0.01` = 1%)
- Low memory — 1 million items at 1% error rate uses ~1.1 MB of bits

## Installation

pip install cbloom

## Usage

```python
from cbloom import BloomFilter

# 1 million expected items, 1% false positive rate
bf = BloomFilter(capacity=1_000_000, error_rate=0.01)

bf.add("hello")
bf.add("world")

bf.contains("hello")   # True
bf.contains("world")   # True
bf.contains("python")  # False (definitely not added)
```

## API

`BloomFilter(capacity, error_rate)`
- `capacity` — maximum number of items you expect to insert
- `error_rate` — acceptable false positive rate, e.g. `0.01` for 1%

| Method | Returns | Description |
|--------|---------|-------------|
|`.add(key)`|`None`|Add a string to the filter |
|`.contains(key)`|`bool`|`True` if probably present, `False` if definitely absent |
|`.item_count()` | `int` | Number of items inserted |
|`.bit_count()` | `int` | Size of the internal bit array (m) |
| `.hash_count()` | `int` | Number of hash functions in use (k) |


## How it works

Internally the filter allocates a bit array of size `m` (computed from your
`capacity` and `error_rate`). When you call `.add(key)`, two hash functions
(FNV-1a and DJB2) produce `k` positions in the array and set those bits to 1.
`.contains(key)` checks all `k` positions -- if any bit is 0 the item was
definitely never added.

The tradeoff: once enough bits are set, unrelated keys can accidentally match
all `k` positions, producing a false positive. The math guarantees this stays
below your configured `error_rate` as long as you stay within `capacity`.

## Use cases

- Deduplicating URLs in a web crawler
- Skipping expensive database lookups
- Checking passwords against breach lists
- Filtering seen events in a streaming pipeline

## License

MIT
