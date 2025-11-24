#!/usr/bin/env bash

# compile gim
if gcc gim.c -o gim -O1 -Wall -Wextra -pedantic -lncurses; then
    # move to PATH
    if sudo cp ./gim /usr/bin/gim; then
        # remove binary from local repo
        if rm ./gim; then
            # status message
            printf ":: done ♥\n"
        else
        # or kill myself
            printf ":: couldnt remove binary from local repo, fuck you ♥\n"
        fi
    else
    # or kill yourself
        printf ":: couldnt copy to PATH, fuck you ♥\n"
    fi
else 
# or kill ourselves
    printf ":: couldnt compile, fuck you ♥\n"
fi

