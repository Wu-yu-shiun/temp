from flask import Flask, request, jsonify
import requests

app = Flask(__name__)

OLLAMA_API_URL = "http://localhost:11434/api/chat"

def ask_ollama(model, messages, stream=False):
    ollama_request_data = {
        "model": model,
        "messages": messages,
        "stream": stream
    }
    response = requests.post(OLLAMA_API_URL, json=ollama_request_data)
    return response.json()

@app.route('/ask', methods=['POST'])
def ask_model():
    data = request.json
    input_content = data.get('content', '')
    model = data.get('model', 'TaideF32')

    question_array = [
        "根據上述各則通知的內容，依序判斷每一則通知是否重要。(用是非題的方式進行簡答，也就是說回答的內容只能為「是」或「否」，不需加上其他說明，回答所有問題之前和之後也不需對整體結果有任何說明)",
        "根據上述各則通知的內容，依序判斷每一則通知是否來自使用者身邊的人所傳的訊息。(用是非題的方式進行簡答，也就是說回答的內容只能為「是」或「否」，不需加上其他說明，回答所有問題之前和之後也不需對整體結果有任何說明)",
        "根據上述各則通知的內容，依序判斷每一則通知是否提及發票。(用是非題的方式進行簡答，也就是說回答的內容只能為「是」或「否」，不需加上其他說明，回答所有問題之前和之後也不需對整體結果有任何說明)",
        "根據上述各則通知的內容，依序判斷每一則通知是否存在驚嘆號。(用是非題的方式進行簡答，也就是說回答的內容只能為「是」或「否」，不需加上其他說明，回答所有問題之前和之後也不需對整體結果有任何說明)"
    ]

    answers = []
    for question in question_array:
        messages = [
            {"role": "user", "content": input_content},
            {"role": "user", "content": question}
        ]
        response = ask_ollama(model, messages)
        answers.append({"question": question, "answer": response})

    return jsonify(answers)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)