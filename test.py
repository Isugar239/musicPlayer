import sqlite3

# Путь к вашей гифке
gif_path = "send.png"  # Замените на свой файл
gif_name = "send"        # Название для записи

# Подключение к базе данных
conn = sqlite3.connect("music.db3")
cursor = conn.cursor()

# Создание таблицы, если не существует
# Чтение гифки как бинарных данных
with open(gif_path, "rb") as f:
    gif_data = f.read()

# Вставка в таблицу
cursor.execute("INSERT INTO images (name, image) VALUES (?, ?)", (gif_name, sqlite3.Binary(gif_data)))

conn.commit()
conn.close()

print(f"✅ GIF '{gif_name}' успешно добавлен в базу данных.")