# Code Standard
  - This document is created as a reminder on what code standard is used throughout the project. The code standard has been changed a few times before it was set to this one.

Why have I written all this when I am the only to use this repo!?

## Indentation
Tabs are used. The default tab spacing is equal to 4 spaces.

Tabs to align multiple definitions/declarations are encouraged.

```
Prefer:                 Ok but not encouraged:
int a       = 1;        int a = 1;
int foobar  = 2;        int foobar = 2;
int bar     = 3;        int bar = 3;
```

## Variables
  - For normal scoped variables use camelCase `foo = 0`.
  - For global variables use g_PascalCase `g_Bar = 1`.
  - For static variables use s_PascalCase `s_Baz = 2`.
  - For private variable use m_PascalCase `m_Qux = 3`.
  - For pointer variables, use same amount of p as there are pointers. `Test* m_pQuux = nullptr`, `Test** m_ppCorge = nullptr`.
  - For parameters use same as scoped variables. If pointers add the aforementioned p:s.

## Classes
```
class FooBar
{
public:
    struct Baz
    {
        int Qux = 4;
    };

public:
    FooBar() = default;
    
public:
    int quux = 0;
    
(if proteced: same order put here)
(if private: same order put here)
}

```

## Functions
  - Use PascalCase for functions `void Function() = 0;`.
  - If function is an override, type that `void Function() override;`.
  - If function is final override, type that `void Function() override final`.
  - Use same parameters named in definition and declaration.

## If-statements
  - Prefer spaces after keywords if, else if, and else.
  - If statement is relatively short then curly brackets can start on same line.
  - If statment is longer (subjectively) then have new line.
  - If statement is only one row, brackets can be ignored

## For
Follows same rules as if-statements.

## While
Follows same rules as if-statements.
