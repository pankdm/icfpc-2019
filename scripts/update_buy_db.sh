

for i in `seq 20 10000`; do
    git pull --rebase
    git add ../buy_db
    ./buy_db_analytics.py >roi-full.txt
    git commit -a -m "update buy_db v${i}"
    git push
    sleep 180
done