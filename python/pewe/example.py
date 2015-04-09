from datetime import date
from pewe import *

db = MySQLDatabase('test')


class Person(Model):
    name = CharField()
    birthday = DateField()
    is_relative = BooleanField()

    class Meta:
        database = db


if __name__ == '__main__':
    db.create_tables([Person, ])

    bob = Person(name='Bob', birthday=date(1960, 1, 15), is_relative=True)
    bob.save()