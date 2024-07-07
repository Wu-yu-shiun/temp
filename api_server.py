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
        "以下通知的內容對使用者來說是否重要？(一則通知做一次回答，回答只需一個字：是/否)",
        "以下通知的內容是否和使用者身邊的人相關？(一則通知做一次回答，回答只需一個字：是/否)",
        "判斷以下的通知是否來自於社交媒體？(一則通知做一次回答，回答只需一個字：是/否)",
        "以下通知的內容是否有提及數字？(一則通知做一次回答，若有，回答只需呈現出現過的數字。若無，回答只需一個字：無)",
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
