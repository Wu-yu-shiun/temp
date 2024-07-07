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
    model = data.get('model', 'Taide')

    questions = [
        "幫我逐一判斷每一則通知的內容是否重要？請用一個字「是」或「否」做簡答，請不要進一步說明",
        "幫我逐一判斷每一則通知的內容是否和使用者身邊的人相關？請用一個字「是」或「否」做簡答，請不要進一步說明",
        "幫我逐一判斷每一則通知的內容是否來自於社交媒體？請用一個字「是」或「否」做簡答，請不要進一步說明",
        "幫我逐一判斷每一則通知的內容是否存在數字，若有，請用簡答的方式將所有數字列出，若無，請用一個字「無」做簡答，請不要進一步說明",
    ]

    answers = []
    for question in questions:
        messages = [
            {"role": "user", "content": input_content},
            {"role": "user", "content": question}
        ]
        response = ask_ollama(model, messages)
        answers.append({"question": question, "answer": response})

    return jsonify(answers)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
