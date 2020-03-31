# cli-fm
The only real and effective CLI file manager.

**This software is in early development stage and it's syntax will be changing
regularly**

## Usage

```
cli-fm [options] <command>
```

cli-fm will run the command modifying the paths in the command's argument using
the `CDPATH` environment variable (`help cd`, to learn about `CDPATH`) and
other expansion rules.

cli-fm can also infer certain path arguments using the `-p` option. `-p` should
be the number of path arguments `command` is expecting. The default value of
`-p` changes based on what command is, using some basic rules (e.g. 2 for
`cp`/`mv` or 1 for `ls`).

If the number of arguments that are paths is 1 under the value of `-p`, then
`cli-fm` will add `.` to the argument list.

### Expansion Rules

Expansion works with the colon (`:`) character and commands.

List of available commands:

* `:t`, sorts using the modification time

### Examples

```sh
export CDPATH=~
cd /tmp
cli-fm cp Downloads Documents
```

gets expanded to

```sh
cp ~/Downloads ~/Documents
```

A common use case is to move files you have just downloaded to your workspace.

```sh
export CDPATH=~
cd /tmp
cli-fm mv Downloads/:t
```

gets expanded to

```sh
mv ~/Downloads/last_downloaded_file .
```

Here, the `:t` is replaced with the last modified file (i.e. the file you have
just downloaded), Downloads gets expanded by `CDPATH` and an implicit `.` is
added.

#### `-p`

```sh
$ cli-fm mv Downloads
```

will actually run

```sh
$ mv Downloads .
```

cli-fm detects that the command is `mv` and so sets `-p` to `2`. It is
equivalent to running:

```sh
$ cli-fm mv -p 2 Downloads
```

## Build

```sh
mkdir build && cd build
cmake ..
make
```

## Support

* You can ask me questions on [my stream](https://www.twitch.tv/nilsirl)
* [matrix](https://matrix.to/#/!tLDcLvclSWOGibbnSx:matrix.org?via=matrix.org)
* Email me (My email address can be found on [my github
  profile](https://github.com/NilsIrl/))

## [License](LICENSE)

cli-fm is licensed under the GNU General Public License v3.0.

Copyright © 2020 Nils André
