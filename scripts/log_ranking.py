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

        prev = self.ranking[index - 1]
        cur = self.ranking[index]
        nxt = self.ranking[index + 1]

        # msg = "\n".join([
        #     "```",
        #     f" rank |  score  | score + LAM |",
        #     f"   #{index} | {prev.score} |  {prev.score_lam}    |",
        #     f">> #{index + 1} | {cur.score} |  {cur.score_lam}    |",
        #     f"   #{index + 2} | {nxt.score} |  {nxt.score_lam}    |",
        #     "```"
        # ])

        table_data = [
            ['Rank', 'Team', 'Score', 'Score + LAM'],
            prev.to_table('   ' + str(index)),
            cur.to_table('>> ' + str(index + 1)),
            nxt.to_table('   ' + str(index + 2))
        ]

        table = AsciiTable(table_data)
        msg = "\n".join([
            '```',
            table.table,
            '```'
        ])
        post_to_slack(msg)


while True:
    try:
        c = Collector()
        c.log_everything()
    except:
        pass
    # sleep 5 minutes
    time.sleep(5 * 60)
