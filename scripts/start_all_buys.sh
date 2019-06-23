
for i in `seq 0 4`; do
    ./buys_solve_many.py $i > buys.$i.log &
done