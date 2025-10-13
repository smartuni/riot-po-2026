#!/usr/bin/env python3

import sys
from testrunner import run_check_unittests


TIMEOUT = 120


if __name__ == "__main__":
    sys.exit(run_check_unittests(timeout=TIMEOUT, nb_tests=1))
