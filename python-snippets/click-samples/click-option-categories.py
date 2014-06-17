#
# This script is only for reference.
#

import click

@click.command()
@click.option('--count', default=1, help='number of greetings')
@click.argument('name')
def hello(count, name):
    for x in range(count):
        print('Hello %s!' % name) 

# basic value options
@click.command()
@click.option('--n', default=1)
def dots(n):
    print('.' * n) # python dots.py --n=2

# multi value options
@click.command()
@click.option('--pos', nargs=2, type=float)
def findme(pos):
    print('%s / %s' % pos) # python findme.py --pos 2.0 3.0

# multiple options
@click.command()
@click.option('--message', '-m', multiple=True)
def commit(message):
    print('\n'.join(message)) # python commit.py -m foo -m bar

# counting & flag
@click.command()
@click.option('-v', '--verbose', is_flag=True, multiple=True)
def log(verbose):
    verbosity = len(verbose)
    print('Verbosity: %s' % verbosity) # "python log.py -vvv" gets "Verbosity: 3" 

# boolean flags
@click.command()
@click.option('--shout/--no-shout', default=False)
def info(shout):
    if shout:
        print '!!!!' # python info.py --shout, python info.py --no-shout

# choice options
@click.command()
@click.option('--hash-type', type=click.Choice(['md5', 'sha1']))
def digest(hash_type):
    print(hash_type) # python digest --hash-type=md5

# prompting
@click.command()
@click.option('--name', prompt=True)
# or: @click.option('--name', prompt='Your name please')
# or: @click.option('--password', prompt=True, hide_input=True, confirmation_promt=True)
def hello(name):
    print('Hello %s!' % name) # "python hello.py --name=John" or "python hello.py" and wait for input

if __name__ == '__main__':
    hello()

