import sys
import csv
import os
import json
import random
from datetime import datetime

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

#####################################################################################

def reverseHostName ( email ) :
    name, sep, host = email.partition('@')
    hostparts = host[:-1].split('.')
    r_host = ''
    for part in hostparts :
        r_host = part + '.' + r_host
    return r_host + sep + name

#####################################################################################

ycsb_dir = 'YCSB/'
workload_dir = 'workload_spec/'
output_dir='workloads/'

def generateWorkload(workload, key_type, rawKeysFilename, workloadIdentifier) :

    print bcolors.OKGREEN + 'workload = ' + workload
    print 'key type = ' + key_type + bcolors.ENDC

    workload_name = workload + '_' + key_type;
    workloadIdentifier = '_' + workloadIdentifier if workloadIdentifier else ''
    output_workload_name = workload + '_' + key_type + workloadIdentifier

    out_ycsb_load = output_dir + 'ycsb_load_' + workload_name;
    out_ycsb_txn = output_dir + 'ycsb_txn_' + workload_name
    out_load_ycsbkey = output_dir + 'load_' + 'ycsbkey' + '_' + workload_name
    out_txn_ycsbkey = output_dir + 'txn_' + 'ycsbkey' + '_' + workload_name
    out_load = output_dir + output_workload_name + '_load.dat'
    out_txn = output_dir + output_workload_name + '_txn.dat'

    cmd_ycsb_load = ycsb_dir + 'bin/ycsb load basic -P ' + workload_dir + workload + ' -s > ' + out_ycsb_load
    cmd_ycsb_txn = ycsb_dir + 'bin/ycsb run basic -P ' + workload_dir + workload + ' -s > ' + out_ycsb_txn

    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    os.system(cmd_ycsb_load)
    os.system(cmd_ycsb_txn)

    #####################################################################################

    originalKeys = []
    f_load = open (out_ycsb_load, 'r')
    f_load_out = open (out_load_ycsbkey, 'w')
    for line in f_load :
        cols = line.split()
        if len(cols) > 0 and cols[0] == 'INSERT':
            originalKeys.append(long(cols[2][4:]))
            f_load_out.write (cols[0] + " " + cols[2][4:] + "\n")
    f_load.close()
    f_load_out.close()

    f_txn = open (out_ycsb_txn, 'r')
    f_txn_out = open (out_txn_ycsbkey, 'w')
    for line in f_txn :
        cols = line.split()
        if (cols[0] == 'SCAN') or (cols[0] == 'INSERT') or (cols[0] == 'READ') or (cols[0] == 'UPDATE'):
            startkey = cols[2][4:]
            if cols[0] == 'INSERT':
                originalKeys.append(long(startkey))
            if cols[0] == 'SCAN' :
                numkeys = cols[3]
                f_txn_out.write (cols[0] + ' ' + startkey + ' ' + numkeys + '\n')
            else :
                f_txn_out.write (cols[0] + ' ' + startkey + '\n')
    f_txn.close()
    f_txn_out.close()

    originalKeys.sort()
    numberUniqueKeys = len(originalKeys)

    cmd = 'rm -f ' + out_ycsb_load
    os.system(cmd)
    cmd = 'rm -f ' + out_ycsb_txn
    os.system(cmd)

    #####################################################################################

    valueDictionary = {}
    rankDictionary = {}
    for i, key in enumerate(originalKeys):
        rankDictionary[key] = i
    random.seed(datetime.now())  

    if key_type == 'randint':
        randomNumbers = [];
        for key in originalKeys:
            randomNumbers.append(random.getrandbits(63))
        randomNumbers.sort()
        currentId = 0;
        for key in originalKeys: 
            valueDictionary[key] = str(randomNumbers[currentId])
            currentId = currentId + 1
    elif key_type == 'monoint':
        currentId = 0
        for key in originalKeys:
            valueDictionary[key] = str(currentId)
            currentId = currentId + 1
    elif key_type == 'face':
        f_inputKeys = open (rawKeysFilename, 'r')
        inputKeys = f_inputKeys.readlines()
        selectedKeys = []
        for i, key in enumerate(originalKeys):
            selectedKeys.append(long(inputKeys[i]))
        selectedKeys.sort()
        for i, key in enumerate(originalKeys):
            valueDictionary[key] = str(selectedKeys[i]);    
    elif key_type == 'osm':
        f_inputKeys = open (rawKeysFilename, 'r')
        inputKeys = f_inputKeys.readlines()
        selectedKeys = []
        for i, key in enumerate(originalKeys):
            selectedKeys.append(long(inputKeys[i]))
        selectedKeys.sort()
        for i, key in enumerate(originalKeys):
            valueDictionary[key] = str(selectedKeys[i]);  
    elif key_type == 'wiki':
        f_inputKeys = open (rawKeysFilename, 'r')
        inputKeys = f_inputKeys.readlines()
        selectedKeys = []
        for i, key in enumerate(originalKeys):
            selectedKeys.append(long(inputKeys[i]))
        selectedKeys.sort()
        for i, key in enumerate(originalKeys):
            valueDictionary[key] = str(selectedKeys[i]);  

    keymap = {}
    f_load = open (out_load_ycsbkey, 'r')
    f_load_out = open (out_load, 'w')
    for line in f_load :
        cols = line.split()
        f_load_out.write (cols[0] + ' ' + valueDictionary[long(cols[1])] + '\n')
        

    f_txn = open (out_txn_ycsbkey, 'r')
    f_txn_out = open (out_txn, 'w')
    for line in f_txn :
        cols = line.split()
        if cols[0] == 'SCAN' :
            # f_txn_out.write (cols[0] + ' ' + valueDictionary[long(cols[1])] + ' ' + cols[2] + '\n')
            f_txn_out.write (cols[0] + ' ' + str(rankDictionary[long(cols[1])]) + ' ' + cols[2] + '\n')
        else :
            f_txn_out.write (cols[0] + ' ' + valueDictionary[long(cols[1])] + '\n')

    f_load_out.close()
    f_txn.close()
    f_txn_out.close()

    cmd = 'rm -f ' + out_load_ycsbkey
    os.system(cmd)
    cmd = 'rm -f ' + out_txn_ycsbkey
    os.system(cmd)

