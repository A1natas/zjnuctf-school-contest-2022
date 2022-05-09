#!/bin/bash

if [[ -f /flag.sh ]]; then
	source /flag.sh
	rm -f /flag.sh
fi

apache2-foreground

