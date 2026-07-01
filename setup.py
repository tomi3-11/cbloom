import os
from setuptools import Extension, setup

bloom_ext = Extension(
    name="_cbloom", # the import name
    sources=[
        "csrc/bloom.c",
        "csrc/bloommodule.c",
    ],
    include_dirs=["csrc"], # absolute path so #include "bloom.h" works
    extra_compile_args=["-O2", "-std=c99"], # same flags as Makefile
    libraries=["m"], # link libm for log(), ceil()
)

setup(
    ext_modules=[bloom_ext],
)
