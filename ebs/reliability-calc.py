#!/usr/sbin/python

# http://xiaoquqi.github.io/blog/2015/03/04/ceph-reliability/
# http://oceanbase.org.cn/?p=151
# https://www.ustack.com/blog/build-block-storage-service/
# https://en.wikipedia.org/wiki/Annualized_failure_rate
# http://www.microsemi.com/document-portal/doc_view/124041-calculating-reliability-using-fit-mttf-arrhenius-htol-model

import decimal
import math
import sys 


afr = decimal.Decimal(str(sys.argv[2])) # 0.188, means 0.188%, disk annual failure rate, expressed in %
fit = decimal.Decimal(str(afr/24/365)) # probability of n failure events during time t, here t in hours
e = decimal.Decimal(str(math.e))


def factorial(n):
    '''n!'''
    s = decimal.Decimal("1")
    for i in range(1, n+1):
        s = s * decimal.Decimal(str(i))
    return s


def C(n, m):
    '''choose m out of n'''
    return factorial(n) / (factorial(m) * factorial(n-m))


def poisson(t, n):
    '''
    t: time unit, can be year/day/hour/second, you define it.
    Pn(lamda, t) = (lamda * t)^n * e^(-lamda*t) / n!
        -- in time t, probability of event "n disks failed"
    P1(lamda, 1) = lamda * e^(-lamda) = afr
    '''
    lamda = fit * n
    print 'poisson n={0}, lamda={1}'.format(n, lamda)
    return ((lamda * t) ** n) * (e ** (-lamda * t)) / factorial(n)


def rebuild_hrs_1disk(nr_peers):
    disk_capacity = decimal.Decimal(str(745 * 1024 * 0.7))  # MB, 800G ssd
    throttle_mb_per_peer = decimal.Decimal(str(5 * 4))  # 20MB/s
    secs = decimal.Decimal(disk_capacity / (throttle_mb_per_peer * nr_peers))
    hrs = decimal.Decimal(secs / 60 / 60)
    return hrs


def copy_sets(nr_disks, nr_zones, nr_replicas):
    nr_disks_per_zone = nr_disks / nr_zones
    return C(nr_zones, nr_replicas) * (C(nr_disks_per_zone, 1) ** nr_replicas)


def possibility_fail(time, nr_replicas):
    return afr * poisson(time, nr_replicas - 1)


def reliability_replicas(nr_zones, nr_disks, time, nr_replicas):
    loss_data = possibility_fail(time, nr_replicas) * copy_sets(nr_disks, nr_zones, nr_replicas) / C(nr_disks, nr_replicas)
    return 1 - loss_data

if __name__ == '__main__':
    nr_servers = int(sys.argv[1])
    nr_disks = 12 * nr_servers
    nr_peers = nr_servers / 3 * 12 - 1
    rebuild_hrs = rebuild_hrs_1disk(nr_peers)
    print 'afr={0}, fit={1}, nr_servers={2}, nr_disks={3}, nr_peers={4}, rebuild_hrs={5}'.format(afr, fit, nr_servers, nr_disks, nr_peers, rebuild_hrs)
    print reliability_replicas(3, nr_disks, rebuild_hrs, 3)

