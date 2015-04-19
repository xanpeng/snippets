# rewrite from https://github.com/jlhutch/pylru

# Cache implementaion with a Least Recently Used (LRU) replacement policy and
# a basic dictionary interface.
# Copyright (C) 2006, 2009, 2010, 2011 Jay Hutchinson

# The cache is implemented using a combination of a python dictionary (hash
# table) and a circular doubly linked list. Items in the cache are stored in
# nodes. These nodes make up the linked list. The list is used to efficiently
# maintain the order that the items have been used in. The front or head of
# the list contains the most recently used item, the tail of the list
# contains the least recently used item. When an item is used it can easily
# (in a constant amount of time) be moved to the front of the list, thus
# updating its position in the ordering. These nodes are also placed in the
# hash table under their associated key. The hash table allows efficient
# lookup of values by key.
import functools


# Class for the node objects.
class _DoubleLinkedNode(object):
    def __init__(self):
        self.empty = True
        self.prev = None
        self.next = None


class LRUCache(object):
    def __init__(self, size, callback=None):
        self.callback = callback

        # Create an empty hash table.
        self.table = {}

        # Initialize the doubly linked list with one empty node. This is an
        # invariant. The cache size must always be greater than zero. Each
        # node has a 'prev' and 'next' variable to hold the node that comes
        # before it and after it respectively. Initially the two variables
        # each point to the head node itself, creating a circular doubly
        # linked list of size one. Then the size() method is used to adjust
        # the list to the desired size.
        self.head = _DoubleLinkedNode()
        self.head.next = self.head
        self.head.prev = self.head
        self.list_size = 1
        # Adjust the size
        self.size(size)

    def __len__(self):
        return len(self.table)

    def clear(self):
        for node in self.double_linked_iterate():
            node.empty = True
            node.key = None
            node.value = None
        self.table.clear()

    def __contains__(self, key):
        return key in self.table

    # Looks up a value in the cache without affecting cache order.
    def peek(self, key):
        node = self.table[key]
        return node.value

    def __getitem__(self, key):
        node = self.table[key]

        # Update the list ordering. Move this node so that is directly
        # proceeds the head node. Then set the 'head' variable to it. This
        # makes it the new head of the list.
        self.move_to_front(node)
        self.head = node
        return node.value

    def get(self, key, default=None):
        """Get an item - return default (None) if not present"""
        try:
            return self[key]
        except KeyError:
            return default

    def __setitem__(self, key, value):
        # if key exists
        if key in self.table:
            node = self.table[key]
            node.value = value
            self.move_to_front(node)
            self.head = node
            return

        # if key does't exist, choose tail node, the empty node or least recently used node
        node = self.head.prev
        if not node.empty:  # tail node contains something
            if self.callback is not None:
                self.callback(node.key, node.value)
            del self.table[node.key]
        # Place the new key and value in the node
        node.empty = False
        node.key = key
        node.value = value
        self.table[key] = node

        self.head = node

    def __delitem__(self, key):
        node = self.table[key]
        del self.table[key]
        node.empty = True

        # Not strictly necessary.
        node.key = None
        node.value = None

        # move empty node to tail
        self.move_to_front(node)
        self.head = node.next

    def __iter__(self):
        # in order from the most recently to least recently used, and does't modify the cache order
        for node in self.double_linked_iterate():
            yield node.key

    def items(self):
        # in order from the most recently to least recently used, and does't modify the cache order
        for node in self.double_linked_iterate():
            yield (node.key, node.value)

    def keys(self):
        # in order from the most recently to least recently used, and does't modify the cache order
        for node in self.double_linked_iterate():
            yield node.key

    def values(self):
        # in order from the most recently to least recently used, and does't modify the cache order
        for node in self.double_linked_iterate():
            yield node.value

    def size(self, size=None):
        if size is not None:
            assert size > 0
            if size > self.list_size:
                self.add_tail_node(size - self.list_size)
            elif size < self.list_size:
                self.remove_tail_node(self.list_size - size)
        return self.list_size

    # Increases the size of the cache by inserting n empty nodes at the tail of the list.
    def add_tail_node(self, n):
        for i in xrange(n):
            node = _DoubleLinkedNode()
            node.next = self.head
            node.prev = self.head.prev
            self.head.prev.next = node
            self.head.prev = node
        self.list_size += n

    # Decreases the size of the list by removing n nodes from the tail of the list.
    def remove_tail_node(self, n):
        assert self.list_size > n
        for i in range(n):
            node = self.head.prev
            if not node.empty:
                if self.callback is not None:
                    self.callback(node.key, node.value)
                del self.table[node.key]

            # Splice the tail node out of the list
            self.head.prev = node.prev
            node.prev.next = self.head
            # The next four lines are not strictly necessary.
            node.prev = None
            node.next = None
            node.key = None
            node.value = None
        self.list_size -= n

    # This method adjusts the ordering of the doubly linked list so that
    # 'node' directly precedes the 'head' node. Because of the order of
    # operations, if 'node' already directly precedes the 'head' node or if
    # 'node' is the 'head' node the order of the list will be unchanged.
    def move_to_front(self, node):
        node.prev.next = node.next
        node.next.prev = node.prev

        node.prev = self.head.prev
        node.next = self.head.prev.next

        node.next.prev = node
        node.prev.next = node

    # iterate from most recently to least recently used
    def double_linked_iterate(self):
        node = self.head
        for i in range(len(self.table)):
            yield node
            node = node.next


