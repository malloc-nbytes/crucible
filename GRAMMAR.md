__Note__:
- `[]` means optional
- `<>` means required
- plain lowercase text are required characters
- plain UPPERCASE text are required grammar (STMT|EXPR)
- `[...]` means optional continuation of what was written previously
- `...VARIADIC` means variadic argument(s). The language syntax to declare a function as variadic is `...`

# Module (names the current file's module)

```
module <id> where
```

# Procedures

```
[export] proc <id>([<id>: <type>[,...][, ...VARIADIC]): <type>
{
}
```

# Externs

## Procedures
```
extern proc <id>([[id:] <type>[,...][,...VARIADIC]): <type>;
```

# Return

```
return [EXPR];
```

# Variable Declarations

```
let <id>: <type> = EXPR;
```

# Pointers

```
<type>*
```

# Conditionals

## If

```
if EXPR STMT
[else STMT]
```

## Structs

```
struct <id> {
       item1: <type>,
       item2: <type>,
       [...]
}
```

# Enums

```
enum <id> {
     item1 [= CONST_EXPR],
     item2 [= CONST_EXPR],
     [...]
}
```
