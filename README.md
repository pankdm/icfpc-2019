# Mining

## Map generator

Map generation is pretty simple. As soon as map quality and difficulty doesn't bring any value to the final score, any valid solution will work.

We're building kind of Spanning tree, where we're trying to connect points from oSqr to the border of the area. By construction, it resolves almost every constraint, and remaining parts are fulfilled later. For example, to ensure we have enough vertices, every time we have 3+ pixels straight edge, we can add one pixel wide "hair", increasing amount of vertices by 4. Fortunately, we never faced situation when we need to reduce amound of vertices. All the boosters are put randomily, because, again, there are no incentives to build complicated map, or map specially tailored to our solvers.

# Tooling

## submit.py

Script submit.py is responsible for submission and persistence of the best known "pure solution" (i.e. solution without purchasing boosters). Best solutions are stored in solutions_gold to ensure no breaking changes will affect final solution.

Script compares new solutions with gold ones, does client-side validation if necessary, creates zip file with combined best known set, sends it to the server. Then it parses html response, and waits in busy loop (sorry, we didn't have better API available) for test results. After server validation we're updating gold solutions and gold archive if there are good updates we can store.

## best_buy.py

Script best_buy.py is responsible for merging pure solutions with list of solutions with purchased boosters. It uses greedy knapsack approach based on empirical ROI formula. As soon as we're spending less lambdacoins than boost we gain, we should be good to use. As well as submit.py, this script ensures we're storing "safe" solution in dedicated directory. Other details are similar to submit.py.