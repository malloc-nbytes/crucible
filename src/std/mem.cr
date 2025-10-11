module mem where

export proc set(src: u8*, val: u8, n: size_t): void
{
        for (let i: size_t = 0; i < n; i += 1) {
                src[i] = val;
        }
}
