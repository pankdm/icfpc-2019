
# git pull --rebase

cd ../src
make clean
make

cd ../scripts

for i in `seq 0 2`; do
    echo "starting $i"
    ./buys_solve_many.py $i 3 > buys.${i}.log &
done

wait
