#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaCommon.Logging import logging
_msg = logging.getLogger('AccumulatorCache')

import functools
import time
from abc import ABC, abstractmethod
from copy import deepcopy
from collections.abc import Hashable, Iterable
from collections import defaultdict
from dataclasses import dataclass

try:
    from GaudiKernel.DataHandle import DataHandle
except ImportError:
    class DataHandle: pass  # for analysis releases


class AccumulatorCachable(ABC):
    """Abstract base for classes needing custom AccumulatorCache behavior."""

    @abstractmethod
    def _cacheEvict(self):
        """This method is called by AccumulatorCache when an object is removed
        from the cache. Implement this for custom cleanup actions."""
        pass


class AccumulatorDecorator:
    """Class for use in function decorators, implements memoization.

    Instances are callable objects that use the
    hash value calculated from positional and keyword arguments
    to implement memoization. Methods for suspending and 
    resuming memoization are provided.
    """

    _memoize = True

    VERIFY_NOTHING = 0
    VERIFY_HASH = 1

    @dataclass
    class CacheStats:
        hits  : int = 0
        misses: int = 0
        t_hits: float = 0
        t_misses: float = 0

    _stats = defaultdict(CacheStats)

    def __init__(self , func , size , verify , deepCopy): 
        """The constructor is typically called in a function returning a decorator.

        Args:
            func:       function for memoization
            size:       maximum size for the cache of function results
            verify:     takes two possible values
                        
                        AccumulatorDecorator.VERIFY_NOTHING -   the cached function result is returned with no verification
                        AccumulatorDecorator.VERIFY_HASH -      before returning the cached function value, the hash of the
                                                                result is checked to verify if this object was not modified
                                                                between function calls
            deepCopy:   if True a deep copy of the function result will be stored in the cache.

        """
        functools.update_wrapper(self , func)
        self._maxSize = size
        self._func = func
        self._cache = {}
        self._resultCache = {}
        self._verify = verify
        self._deepcopy = deepCopy

        if self._verify not in [self.VERIFY_NOTHING, self.VERIFY_HASH]:
            raise RuntimeError(f"Invalid value for verify ({verify}) in AccumulatorCache for {func}")

    def getInfo(self):
        """Return a dictionary with information about the cache size and cache usage"""
        return {"cache_size" : len(self._cache),
                "misses" : self._stats[self._func].misses,
                "hits" : self._stats[self._func].hits,
                "function" : self._func,
                "result_cache_size" : len(self._resultCache)}

    @classmethod
    def printStats(cls):
        """Print cache statistics"""
        header = "%-70s |    Hits (time) |  Misses (time) |" % "AccumulatorCache"
        print("-"*len(header))
        print(header)
        print("-"*len(header))
        # Print sorted by hit+miss time:
        for func, stats in sorted(cls._stats.items(), key=lambda s:s[1].t_hits+s[1].t_misses, reverse=True):
            name = f"{func.__module__}.{func.__name__}"
            if len(name) > 70:
                name = '...' + name[-67:]
            print(f"{name:70} | {stats.hits:6} ({stats.t_hits:4.1f}s) | "
                  f"{stats.misses:6} ({stats.t_misses:4.1f}s) |")
        print("-"*len(header))

    @classmethod
    def suspendCaching(cls):
        """Suspend memoization for all instances of AccumulatorDecorator."""
        cls._memoize = False

    @classmethod
    def resumeCaching(cls):
        """Resume memoization for all instances of AccumulatorDecorator."""
        cls._memoize = True

    def _hasHash(x):
        if hasattr(x, "athHash"):
            return True
        elif isinstance(x, (Hashable, DataHandle)):
            return True
        return False

    def _getHash(x):
        if hasattr(x, "athHash"):
            return x.athHash()
        elif isinstance(x, Hashable):
            return hash(x)
        elif isinstance(x, DataHandle):
            return hash(repr(x))
        return None

    def _evict(x):
        if isinstance(x, AccumulatorCachable):
            x._cacheEvict()

    def __get__(self, obj, objtype):
        """Support instance methods."""
        return functools.partial(self.__call__, obj)

    def __call__(self , *args , **kwargs):
        cacheHit = None
        try:
            t0 = time.perf_counter()
            res, cacheHit = self._callImpl(*args, **kwargs)
            return res
        finally:
            t1 = time.perf_counter()
            if cacheHit is True:
                self._stats[self._func].hits += 1
                self._stats[self._func].t_hits += (t1-t0)
            elif cacheHit is False:
                self._stats[self._func].misses += 1
                self._stats[self._func].t_misses += (t1-t0)

    def _callImpl(self , *args , **kwargs):
        """Implementation of __call__.

        Returns: (result, cacheHit)
        """
        if(AccumulatorDecorator._memoize):
            hashable_args = True
            for a in args:
                if(not AccumulatorDecorator._hasHash(a)):
                    hashable_args = False
                    _msg.warning("Positional argument '%r' in %s is not hashable.", a, self._func)
                    break
            for k , v in kwargs.items():
                if(not AccumulatorDecorator._hasHash(v)):
                    hashable_args = False
                    _msg.warning("Value '%r' of keyword argument '%s' in %s is not hashable.", v, k, self._func)
                    break
            if(hashable_args):
                # frozen set makes the order of keyword arguments irrelevant
                hsh = hash(tuple((tuple(AccumulatorDecorator._getHash(a) for a in args) , frozenset((hash(k) , AccumulatorDecorator._getHash(v)) for k , v in kwargs.items()))))

                if(hsh in self._cache):
                    res = self._cache[hsh]
                    cacheHit = None
                    if(AccumulatorDecorator.VERIFY_HASH == self._verify):
                        resHsh = self._resultCache[hsh]
                        chkHsh = None
                        if(AccumulatorDecorator._hasHash(res)):
                            chkHsh = AccumulatorDecorator._getHash(res)
                        if((not (chkHsh is None)) and (not (resHsh is None))): 
                            # hashes are available and can be compared
                            if(chkHsh != resHsh):
                                _msg.debug("Hash of function result, cached using AccumulatorDecorator, changed.")
                        else:
                                _msg.debug("Hash of function result, cached using AccumulatorDecorator, not available for verification.")
                        if((chkHsh is None) or (resHsh is None) or resHsh != chkHsh): 
                            # at least one hash is not available (None) so no verification can be performed 
                            # or hashes are different
                            cacheHit = False
                            res = self._func(*args , **kwargs)
                            self._cache[hsh] = res
                            self._resultCache[hsh] = None
                            if(AccumulatorDecorator._hasHash(res)):
                                self._resultCache[hsh] = AccumulatorDecorator._getHash(res)
                        else:
                            cacheHit = True
                    else:
                        cacheHit = True
                
                    if self._deepcopy:
                        return deepcopy(res), cacheHit
                    else:
                        # shallow copied CA still needs to undergo merging
                        from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
                        if isinstance(res, ComponentAccumulator):
                            res._wasMerged=False
                        return res, cacheHit

                else:
                    _msg.debug('Hash not found in AccumulatorCache for function %s' , self._func)
                    if(len(self._cache) >= self._maxSize):
                        _msg.warning("Cache limit (%d) reached for %s.%s", self._maxSize, self._func.__module__, self._func.__name__)
                        oldest = self._cache.pop(next(iter(self._cache)))
                        AccumulatorDecorator._evict(oldest)

                    res = self._func(*args , **kwargs)

                    if(AccumulatorDecorator.VERIFY_HASH == self._verify):
                        if(len(self._resultCache) >= self._maxSize):
                            del self._resultCache[next(iter(self._resultCache))]
                        self._resultCache[hsh] = None
                        if(AccumulatorDecorator._hasHash(res)):
                            self._resultCache[hsh] = AccumulatorDecorator._getHash(res)
                        self._cache[hsh] = res
                    else:
                        self._cache[hsh] = res

                    return (deepcopy(res) if self._deepcopy else res, False)
            else:
                return (self._func(*args , **kwargs), False)
        else:
            return (self._func(*args , **kwargs), None)

    def __del__(self):
        for v in self._cache.values():
            if isinstance(v, Iterable):
                for el in v:
                    AccumulatorDecorator._evict(el)
            else:
                AccumulatorDecorator._evict(v)


def AccumulatorCache(func = None, maxSize = 128,
                     verifyResult = AccumulatorDecorator.VERIFY_NOTHING, deepCopy = True):
    """Function decorator, implements memoization.

    Keyword arguments:
        maxSize: maximum size for the cache associated with the function (default 128)
        verifyResult:   takes two possible values
                        
                        AccumulatorDecorator.VERIFY_NOTHING -   default, the cached function result is returned with no verification
                        AccumulatorDecorator.VERIFY_HASH -      before returning the cached function value, the hash of the
                                                                result is checked to verify if this object was not modified
                                                                between function calls
        deepCopy:   if True (default) a deep copy of the function result will be stored in the cache.
    
    Returns:
        An instance of AccumulatorDecorator.
    """

    def wrapper_accumulator(func):
        return AccumulatorDecorator(func, maxSize, verifyResult, deepCopy)

    return wrapper_accumulator(func) if func else wrapper_accumulator
