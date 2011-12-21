#
if [ "$1" == "" -o "$2" == "" -o "$3" == "" ]
then
   echo usage: benchmark.sh max-code number-of-bits directory
   exit
fi
echo "Filename" "Original-size" "LZW-size" "gzip-size"
echo "--------" "-------------" "--------" "---------"
for a in $3/*
do 
    if [ -f $a ]
    then
        ./lzw -max $1 -c $a /tmp/$$.lzw
        ./lzw -max $1 -d /tmp/$$.lzw /tmp/$$.out
        gzip -c $a > /tmp/$$.gzip
        echo $a `stat $a --format=%s` `stat /tmp/$$.lzw --format=%s` `stat /tmp/$$.gzip --format=%s`
        cmp $a /tmp/$$.out
        if [ $? -ne 0 ]
        then
            echo "error compressing $a"
            break; 
        fi
    fi
done
