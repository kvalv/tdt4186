**TLDR**: `make all-tests` for testing the code.

### Some words about the code
The could could be made a bit simpler if we didn't spend so much time
on making tests and 'printing' out the heap after a run. However, this
extra check made it quite easy for us to validate the behaviour and find
bugs.

When calling `myfree` we zero out the data. When calling `mymalloc` we
fill the data with the value `1` in each byte. This makes it easy to
identify which blocks are not used and which ones are potentially used.
This is strictly speaking not a part of the exercise, but it makes it
a bit easier to debug the code.

We also add a magic variable `0xaaaabbbbccccdd00 | g_block_id` as the
`mem_control_block->next` pointer for the occupied blocks, since they
are not used at all. This is very hacky, but again pretty useful to
identify which blocks are what. The `g_block_id` is incremented
after each allocation. One potential bug is if the user (who called
`mymalloc`) writes this exact string somewhere in the allocated memory,
in which case we'll identify it as an occupied `mem_control_block`... :-)
(though that should only have an impact on the visualization of the
heap, not the actual behaviour).

### About our tests
We have a bunch of tests files located in the `tests/` directory. We
figured this was a cleaner approach than populating the `main` function
with a bunch of test cases. Also, having the tests in separate files
makes it easier to reason about them, and provide textual description
on what we're testing.

The syntax inside of the files is as follows:
```
line 1: The memory size
line 2: the sequence of actions.
line 3: the expected output
line 4+: ignored. Useful for adding comments / explanation.
```

For `the sequence of actions`, each action is separated by whitespace, and each
action is one of:

* `M<int>`: malloc `<int>` number of bytes
* `F[a-z]`: Free memory block identified by the single character. The first malloced
memory block will have id `'a'`, the next one will have id `'b'` and so on. We assume
that we'll have 26 mallocs or less for the tests.
* **Example**: `M16 M16 fa M8 ` would mean `malloc 16 bytes; malloc 16 bytes; free the first malloc; malloc 8 bytes.`

For `the expected output`, this will be a string which is of length `memory_size / 8`. Each character
in the string consist of 8 bytes.

* `Z`: 8 bytes of data is unused ('zero')
* `D`: 8 bytes of data that has been malloc'ed (i.e has been reserved and is available for use by whoever malloc'ed).
* `FF`: These 16 bytes is a `mem_control_block` which is **free** (not malloced).
* `Ba`: These 16 bytes is a `mem_control_block` which is **not free** (has been malloced). In addition, this
one has identifier `a`. The second allocated block has identifier `b` and so on.
* **Example**: `FFZZZZBaDDDD` would mean `free block at start; 32 consecutive free bytes; allocated block a, together with 32 consecutive bytes that can be used by whoever malloc'ed block a`.


### Running the tests
To run the single test `tests/1`:
```
./run-tests tests/1
```

To run all tests:
```
./run-tests tests/*
```

The `run-tests` script has the `-e` flag set, so it should fail upon the
first error, presumably if a test fails.
