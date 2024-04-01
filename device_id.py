import os

def get_id():
    f = open('/id.txt', 'a')
    id = f.read()
    f.close()
    return id