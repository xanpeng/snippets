def set_excepthook():
    """print exception stack when catch exception"""
    def my_excepthook(exctype, excvalue, tb):
        import traceback
        traceback.print_exception(exctype, excvalue, tb)

    if sys.excepthook == sys.__excepthook__:
        sys.excepthook = my_excepthook
