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
    model = data.get('model', 'TaideQ3KL')

    questions = [
        "請逐一判斷每一則通知的內容是否重要，判斷完後回答「是」或「否」即可。",
        "請逐一判斷每一則通知的內容是否和使用者身邊的人相關，判斷完後回答「是」或「否」即可。",
        "請逐一判斷每一則通知的內容是否存在驚嘆號，判斷完後回答「是」或「否」即可。",
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
