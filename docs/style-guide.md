# Style Guide

This is a guide as to how changes on the *main line* ought to be formatted,
it is expected that changes pushed to main follow this guide.

## Contents
- [C code](#c-code)
    - [Function names](#function-names)
    - [Type names](#type-names)
    - [Asserting invariants](#asserting-invariants)
- [Documentation](#documentation)
    - [Doc-comments](#doc-comments)
    - [Markdown](#markdown)
- [Commit Style](#commit-style)

## C code

In any cases where the best practice is not made explictly clear below, use
best judgement and precedent, bearing in mind that consistency is the guiding
principle of good style.

### Function names

Generally, each source file/module has a corresponding function prefix which
should be defined at the top of the related header file in a
[doc-comment](#doc-comment).

This prefix should be used for all *public* functions in a module (i.e. part
of the API).

```c
extern key_t mod_generate_index();
```

### Type names

No `typedef` (other than for *really* well-documented, *really* important
stuff).

### Asserting invariants

Use C-native `assert()` to ensure invariant conditions are met at the start of
functions; do not leave the success of a function call to chance.

## Documentation

### Doc-comments

Documentation comments are indicated using a multi-line comment commencing
with `/**` used to describe and outline important features in header files.
Use the [Doxygen Standard](https://www.doxygen.nl/manual/docblocks.html) with *Javadoc*-style comment blocks

Doc-comments should *immediately precede* the component they describe, this applies for:

- Functions signatures
- Global variables, 
- Structs and typedefs

And should try to be exhaustive.

### Markdown

Keep margins to an 80 character limit.

Write instructive and concise documentation with clearly-labelled headings and
subheadings.

Do not rely heavily on features of markdown that are flavour-specific (e.g.
GitHub callouts), try to write universally-accessible markdown.

## Commit Style

Commits on the main line should be formatted with
[conventional commits](conventional-commits.org/) with a subject line up to 50
characters and lines in the body up to 72 characters.

### Commit Types

We use a slightly constrained set of commit types:

- `bugfix`
- `ci`
- `cmds`
- `compat`
- `docs`
- `ds`
- `memfix`
- `refact`

Changes should be small enough to be captured by a single type.

### Commit Scopes

As set out in [conventional commits](conventional-commits.org/) commit scopes
are optional, but should be used where possible. The scope name should be the
same as the directory the main changes are in.

> It is common to make changes in some 'scope' with complementary updates to
> dependant source code or documentation in the same set of changes, these
> should be noted in the commit *body* clearly, but the scope can remain
> unchanged.
