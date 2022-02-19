../capi -t 0 -p 0 -P 7777 -I 127.0.0.1 -d -w &
../capi -t 1 -p 0 -P 7777 -I 127.0.0.1 -d -w &
wait
exit
