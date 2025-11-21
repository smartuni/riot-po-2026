#!/usr/bin/env python3

import sys
from testrunner import run


def testfunc(child):
    child.expect("2 Tests 0 Failures 0 Ignored")
    child.expect("OK")

    print("All tests successful")


if __name__ == "__main__":
    sys.exit(run(testfunc, timeout=10))
