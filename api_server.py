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
    model = data.get('model', 'llama2')

    questions = [
        # "What are the key points of this information?",
        "Does this information mention any people?",
        "Does this information mention any times or dates?",
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
