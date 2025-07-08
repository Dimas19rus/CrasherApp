TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += \
    ./example_lib \
    ./example_app

example_lib.subdir = examples/example_lib
example_app.subdir = examples/example_app

# Установим зависимость: example_app зависит от example_lib
example_app.depends = example_lib
