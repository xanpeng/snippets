# coding=utf-8

import logging
import logging.config

default_logtofile = False
default_logconf = '/path/to/file'
global_loglevel = logging.INFO

GLOBAL_LEVEL_NAMES = {
    logging.CRITICAL: 'CRITICAL',
    logging.ERROR: 'ERROR',
    logging.WARNING: 'WARNING',
    logging.INFO: 'INFO',
    logging.DEBUG: 'DEBUG',
    logging.NOTSET: 'NOTSET',
    'CRITICAL': logging.CRITICAL,
    'ERROR': logging.ERROR,
    'WARN': logging.WARNING,
    'WARNING': logging.WARNING,
    'INFO': logging.INFO,
    'DEBUG': logging.DEBUG,
    'NOTSET': logging.NOTSET,
}


def set_logtofile(logtofile=True):
    global default_logtofile
    default_logtofile = logtofile


def set_logconf(log_conf):
    global default_logconf
    default_logconf = log_conf


def get_logger(name=None):
    if not default_logtofile:
        return get_console_logger(None)  # 只用root logger，否则console logger在fileConfig()后失效
    return get_file_logger(None)


def get_console_logger(name):
    logging.basicConfig()
    logger = logging.getLogger(name)
    logger.setLevel(global_loglevel)
    return logger


def get_file_logger(logger_name):
    logging.config.fileConfig(default_logconf)
    logger = logging.getLogger(logger_name)
    return logger


def set_global_loglevel(level):
    global global_loglevel
    if level == global_loglevel:
        return
    if level not in GLOBAL_LEVEL_NAMES:
        return
    global_loglevel = level
    logging.root.setLevel(global_loglevel)
    for logger in logging.Logger.manager.loggerDict.itervalues():
        if not isinstance(logger, logging.PlaceHolder):
            logger.setLevel(global_loglevel)


def name_level_map(level_or_name):
    if level_or_name not in GLOBAL_LEVEL_NAMES:
        if isinstance(level_or_name, str):
            return logging.DEBUG
        else:
            return 'DEBUG'
    return GLOBAL_LEVEL_NAMES[level_or_name]

# usage
if __name__ == '__main__':
    logger = log.get_logger(__name__)
    logger.debug('hello')
    log.set_global_loglevel(logging.DEBUG)
    logger.debug('hello again')
    
