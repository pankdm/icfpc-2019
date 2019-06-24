#!/usr/bin/env python3.7

from lxml import html
import requests

from terminaltables import AsciiTable

from slack_integration import post_to_slack

import time

TEAM_NAME = 'Lambding Snakes vs. Coding Monkeys'

class Rank:
    def __init__(self):
        self.name = None
        self.score = None
        self.score_lam = None

    def to_table(self, rank):
        return [rank, self.name, self.score, self.score_lam]

class Collector:
    def __init__(self):
        self.ranking = []
        self.our_index = None

    def collect_ranking(self):
        page = requests.get('https://monadic-lab.org/rankings/latest.html')
        tree = html.fromstring(page.content)
        elements = tree.xpath('/html/body/center/table/tbody/tr')

        for index, elem in enumerate(elements):
            trs = elem.getchildren()
            rank = Rank()
            rank.name = trs[1].text
            rank.score = trs[2].text
            rank.score_lam = trs[3].text
            self.ranking.append(rank)

            if rank.name == TEAM_NAME:
                self.our_index = index

    def log_everything(self):
        self.collect_ranking()

        index = self.our_index
        start_index = max(index - 6, 0)

        # header
        table_data = [
            ['Rank', 'Team', 'Score', 'Score + LAM'] ]

        def add_table(i, our):
            prefix = '>> ' if our else '   '
            str_i = prefix + str(i + 1)
            now = self.ranking[i]
            table_data.append(now.to_table(str_i))

        for i in range(start_index, index + 2):
            add_table(i, i == index)

        # msg = "\n".join([
        #     "```",
        #     f" rank |  score  | score + LAM |",
        #     f"   #{index} | {prev.score} |  {prev.score_lam}    |",
        #     f">> #{index + 1} | {cur.score} |  {cur.score_lam}    |",
        #     f"   #{index + 2} | {nxt.score} |  {nxt.score_lam}    |",
        #     "```"
        # ])
        table = AsciiTable(table_data)

        msg = "\n".join([
            '```',
            table.table,
            '```'
        ])

        print(msg)
        post_to_slack(msg)


while True:
    try:
        c = Collector()
        c.log_everything()
    except:
        pass
    # sleep 5 minutes
    time.sleep(5 * 60)
