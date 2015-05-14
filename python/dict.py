class AttrDict(dict):
    # easydict, attrdict
    def __init__(self, d=None, **kwargs):
        super(dict, self).__init__()
        if d is None:
            d = {}
        if kwargs:
            d.update(**kwargs)
        for k, v in d.items():
            setattr(self, k, v)
        # class attributes
        for k in self.__class__.__dict__.keys():
            if not (k.startswith('__') and k.endswith('__')):
                setattr(self, k, getattr(self, k))

    def __setattr__(self, key, value):
        if isinstance(value, (list, tuple)):
            value = [self.__class__(x) if isinstance(x, dict) else x for x in value]
        else:
            value = self.__class__(value) if isinstance(value, dict) else value
        super(AttrDict, self).__setattr__(key, value)
        super(AttrDict, self).__setitem__(key, value)

    __setitem__ = __setattr__
