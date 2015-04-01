# coding=utf-8

"""
http://dev.mysql.com/doc/connector-python/en/connector-python-versions.html:
mysql: 5.1.73,
    yum install mysql-server  # will install dependent package 'mysql'
    yum install mysql-libs
connector: 1.2,
    # yum install MySQL-python  # not this one
    yum install mysql-connector-python
Connector/Python C Extension: need Connector/Python 2.1.1, we don't use it.
python 2.6.6


operations:
    service mysqld start

"""

import mysql.connector
from mysql.connector import connection
from mysql.connector import errorcode

config = {
    'user': 'root',
    'password': '',
    'host': '127.0.0.1',
    'database': 'testdb',
    'raise_on_warnings': True,
}


def show_connect():
    try:
        cnx = mysql.connector.connect(user='root', password='',
                                      host='127.0.0.1',
                                      database='test')
    except mysql.connector.Error as err:
        if err.errno == errorcode.ER_ACCESS_DENIED_ERROR:
            print("username/password error")
        elif err.errno == errorcode.ER_BAD_DB_ERROR:
            print("db does not exist")
        else:
            print(err)
    else:
        cnx.close()

    cnx = connection.MySQLConnection(user='root', password='',
                                     host='127.0.0.1',
                                     database='test')
    cnx.close()

    cnx = mysql.connector.connect(**config)
    cnx.close()


def show_op_database():
    db_name = 'testdb'
    tables = {}
    tables['employees'] = (
        "CREATE TABLE `employees` ("
        "  `emp_no` int(11) NOT NULL AUTO_INCREMENT,"
        "  `first_name` varchar(14) NOT NULL,"
        "  PRIMARY KEY (`emp_no`)"
        ") ENGINE=InnoDB")

    tables['departments'] = (
        "CREATE TABLE `departments` ("
        "  `dept_no` char(4) NOT NULL,"
        "  `dept_name` varchar(40) NOT NULL,"
        "  PRIMARY KEY (`dept_no`), UNIQUE KEY `dept_name` (`dept_name`)"
        ") ENGINE=InnoDB")

    def create_database(cursor):
        try:
            cursor.execute("CREATE DATABASE {0} DEFAULT CHARACTER SET 'utf8'".format(db_name))
        except mysql.connector.Error as err:
            print('failed create db')
            exit(1)

    try:
        cnx = mysql.connector.connect(**config)
        cursor = cnx.cursor()
        cnx.database = db_name
    except mysql.connector.Error as err:
        if err.errno == errorcode.ER_BAD_DB_ERROR:
            create_database(cursor)
            cnx.database = db_name
        else:
            print(err)
            exit(1)

    for name, ddl in tables.iteritems():
        try:
            print('creating table {0}'.format(name))
            cursor.execute(ddl)
        except mysql.connector.Error as err:
            if err.errno == errorcode.ER_TABLE_EXISTS_ERROR:
                print('table already exists')
            else:
                print(err.msg)
        else:
            print('OK')
    cursor.close()
    cnx.close()


def show_rw_table():
    # write table
    cnx = mysql.connector.connect(**config)
    cursor = cnx.cursor()

    add_employee = ("INSERT INTO employees "
                    "(first_name)"
                    "VALUES (%s)")
    data_employee = ('xan',)
    cursor.execute(add_employee, data_employee)
    emp_no = cursor.lastrowid

    # make sure data is committed to the database
    cnx.commit()

    cursor.close()
    cnx.close()

    #
    # read table
    #
    cnx = mysql.connector.connect(**config)
    cursor = cnx.cursor()
    query = ("SELECT first_name FROM employees "
             "WHERE emp_no BETWEEN %s AND %s")
    cursor.execute(query, (str(0), str(100)))
    for first_name in cursor:
        print(first_name)

    cursor.close()
    cnx.close()


if __name__ == '__main__':
    # show_connect()
    # show_op_database()
    show_rw_table()
