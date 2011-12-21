#
if [ "$1" == "" -o "$2" == "" -o "$3" == "" ]
then
   echo usage: benchmark.sh max-code number-of-bits directory
   echo ""
   echo The output of this script looks nice when piped into "column -t"
   exit
fi
echo "Filename" "Original-size" "LZW-size" "Compress-size"
echo "--------" "-------------" "--------" "-------------"
for a in $3/*
do 
    if [ -f $a ]
    then
        ./lzw -max $1 -c $a /tmp/$$.lzw
        ./lzw -max $1 -d /tmp/$$.lzw /tmp/$$.out
        compress -b $2 -c $a > /tmp/$$.compress
        echo $a `stat $a --format=%s` `stat /tmp/$$.lzw --format=%s` `stat /tmp/$$.compress --format=%s`
        cmp $a /tmp/$$.out
        if [ $? -ne 0 ]
        then
            echo "error compressing $a"
            break; 
        fi
    fi
done
