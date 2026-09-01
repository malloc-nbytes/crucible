extern proc printf(fmt: u8*, ...): void;
extern proc free(ptr: u8*): void;
extern proc load_file(path: u8*): u8*;

export proc main(): i32
{
        let src: u8* = load_file("build.sh");

        printf("%s", src);

        return 0;
}
