from setuptools import Extension, setup

setup(
    ext_modules=[
        Extension(
            name="ctask2",
            sources=["src/ctask2module.c"],
            py_limited_api=True,
        ),
        Extension(
            name="ctask3",
            sources=["src/ctask3module.c"],
            py_limited_api=True,
        ),
    ]
)
