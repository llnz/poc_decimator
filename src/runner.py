#! /usr/bin/env python

import sys
import time
import os
import tempfile
import subprocess
import csv

def rundecimator(file, rlvl, qlvl):
    '''run the decimator with the given max quality levels'''
    
    print "running decimator with resolution %d and quality %s on %s" % (rlvl, qlvl, file), 
    sys.stdout.flush()
    
    #create destfile name securely
    tfile = tempfile.NamedTemporaryFile()#delete=False)
    destfile = tfile.name
    tfile.close()
    
    stime = time.time()
    
    rv = subprocess.call('./poc_decimator -r %d -l %d %s %s %s > /dev/null' % (rlvl, qlvl, file, destfile, '/dev/null'), shell=True)
    
    etime = time.time()
    dtime = etime - stime
    
    if rv == 0:
         print "ok, %s s" % dtime
    else:
        print "failed"
    
    size = os.path.getsize(destfile)
    
    result = (rv, file, rlvl, qlvl, dtime, size)
    
    return result


if __name__ == "__main__":
    
    if len(sys.argv) != 3:
        sys.exit("Wrong arguments: %s <img> <result.csv>" % sys.argv[0])
        
    file = sys.argv[1]
    
    result = []
    
    max_qlvl = 10
    max_rlvl = 10
    
    largest_size = 0
    
    for rlvl in range(1,max_rlvl):
        
        last_size = 0
        
        for qlvl in range(1, max_qlvl):
            
            resultrow = rundecimator(file, rlvl, qlvl)
            
            if resultrow[5] == last_size:
                max_qlvl = qlvl
                break
            
            last_size = resultrow[5]
            result.append(resultrow)
            
        if last_size == largest_size:
            result.pop()
            break
        
        largest_size = last_size
    
    writer = csv.writer(open(sys.argv[2], 'wb'))
    writer.writerows(result)
    