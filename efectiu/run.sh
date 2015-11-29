#!/bin/csh
if ( $1 == "" ) then
    printf "Usage: $0 <traces-directory>\n"
    exit 1
endif
rm -f speedups.out
foreach benchmark ( $1/*.trace.gz )
    foreach policy ( 0 2 )
        rm -f $policy.ipc
        setenv DAN_POLICY $policy
        ./efectiu $benchmark |& tee efectiu.out
        set ipc = `tail efectiu.out | grep "core.*IPC" | sed -e '/^.*: /s///' | sed -e '/ IPC/s///'`
        echo $ipc > $policy.ipc
    end
    printf "scale=10 ; %s / %s\n" `cat 2.ipc` `cat 0.ipc` | bc >> speedups.out
end
set N = `wc -l < speedups.out`
set i = 1
printf "scale = 10\na = " > bc.in
foreach x ( `cat speedups.out` )
    printf "$x " >> bc.in
    if ( $i != $N ) printf "* " >> bc.in
    @ i = $i + 1
end
printf "\nb = 1 / $N\ne(b * l(a))\n" >> bc.in
printf "Geometric mean speedup is "
bc -l < bc.in

./geometric.m
