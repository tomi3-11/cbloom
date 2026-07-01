import pytest

from cbloom import BloomFilter

# construction 

def test_create_valid():
    bf = BloomFilter(1000, 0.01)
    assert bf is not None


def test_create_zero_capacity():
    with pytest.raises(ValueError):
        BloomFilter(0, 0.01)


def test_create_zero_error_rate():
    with pytest.raises(ValueError):
        BloomFilter(1000, 0.0)


def test_create_error_rate_above_one():
    with pytest.raises(ValueError):
        BloomFilter(1000, 1.0)


# internal dimensions 

def test_bit_count_positive():
    bf = BloomFilter(1000, 0.01)
    assert bf.bit_count() > 0


def test_hash_count_positive():
    bf = BloomFilter(1000, 0.01)
    assert bf.hash_count() > 0


def test_item_count_starts_zero():
    bf = BloomFilter(1000, 0.01)
    assert bf.item_count() == 0


def test_larger_capacity_more_bits():
    bf_small = BloomFilter(1000, 0.01)
    bf_large = BloomFilter(1000000, 0.01)
    assert bf_large.bit_count() > bf_small.bit_count()


def test_lower_error_rate_more_bits():
    bf_loose = BloomFilter(1000, 0.1)
    bf_tight = BloomFilter(1000, 0.001)
    assert bf_tight.bit_count() > bf_loose.bit_count()


# add and contains

def test_add_and_contains():
    bf = BloomFilter(1000, 0.01)
    bf.add("hello")
    assert bf.contains("hello") is True


def test_definitely_absent():
    bf = BloomFilter(1000, 0.01)
    assert bf.contains("never_added") is False


def test_item_count_increments():
    bf = BloomFilter(1000, 0.01)
    bf.add("a")
    bf.add("b")
    bf.add("c")
    assert bf.item_count() == 3


def test_multiple_items():
    bf = BloomFilter(1000, 0.01)
    items = ["apple", "banana", "cherry", "date", "elderberry"]
    for item in items:
        bf.add(item)
    for item in items:
        assert bf.contains(item) is True


def test_empty_string():
    bf = BloomFilter(1000, 0.01)
    bf.add("")
    assert bf.contains("") is True


def test_unicode_string():
    bf = BloomFilter(1000, 0.01)
    bf.add("héllo")
    assert bf.contains("héllo") is True


def test_long_string():
    bf = BloomFilter(1000, 0.01)
    key = "x" * 10000
    bf.add(key)
    assert bf.contains(key) is True


def test_duplicate_adds():
    bf = BloomFilter(1000, 0.01)
    bf.add("hello")
    bf.add("hello")
    bf.add("hello")
    assert bf.contains("hello") is True
    assert bf.item_count() == 3   # counts raw insertions, not unique


# false positive rate
def test_false_positive_rate_within_bound():
    """
    Insert 10000 known items, then check 100000 unknown ones.
    Larger sample means variance shrinks and 2x headroom is reliable.
    """
    capacity = 10000
    error_rate = 0.01
    bf = BloomFilter(capacity, error_rate)

    for i in range(capacity):
        bf.add(f"known:{i}")

    false_positives = 0
    checks = 100000
    for i in range(checks):
        if bf.contains(f"unknown:{i}"):
            false_positives += 1

    actual_rate = false_positives / checks
    assert actual_rate < error_rate * 2, (
        f"False positive rate {actual_rate:.4f} exceeded 2x bound "
        f"of {error_rate * 2:.4f}"
    )
