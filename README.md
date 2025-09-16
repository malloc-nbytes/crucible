# crucible

A compiled programming that I am making on stream.

Visit at: https://www.twitch.tv/malloc_nbytes

# Requirements

I am currently using a library for basic data structures/string functionality
and this will be removed once this compiler is bootstrapped. It requires `forge` [https://github.com/malloc-nbytes/forge].

# Compilation

There are currently two options to build:

1. using `earl` (requires the `earl` language [https://github.com/malloc-nbytes/earl]):
```
earl build.rl
earl build.rl -- debug # build in debug mode
```

2. using `BASH`
```
/bin/sh build.sh
```

3. using `forge` (requires the `forge` package manager [https://github.com/malloc-nbytes/forge] and it's associated repo)
```
sudo forge install malloc-nbytes@crucible
```