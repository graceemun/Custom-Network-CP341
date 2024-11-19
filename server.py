from flask import Flask, render_template, request
import mysql.connector

app = Flask(__name__)

def get_db():
    db = mysql.connector.connect(
            host = 'localhost',
            user = 'tori',
            password = '123456',
            database = 'exchange_language'
    )
    return db

@app.route('/')
def welcome():
    return "Welcome to my webpage!!!"

@app.route('/users/')
def get_users():
    # print all users information in the DB!
    db = get_db()
    cursor = db.cursor()
    sql_cmd = 'SELECT * FROM users'
    cursor.execute(sql_cmd)
    results = []
    for row in cursor.fetchall():
        results.append(f'Username: {row[0]}, Prefer_language: {row[1]}')
    # print(results)
    #return "Successfully ran the MYSQL cmd! You're welcome! ;)"
    return '\n'.join(results)

app.run(host='0.0.0.0', port=8080, debug=True)
