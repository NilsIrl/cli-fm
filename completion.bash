#!/usr/bin/env bash

_cli-fm() {
	if [ "$1" == "$3" ]; then
		COMPREPLY=($(compgen -W "cp mv ls" "$2"))
	else
		_cd
	fi
}

complete -F _cli-fm cli-fm
