
git commit -a -m "local changes"


for i in `seq 23 10000`; do
    git pull --rebase
    git add ../buy_db
    git commit -a -m "update buy_db v${i}"
    ./buy_db_analytics.py >roi-full.txt
    git commit -a -m "update ROI-full to v${i}"
    git push
    sleep 60
done