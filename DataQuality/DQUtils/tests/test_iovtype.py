#! /usr/bin/env python

# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from DQUtils.sugar.iovtype import RANGEIOV_VAL, make_iov_type
from DQUtils.sugar import RunLumi
from DQUtils.sugar.runlumi import TimestampType


def test_empty_is_boolean_false():
    """
    Check that "empty" IOV type evaluates to False
    """
    empty = RANGEIOV_VAL.empty()
    assert not empty
    return True


def test_runs():
    iov = RANGEIOV_VAL(RunLumi(1,1), RunLumi(3,1))
    assert list(iov.runs) == [1,2,3]
    assert iov.run == 1


def test_contains_point():
    iov = RANGEIOV_VAL(RunLumi(1,1), RunLumi(3,1))
    assert iov.contains_point(RunLumi(2,1))
    assert not iov.contains_point(RunLumi(4,1))


def test_intersect():
    iov1 = RANGEIOV_VAL(RunLumi(1,1), RunLumi(3,1))
    iov2 = RANGEIOV_VAL(RunLumi(2,1), RunLumi(4,1))
    iov3 = RANGEIOV_VAL(RunLumi(4,1), RunLumi(5,1))
    assert iov1.intersects(iov2)
    assert not iov1.intersects(iov3)
    assert iov1.intersect(iov2) == RANGEIOV_VAL(RunLumi(2,1), RunLumi(3,1))
    assert iov1.intersect(iov3) is None
    assert iov1.intersect_run(1) == RANGEIOV_VAL(RunLumi(1,1), RunLumi(1,0xffffffff))
    assert iov1.intersect_run(4) is None


def test_length():
    iov1 = RANGEIOV_VAL(RunLumi(1,1), RunLumi(3,1))
    assert iov1.length == 2 << 32


def test_connected_to():
    CLS_VAL = make_iov_type('TESTIOV', ['var'])
    iov1 = CLS_VAL(RunLumi(1,1), RunLumi(3,1), var=5)
    iov2 = CLS_VAL(RunLumi(2,1), RunLumi(4,1), var=5)
    iov3 = CLS_VAL(RunLumi(3,1), RunLumi(5,1), var=5)
    iov4 = CLS_VAL(RunLumi(4,1), RunLumi(6,1), var=5)
    iov5 = CLS_VAL(RunLumi(3,1), RunLumi(5,1), var=6)
    assert not iov1.connected_to(iov2)  # false when overlapping
    assert not iov1.connected_to(iov4)  # false when a gap
    assert not iov1.connected_to(iov5)  # false when connected by contents different
    assert iov1.connected_to(iov3)  # this one yes
    assert iov1.merge(iov3) == CLS_VAL(RunLumi(1,1), RunLumi(5,1), var=5)


def test_is_time_based():
    assert RANGEIOV_VAL(TimestampType.from_string('01/01/2020'),
                        TimestampType.from_string('02/01/2020')).is_time_based
    assert not RANGEIOV_VAL(RunLumi(1,1), RunLumi(3,1)).is_time_based


def test_trimming():
    iov = RANGEIOV_VAL(RunLumi(1,0), RunLumi(1, 0xffffffff))
    assert iov.trimmed == RANGEIOV_VAL(RunLumi(1,1), RunLumi(1, 0xffffffff))


if __name__ == '__main__':
    test_empty_is_boolean_false()
    test_runs()
    test_contains_point()
    test_intersect()
    test_length()
    test_connected_to()
    test_is_time_based()
    test_trimming()
