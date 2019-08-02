# cli-fm
The only real and effective CLI file manager.

**This software is in early development stage and it's syntax will be changing
regularly**

## Usage

```
cli-fm <command> [options]
```

The command may be on of the following:
* `ls`, `list`
* `mv`, `move`
* `cp`, `copy`

All these commands will work like the standard `coreutils` one except they will
take into account the `CDPATH` environment variable. Also when the source is a
directory, the most recently modified item in the directory will be cp/mv-ed.
If you want more than one item use `-t` as an option.

## Features

* Takes full advantage of the `CDPATH` environment variable. No more need to type
  in long full paths
* Copy/move the most recent/old files
* Fast
* Easily scriptable

## Dependencies

* coreutils (cp, ls, mv)

## TODO

* Make this program universal and work on any commmand
* Add shell completion (not in shell because that's bloat)
* Add tests because I can't remember everything to test

