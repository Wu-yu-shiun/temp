import requests
import json

# content_text = '''以下共有20則通知：
# 1. 新訊息來自[爸爸]:下來吃飯。
# 2. 你的包裹已經出貨，點擊這裡查看追蹤資訊。
# 3. 緊急！低電量：剩餘 5%，請立即充電。
# 4. [Facebook]新版本更新：包含多項性能改進和錯誤修正，立即更新吧！
# 5. 恭喜！你已達成今日的 10,000 步目標！
# 6. 新照片已成功上傳至雲端相簿。
# 7. 你的朋友 [王小明] 在 Instagram 上發表了新照片。
# 8. 今日天氣：晴朗，最高溫度 25°C，最低溫度 18°C。
# 9. 未接來電來自 [媽媽]，請回電。
# 10. 你今晚 7 點的餐廳預約已確認。
# 11. [Line]你有 5 條未讀訊息，立即查看。
# 12. 付款成功，訂單號碼 [AAA1234567]。
# 13. [Netflix]免費試用結束提醒：點擊這裡訂閱以繼續使用完整功能。
# 14. [Gmail]免費贈品！點擊這裡獲取你的限時優惠。
# 15. [Gmail]恭喜你中獎了！點擊這裡領取你的獎品。
# 16. [應用名稱] 提醒：你有一個新的待辦事項。
# 17. [Foodpanda]新的優惠券可用：輸入代碼[abcdefg]享受50%折扣。
# 18. 你的健康報告已更新，點擊這裡查看詳細數據。
# 19. 提醒：今天有兩個會議，請查看你的日程表。
# 20. [蝦皮購物]感謝你的購買，點擊這裡查看你的訂單狀態。'''

noti = ['[Dcard]閒聊版有一篇討論熱烈的文章:「我被軟禁在家(我已成功逃生了)」',
        '[小雞上工]只要30秒，先收藏再說:符合你期望的工作機會又新增2筆，快把握！',
        '[Instagram]1234abc:對你傳送的訊息表示喔',
        '[發票存摺]發票同步通知:113年7月共有1張新發票進來囉，點我前往查看>>',
        '[Line]j王小明:晚上七點要吃飯嗎？',
        '[Dcard]你追蹤的「成功大學版」新文章:「開一個通識成績公布的版」',
        '[Gmail]foodpanda:電子發票開立通知-DH77777777']

response = requests.post(
    'http://34.136.239.98:5000/ask', 
    headers={'Content-Type': 'application/json'},
    json={
        'content': noti,
        'model': 'TaideQ3KL', 
        'stream': False
    }
)

# 檢查請求是否成功
if response.status_code == 200:
    
    data = response.json()
    for item in data:
        print("Q: " + item['question'])
        print("A: " + item['answer']['message']['content'])
        print("---------------------------------------")
else:
    print(f'Error: {response.status_code}')