def printUsage() :
    print bcolors.WARNING + 'Usage: gen_workload.py <workload_file>'

def checkArguments(argv) :
    if(len(sys.argv) != 2) :
        printUsage()
        sys.exit(1)
    if(not os.path.isfile(argv[1])) :
        printUsage()
        print bcolors.FAIL + argv[1] + " does not exist."
        sys.exit(1)

def checkWorkloadRow(row, line) :
    if 'workload' not in row or 'keytype' not in row :
        print row
        print bcolors.FAIL + 'Workload file must be a CSV file with header line consisting of: "workload, keytype"' + bcolors.ENDC
        sys.exit(1)
    key_type = row['keytype']
    workload_file = workload_dir + '/' + row['workload']
    if not os.path.isfile(workload_file) :
        print bcolors.FAIL + 'Workload definition ' + workload_file + ' (line ' + line + ') does not exist.' + bcolors.ENDC
        sys.exit(1)
    if key_type not in [ 'randint', 'monoint', 'face', 'osm', 'wiki' ]:
        print bcolors.FAIL + 'Keytype ' + key_type + ' on line ' + line + ' is unknown. Only randint, monoint, mappedint or email are supported. ' + bcolors.ENDC
        sys.exit(1)

def main(argv):
    checkArguments(argv)
    config_file = argv[1]
    print bcolors.OKGREEN + 'generating workloads defined in ' + config_file + bcolors.ENDC

    with open(config_file) as csvfile:
        reader = csv.DictReader(filter(lambda line: not line.startswith("#"), csvfile), skipinitialspace = True)
        linenumber = 1;
        for row in reader:
            checkWorkloadRow(row, ++linenumber)
            generateWorkload(row['workload'], row['keytype'], row.get('filename', ''), row.get('workloadidentifier', ''))

if __name__ == '__main__':
    main(sys.argv)

