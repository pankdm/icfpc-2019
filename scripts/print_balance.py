import json

for i in range(1, 100):
    filename = "../lambda-client/blocks/%d/balances.json" % i
    s = open(filename).read()
    balance = 0
    j = json.loads(s)
    if "15" in j:
        balance = j["15"]
    print(i, balance)
