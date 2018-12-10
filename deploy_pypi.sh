#!/bin/bash


rm -rf dist

# You will need proper PyPI credentials to do that
python setup.py sdist
cd dist
twine upload -r pypi *.tar.gz
cd ..

rm -rf dist