class WriteThroughCacheManager(object):
    def __init__(self, store, size):
        self.store = store
        self.cache = LRUCache(size)

    def __len__(self):
        return len(self.store)

    # Returns/sets the size of the managed cache.
    def size(self, size=None):
        return self.cache.size(size)

    def clear(self):
        self.cache.clear()
        self.store.clear()

    def __contains__(self, key):
        if key in self.cache:
            return True
        if key in self.store:
            return True
        return False

    def __getitem__(self, key):
        try:
            return self.cache[key]
        except KeyError:
            pass
        # It wasn't in the cache. Look it up in the store, add the entry to
        # the cache, and return the value.
        value = self.store[key]
        self.cache[key] = value
        return value

    def get(self, key, default=None):
        """Get an item - return default (None) if not present"""
        try:
            return self[key]
        except KeyError:
            return default

    def __setitem__(self, key, value):
        # Add the key/value pair to the cache and store.
        self.cache[key] = value
        self.store[key] = value

    def __delitem__(self, key):
        # Write-through behavior cache and store should be consistent. Delete it from the store.
        del self.store[key]
        try:
            del self.cache[key]
        except KeyError:
            pass

    def __iter__(self):
        return self.keys()

    def keys(self):
        return self.store.keys()

    def values(self):
        return self.store.values()

    def items(self):
        return self.store.items()


class WriteBackCacheManager(object):
    def __init__(self, store, size):
        self.store = store
        # Create a set to hold the dirty keys.
        self.dirty = set()

        # Define a callback function to be called by the cache when a
        # key/value pair is about to be ejected. This callback will check to
        # see if the key is in the dirty set. If so, then it will update the
        # store object and remove the key from the dirty set.
        def callback(key, value):
            if key in self.dirty:
                self.store[key] = value
                self.dirty.remove(key)

        # Create a cache and give it the callback function.
        self.cache = LRUCache(size, callback)

    # Returns/sets the size of the managed cache.
    def size(self, size=None):
        return self.cache.size(size)

    def clear(self):
        self.cache.clear()
        self.dirty.clear()
        self.store.clear()

    def __contains__(self, key):
        if key in self.cache:
            return True
        if key in self.store:
            return True
        return False

    def __getitem__(self, key):
        try:
            return self.cache[key]
        except KeyError:
            pass

        value = self.store[key]
        self.cache[key] = value
        return value

    def get(self, key, default=None):
        """Get an item - return default (None) if not present"""
        try:
            return self[key]
        except KeyError:
            return default

    def __setitem__(self, key, value):
        # Add the key/value pair to the cache.
        self.cache[key] = value
        self.dirty.add(key)

    def __delitem__(self, key):
        found = False
        try:
            del self.cache[key]
            found = True
            self.dirty.remove(key)
        except KeyError:
            pass

        try:
            del self.store[key]
            found = True
        except KeyError:
            pass

        if not found:  # If not found in cache or store, raise error.
            raise KeyError

    def __iter__(self):
        return self.keys()

    def keys(self):
        for key in self.store.keys():
            if key not in self.dirty:
                yield key

        for key in self.dirty:
            yield key

    def values(self):
        for key, value in self.items():
            yield value

    def items(self):
        for key, value in self.store.items():
            if key not in self.dirty:
                yield (key, value)

        for key in self.dirty:
            value = self.cache.peek(key)
            yield (key, value)

    def sync(self):
        # For each dirty key, peek at its value in the cache and update the
        # store. Doesn't change the cache's order.
        for key in self.dirty:
            self.store[key] = self.cache.peek(key)
        # There are no dirty keys now.
        self.dirty.clear()

    def flush(self):
        self.sync()
        self.cache.clear()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.sync()
        return False


class FunctionCacheManager(object):
    def __init__(self, func, size):
        self.func = func
        self.cache = LRUCache(size)

    def size(self, size=None):
        return self.cache.size(size)

    def clear(self):
        self.cache.clear()

    def __call__(self, *args, **kwargs):
        kwtuple = tuple((key, kwargs[key]) for key in sorted(kwargs.keys()))
        key = (args, kwtuple)
        try:
            return self.cache[key]
        except KeyError:
            pass

        value = self.func(*args, **kwargs)
        self.cache[key] = value
        return value


def lruwrap(store, size, writeback=False):
    if writeback:
        return WriteBackCacheManager(store, size)
    else:
        return WriteThroughCacheManager(store, size)


class LRUDecorator(object):
    def __init__(self, size):
        self.cache = LRUCache(size)

    def __call__(self, func):
        def wrapper(*args, **kwargs):
            kwtuple = tuple((key, kwargs[key]) for key in sorted(kwargs.keys()))
            key = (args, kwtuple)
            try:
                return self.cache[key]
            except KeyError:
                pass

            value = func(*args, **kwargs)
            self.cache[key] = value
            return value

        wrapper.cache = self.cache
        wrapper.size = self.cache.size
        wrapper.clear = self.cache.clear
        return functools.update_wrapper(wrapper, func)
