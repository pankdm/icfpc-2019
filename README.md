# Solution

The main idea of our solution is pretty obvious. We have a family of strategies. Many strategies have parameters (optional usage of boosters and so on).

The most productive strategy ideas were:
- Clone as many wrappers as possible. Extend one or all wrapper manipulators. Each wrapper wrap the closest unwrapped point.
- Split the whole map into areas (mean to be well connected). Assign an area to a wrapper.

# Mining

For mining we used subset of strategies. Each of the mining blocks was processed in <5 minutes. Every minute we updated binary with compiled strategies from git master.

## Map generator

Map generation is pretty simple. As soon as map quality and difficulty doesn't bring any value to the final score, any valid solution will work.

We're building kind of Spanning tree, where we're trying to connect points from oSqr to the border of the area. By construction, it resolves almost every constraint, and remaining parts are fulfilled later. For example, to ensure we have enough vertices, every time we have 3+ pixels straight edge, we can add one pixel wide "hair", increasing amount of vertices by 4. Fortunately, we never faced situation when we need to reduce amound of vertices. All the boosters are put randomily, because, again, there are no incentives to build complicated map, or map specially tailored to our solvers.

# Infrastructure

## Storage

For storing solutions we used our own buy_db database implemented via file system:

```
buy_db/
   |---- task1/
   |---- task2/
           |--- 1.meta.yaml
           |--- 1.sol
           |--- 1.buy
```
Basically, the optimizer was treating solvers as black box and feeding various booster options,
evaluating and appending results into db together with metadata.
Then, on top special scripts could provide analytics or selection of best solutions (see best_buy.py section)


# Tooling

## submit.py

Script submit.py is responsible for submission and persistence of the best known "pure solution" (i.e. solution without purchasing boosters). Best solutions are stored in solutions_gold to ensure no breaking changes will affect final solution.

Script compares new solutions with gold ones, does client-side validation if necessary, creates zip file with combined best known set, sends it to the server. Then it parses html response, and waits in busy loop (sorry, we didn't have better API available) for test results. After server validation we're updating gold solutions and gold archive if there are good updates we can store.

## best_buy.py

Script best_buy.py is responsible for merging pure solutions with list of solutions with purchased boosters. It uses greedy knapsack approach based on empirical ROI formula. As soon as we're spending less lambdacoins than boost we gain, we should be good to use. As well as submit.py, this script ensures we're storing "safe" solution in dedicated directory. Other details are similar to submit.py.

# Usage

For python scripts, use Python 3 + virtual environment, most of scripts should be launched from scripts folder, except submit.py and best_buy.py. Run `pip3 install -r scripts/requirements.txt` to install libraries. Configuration can be provided via command line or yaml configs.

For C++ code, run `make` in `src` folder and launch built binary providing args as described in source code.

# Team members
- Dmitry Panin
- Oleg Godovykh
- Vitaly Abdrashitov
- Alexey Poyarkov
- Evegeniy Krasko
- Den Raskovalov

# Tools
- C++
- python3
- bash
- git
- make
- Slack
