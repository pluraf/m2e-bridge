#!/bin/bash

set -e

../tools/export_specs.py ../src/ ./source/

clear && make clean && poetry run make html
