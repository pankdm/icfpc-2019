import sys


def post_to_slack(s):
    try:
        import os
        import slack
        print ('Sending to slack')
        client = slack.WebClient(token=os.environ['SLACK_API_TOKEN'])
        response = client.chat_postMessage(
                channel='#bot-logs',
                text=s)
    except:
        print("[slack] Unexpected error:", sys.exc_info()[0:2])

if __name__ == "__main__":
    response = client.chat_postMessage(
            channel='#bot-logs',
            text="Hello hackers!")
    assert response["ok"]
    assert response["message"]["text"] == "Hello hackers!"
