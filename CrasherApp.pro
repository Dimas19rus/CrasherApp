TEMPLATE = subdirs

# Пути к сабпроектам
SUBDIRS += libs/error_logger \
           app/demon_crash \
           examples/examples.pro

# Установка кастомных build-выходов
libs/error_logger.subdir = libs/error_logger
libs/error_logger.target = error_logger
libs/error_logger.CONFIG += ordered

app/demon_crash.subdir = app/demon_crash
app/demon_crash.target = demon_crash
app/demon_crash.CONFIG += ordered

examples/examples.pro.subdir = examples
examples/examples.pro.target = examples
examples/examples.pro.CONFIG += ordered





# Установка кастомной папки сборки для каждого сабпроекта
# Это работает при использовании `qmake` с аргументами `-spec` и `CONFIG+=<тип>`

# Необязательно — просто рекомендуемый порядок
